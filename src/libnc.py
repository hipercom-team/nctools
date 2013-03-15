#---------------------------------------------------------------------------
# Utility functions for libncmodule
#---------------------------------------------------------------------------
# Author: Cedric Adjih
# Copyright 2013 Inria
# All rights reserved. Distributed only with permission.
#---------------------------------------------------------------------------

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

class CodedPacket:
    def __init__(self, log2NbBitCoef = None, content = None):
        if content != None:
            self.content = content
        else:
            assert log2NbBitCoef != None
            self.content = allocCCodedPacket(log2NbBitCoef)

    def __del__(self):
        return # XXX: remove
        freeCCodedPacket(self.content)
        self.content = None

    def clone(self):
        return CodedPacket(content = cloneCCodedPacket(self.content))

    def __str__(self):
        if coded_packet_is_empty_safe(self.content):
            r = "0"
        else:
            rList = []
            for i in range(self.content.coef_index_min, 
                           self.content.coef_index_max+1):
                coef = coded_packet_get_coef(self.content, i)
                if coef != 0:
                    rList.append("%s.P%s" % (coef,i))
            r = "+".join(rList)
        if self.content.coef_index_min != macro_COEF_INDEX_NONE:
            r += "[%s:%s]" % (self.content.coef_index_min, 
                              self.content.coef_index_max)
        return r + "/GF(2^%s)" % (1<<self.content.log2_nb_bit_coef)

    __repr__ = __str__

    def __setitem__(self, coefIndex, value):
        assert self.content.data_size == 0
        coded_packet_set_coef(self.content, coefIndex, value)

    def __getitem__(self, coefIndex):
        return coded_packet_get_coef(self.content, coefIndex)

    def __add__(self, other):
        return CodedPacket(content = addCCodedPacket(self.content, 
                                                     other.content))
    __sub__ = __add__

    def __rmul__(self, coef):
        return CodedPacket(content = scaleCCodedPacket(coef, self.content))

    def getRawCoef(self):
        return [u8block_getitem(self.content.content.u8, i)
                for i in range(macro_COEF_HEADER_SIZE) ]

    def adjust(self):
        coded_packet_adjust_min_max_coef(self.content)
            

def adjusted(p):
    result = p.clone()
    result.adjust()
    return result



l = 3

nbHeaderCoef = (macro_COEF_HEADER_SIZE*8) // (1 << l)
n = nbHeaderCoef
P = []

for i in range(nbHeaderCoef*2):
    print i
    p = CodedPacket(l) 
    p[i] = 1
    P.append(p)

#print P[0]+P[n-1]

#---------------------------------------------------------------------------
#
#---------------------------------------------------------------------------

