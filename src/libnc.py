#---------------------------------------------------------------------------
# Utility functions for libncmodule
#---------------------------------------------------------------------------
# Author: Cedric Adjih
# Copyright 2013 Inria
# All rights reserved. Distributed only with permission.
#---------------------------------------------------------------------------

from libncmodule import *


MaxLog2NbBitCoef = 4 # not included
Log2BitsPerByte = 3
BitsPerByte = (1 << Log2BitsPerByte)

#---------------------------------------------------------------------------
# Low Level Functions
#---------------------------------------------------------------------------

def gfToCVector(coefList, l):
    bitsPerCoef = (1<<l)
    coefPerByte = (BitsPerByte / bitsPerCoef)

    nbCoef = len(coefList)
    s = (nbCoef // coefPerByte)
    if (nbCoef % coefPerByte) != 0:
        s += 1
    result = new_u8array(s)

    for i,coef in enumerate(coefList):
        lc_vector_set(result, coef)

    return (result, s)


def gfFromCVector(v,s):
    pass
    
def gfAddVector(v1, s1, v2, s2):
    s = max(s1,s2)
    result = new_u8array(s)
    s_ptr = new_u16ptr()
    lc_add_vector(v1, s1, v2, s2, result, s_ptr)
    assert u16_value(s_ptr) == result
    delete_u16ptr(result)

def gfMulVector(v1):
    pass

class GFVector:
    def __init__(self, size, l):
        self.l = l
        self.data = new_u8array()

    def __del__(self):
        delete_u8array(self.data)
        self.data = None

#---------------------------------------------------------------------------
