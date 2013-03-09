#---------------------------------------------------------------------------
# Unit test for LibNC
#---------------------------------------------------------------------------
# Author: Cedric Adjih
# Copyright 2013 Inria
# All rights reserved. Distributed only with permission.
#---------------------------------------------------------------------------

import sys, unittest, pprint, random

import libnc

#---------------------------------------------------------------------------

class Test(unittest.TestCase):
    def setUp(self):
        pass

    def checkScalarGF(self, l):
        """check GF(2^(2^l)) identities: 
        * x.1 == x
        * x.0 == 0
        * x.y == y.x
        * x.inv(x) == 1
        * x.(y+z) == x.y + x.z
        * x.(y.z) = (x.y).z
        """
        n = 1<<(1<<l)
        def mul(a,b): return libnc.lc_mul_scalar(a,b,l)
        def inv(a): return libnc.lc_inv_scalar(a,l)
        def add(a,b): return a^b

        # x.1 == x and x.0 == 0
        for x in range(n):
            x_times_1 = mul(x,1)
            x_times_0 = mul(x,0)
            self.assertEqual(x_times_1, x)
            self.assertEqual(x_times_0, 0)

        # x.y == y.x
        for x in range(n):
            for y in range(n):
                x_times_y = mul(x,y)
                y_times_x = mul(y,x)
                self.assertEqual(x_times_y, y_times_x)

        # x.inv(x) == 1
        for x in range(1,n):
            x_inv = inv(x)
            x_times_inv = mul(x, x_inv)
            self.assertEqual(x_times_inv, 1)

        # x.(y.z) = (x.y).z
        for x in range(n):
            for y in range(n):
                for z in range(n):
                    v1 = mul(x, mul(y,z))
                    v2 = mul(mul(x,y), z)
                    self.assertEqual(v1, v2)

        # x.(y+z) == x.y + x.z
        for x in range(n):
            for y in range(n):
                for z in range(n):
                    v1 = mul(x, add(y,z))
                    v2 = add(mul(x,y), mul(x,z))
                    self.assertEqual(v1, v2)


    def test_checkScalarGF(self):
        for l in range(libnc.MaxLog2NbBitCoef):
            self.checkScalarGF(l)


#---------------------------------------------------------------------------

def checkTable():
    import writeGaloisFieldTable as fieldTable
    fieldTable.checkTable()

#---------------------------------------------------------------------------

if __name__ == "__main__":
    
    if len(sys.argv) > 1:
        testName = sys.argv[1]
    else: testName = None

    if testName == "check-table":
        checkTable()
    elif testName == "check" or testName == None:
        sys.argv = sys.argv[0:1] # hack
        unittest.main()
    else: 
        sys.stderr.write("Syntax: %s check|check-table\n" % sys.argv[0])
        sys.exit(1)

#---------------------------------------------------------------------------
