#---------------------------------------------------------------------------
# Utility functions for libncmodule
#---------------------------------------------------------------------------
# Author: Cedric Adjih
# Copyright 2013 Inria
# All rights reserved. Distributed only with permission.
#---------------------------------------------------------------------------

import struct, hashlib, pprint

from libncmodule import *

MaxLog2NbBitCoef = 3 # 3 included
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

    def isSimilar(self, other):
        p1 = self.clone()
        p2 = other.clone()
        same = coded_packet_is_similar(p1.content, p2.content)
        return same

    def isDecoded(self):
        self.adjust()
        return coded_packet_was_decoded(self.content)

def makeCodedPacket(basePos, data, log2NbBitCoef):
    return CodedPacket(content = createCCodedPacket(
            basePos, data, log2NbBitCoef))

#--------------------------------------------------

def adjusted(p):
    result = p.clone()
    result.adjust()
    return result

#--------------------------------------------------

def makeCodedPacketList(l, n = None):
    nbHeaderCoef = (macro_COEF_HEADER_SIZE*8) // (1 << l)
    if n == None: 
        n = nbHeaderCoef
    result = []
    for i in range(n):
        data = "\x01<packet %s GF(%s)>\x00" % (i, 1<<(1<<l))
        data += "*"*(i%5) + "\x02"
        p = makeCodedPacket(i, data, l)
        result.append(p)
    return result

#--------------------------------------------------

def makeCauchyMatrixComb(originalPacketList, coefList):
    assert len(coefList) == 2*len(originalPacketList)
    m = len(originalPacketList)
    if m == 0: 
        return []
    l = originalPacketList[0].getL()
    packetList = []
    for i in range(m):
        current = CodedPacket(l)
        for j in range(m):
            x = coefList[i]
            y = coefList[j+m]
            c = lc_inv(x ^ lc_neg(y,l), l) # 1 / (x -y)  in GF(2^k)
            current += c * originalPacketList[j]

        packetList.append(current)
    return packetList    

#---------------------------------------------------------------------------

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
# Decode source packets from a list of CodedPacket
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
# Packet Set
#---------------------------------------------------------------------------

def allocCPacketSet(log2NbBitCoef, notifyObj = None):
    result = new_packetSet()
    if notifyObj == None:
        packet_set_init(result, log2NbBitCoef, None, None, None)
    else:
        notifyObjPtr = my_inc_ref(notifyObj)
        packet_set_init(result, log2NbBitCoef, 
                        py_callback_packet_decoded,
                        py_callback_set_full,
                        notifyObjPtr)
    return result

def freeCPacketSet(cPacketSet):
    if cPacketSet.notif_data != None:
        my_dec_ref(cPacketSet.notif_data)
    delete_packetSet(cPacketSet)

#--------------------------------------------------

class PacketSet:
    def __init__(self, log2NbBitCoef):
        self.l = log2NbBitCoef
        self.content = allocCPacketSet(self.l, self)
        self.decodedList = []
        self.stat = new_reductionStat()
        reduction_stat_init(self.stat)

    def notifyPacketDecoded(self, packetId):
        self.decodedList.append(packetId)
        self.lastDecodedList.append(packetId)

    def add(self, codedPacket):
        codedPacket = codedPacket.clone()
        self.lastDecodedList = []
        packetId = packet_set_add(self.content, codedPacket.content, self.stat)
        if packetId == macro_PACKET_ID_NONE:
            packetId = None
        result = (packetId, self.lastDecodedList)
        del self.lastDecodedList
        return result

    def getPacketForCoefPos(self, coefPos):
        packetId = packet_set_get_id_of_pos(self.content, coefPos)
        if packetId == macro_PACKET_ID_NONE:
            return None
        cPacketRef = packet_set_get_coded_packet(self.content, packetId)
        cPacket = cloneCCodedPacket(cPacketRef)
        return CodedPacket(content=cPacket)

    def __len__(self): return packet_set_count(self.content)
    def isEmpty(self): return packet_set_is_empty(self.content)

    def __str__(self):
        if self.isEmpty(): return "{}"
        r = []
        for coefPos in range(self.content.coef_pos_min,
                             self.content.coef_pos_max+1):
            p = self.getPacketForCoefPos(coefPos)
            r.append("%d: " % coefPos + str(p))
        return "{ " + "\n  ".join(r) + " }"

    __repr__ = __str__

    def __del__(self):
        delete_reduction_stat(self.stat)
        self.stat = None
        freeCPacketSet(self.content)
        self.content = None

    def toMatrixStr(self, withContent = False):
        rList = []
        s = len("%s"%(1<<(1<<self.l)))
        for coefPos in range(self.content.coef_pos_min,
                             self.content.coef_pos_max+1):
            p = self.getPacketForCoefPos(coefPos)
            if p == None: 
                continue
            r = "[%d]" % coefPos
            coefTable = p.getCoefTable()
            for i in range(self.content.coef_pos_min,
                             self.content.coef_pos_max+1):
                r += " " + ("%s"%coefTable.get(i,0)).rjust(s)
            if withContent:
                r += " " + repr(p.getData())
            rList.append(r)
        return "{ " + "\n  ".join(rList) + " }"

#---------------------------------------------------------------------------
