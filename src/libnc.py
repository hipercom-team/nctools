#---------------------------------------------------------------------------
# Utility functions for libncmodule
#---------------------------------------------------------------------------
# Author: Cedric Adjih
# Copyright 2013 Inria
# All rights reserved. Distributed only with permission.
#---------------------------------------------------------------------------

import struct, hashlib

from libncmodule import *

MaxLog2NbBitCoef = 3 # included
Log2BitsPerByte = 3
BitsPerByte = (1 << Log2BitsPerByte)

#---------------------------------------------------------------------------
# Low Level Functions - untested - unused
#---------------------------------------------------------------------------

#---------------------------------------------------------------------------
# Coded Packet
#---------------------------------------------------------------------------

def allocCCodedPacket(log2NbBitCoef):
    result = new_codedPacket()
    coded_packet_init(result, log2NbBitCoef)
    return result

def createCCodedPacket(basePos, data, log2NbBitCoef):
    result = new_codedPacket()
    coded_packet_init_from_base_packet(result, log2NbBitCoef, basePos,
                                       cast_to_u8ptr(data), len(data))
    return result

def freeCCodedPacket(cCodedPacket):
    delete_codedPacket(cCodedPacket)

def cloneCCodedPacket(other):
    result = new_codedPacket()
    coded_packet_copy_from(result, other)
    return result

def addCCodedPacket(p1, p2):
    result = allocCCodedPacket(p1.log2_nb_bit_coef)
    coded_packet_to_add(result, p1, p2)
    return result

def scaleCCodedPacket(coef, p):
    result = cloneCCodedPacket(p)
    coded_packet_to_mul(result, coef)
    return result

#--------------------------------------------------

class CodedPacket:
    def __init__(self, log2NbBitCoef = None, content = None,
                 coefAndData = None):
        if content != None:
            self.content = content
        else:
            assert log2NbBitCoef != None
            self.content = allocCCodedPacket(log2NbBitCoef)

    def __del__(self):
        try: freeCCodedPacket(self.content)
        except: pass
        self.content = None

    def clone(self):
        return CodedPacket(content = cloneCCodedPacket(self.content))

    def __str__(self):
        if coded_packet_is_empty_safe(self.content):
            r = "0"
        else:
            rList = []
            coefTable = self.getCoefTable()
            for i in sorted(coefTable.keys()):
                rList.append("%s.P%s" % (coefTable[i],i))
            r = "+".join(rList)
        if self.content.coef_pos_min != macro_COEF_POS_NONE:
            r += "[%s:%s]" % (self.content.coef_pos_min, 
                              self.content.coef_pos_max)
        return r + "/GF(%s)" % (1<<(1<<self.content.log2_nb_bit_coef))

    __repr__ = __str__

    def __setitem__(self, coefPos, value):
        assert self.content.data_size == 0
        coded_packet_set_coef(self.content, coefPos, value)

    def __getitem__(self, coefPos):
        return coded_packet_get_coef(self.content, coefPos)

    def __add__(self, other):
        return CodedPacket(content = addCCodedPacket(self.content, 
                                                     other.content))
    __sub__ = __add__
    __xor__ = __add__

    def __rmul__(self, coef):
        return CodedPacket(content = scaleCCodedPacket(coef, self.content))

    def getRawCoef(self):
        return [u8block_getitem(self.content.content.u8, i)
                for i in range(macro_COEF_HEADER_SIZE) ]

    def getCoefTable(self):
        result = {}
        for i in range(self.content.coef_pos_min,
                       self.content.coef_pos_max+1):
            coef = coded_packet_get_coef(self.content, i)
            if coef != 0:
                result[i] = coef
        return result

    def getData(self):
        r = ""
        for i in range(self.content.data_size):
            c = u8block_getitem(self.content.content.u8, 
                                i+macro_COEF_HEADER_SIZE)
            r += chr(c)
        return r
            
    def adjust(self):
        coded_packet_adjust_min_max_coef(self.content)

    def getL(self):
        return self.content.log2_nb_bit_coef


def makeCodedPacket(basePos, data, log2NbBitCoef):
    return CodedPacket(content = createCCodedPacket(
            basePos, data, log2NbBitCoef))

#--------------------------------------------------

def adjusted(p):
    result = p.clone()
    result.adjust()
    return result

#--------------------------------------------------

def makeCodedPacketList(l, n):
    nbHeaderCoef = (macro_COEF_HEADER_SIZE*8) // (1 << l)
    n = nbHeaderCoef
    result = []
    for i in range(n):
        data = "\x01<packet %s GF(%s)>\x00" % (i, 1<<(1<<l))
        data += "*"*(i%5) + "\x02"
        p = makeCodedPacket(i, data, l)
        result.append(p)
    return result

def floatFromHash(data):
    """return a number in [0,1[ computed from the hash of data"""
    hashInt = struct.unpack("!I", hashlib.md5(data).digest()[0:4])[0]
    return hashInt /float (1L<<32L)
    
def intFromHash(intBound, data):
    return int(floatFromHash(data) * intBound)

def generateLinearCombList(packetList, nbComb, window, seed):
    if len(packetList) == 0:
        return []
    l = packetList[0].content.log2_nb_bit_coef
    result = []
    i = 0
    while len(result) != nbComb:
        r = "[%s] comb#%s" % (seed, i)
        i += 1
        firstPos = intFromHash( max(len(packetList)-window, 0), r)
        subPacketList = []
        for j in range(window):
            if firstPos+j > len(packetList):
                continue
            r2 = r + " at%s" % j
            if intFromHash(2, r2) == 0:
                continue
            coef = 1+intFromHash((1<<(1<<l))-1, r2+" coef")
            subPacketList.append(coef * packetList[firstPos+j])
        if len(subPacketList) == 0:
            continue
        p = subPacketList[0]
        for q in subPacketList[1:]:
            p += q
        result.append(p)
    return result

#---------------------------------------------------------------------------
#
#---------------------------------------------------------------------------

def decode(codedPacketList):
    def getCoefMax(codedPacket):
        v = codedPacket.content.coef_pos_max
        return v
    codedPacketList = [adjusted(x) for x in codedPacketList]
    codedPacketList.sort(key=getCoefMax)

    posToBase = {}
    baseToPos = {}

    for i in range(len(codedPacketList)):
        p = codedPacketList[i]
        p.adjust()
        if coded_packet_is_empty(p.content):
            posToBase[i] = None
            continue
        coefPos = p.content.coef_pos_max
        assert coefPos not in baseToPos

        baseToPos[coefPos] = i
        posToBase[i] = coefPos
        assert p[coefPos] != 0
        p = lc_inv(p[coefPos], p.getL()) * p
        codedPacketList[i] = p

        for j in range(len(codedPacketList)):
            if i == j: continue
            q = codedPacketList[j]
            if q.content.coef_pos_min <= coefPos <= q.content.coef_pos_max:
                cq = q[coefPos]
                if cq != 0:
                    q = q - cq * p
                    q.adjust()
                    codedPacketList[j] = q
    return codedPacketList, posToBase, baseToPos

#---------------------------------------------------------------------------
