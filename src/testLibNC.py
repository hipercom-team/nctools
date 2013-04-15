#---------------------------------------------------------------------------
# Unit test for LibNC
#---------------------------------------------------------------------------
# Author: Cedric Adjih
# Copyright 2013 Inria
# All rights reserved. Distributed only with permission.
#---------------------------------------------------------------------------

import sys, unittest, pprint, random, itertools

import libnc

#---------------------------------------------------------------------------

class TestLinearCoding(unittest.TestCase):
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
        def mul(a,b): return libnc.lc_mul(a,b,l)
        def inv(a): return libnc.lc_inv(a,l)
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
        for l in range(libnc.MaxLog2NbBitCoef+1):
            self.checkScalarGF(l)

    def checkVectorGetSet(self, l):
        """Check that setting and getting coefs impact and use the right bits"""
        bitsPerCoef = (1<<l)
        coefPerByte = (libnc.BitsPerByte / bitsPerCoef)

        n = 256*8
        assert n % coefPerByte  == 0
        s = n//coefPerByte

        vref = libnc.u8array(s)
        v = libnc.u8array(s)

        for coef,fillByte in [((1<<bitsPerCoef)-1,0), (0, 0xff)]:
            libnc.u8array_set(vref.cast(), fillByte, s)

            for i in range(256):
                libnc.u8array_copy(v.cast(), vref.cast(), s)
                libnc.lc_vector_set(v.cast(), s, l,  i, coef)
                coef_back = libnc.lc_vector_get(v.cast(), s, l,  i)
                self.assertEqual(coef_back, coef)

                byteDiff = libnc.u8array_count_byte_diff(
                    v.cast(), vref.cast(), s)
                bitDiff = libnc.u8array_count_bit_diff(
                    v.cast(), vref.cast(), s)

                self.assertEqual(byteDiff, 1)
                self.assertEqual(bitDiff, bitsPerCoef)


    def test_checkVectorGetSet(self):
        for l in range(libnc.MaxLog2NbBitCoef+1):
            self.checkVectorGetSet(l)


    def checkVectorMul(self, l):
        """Check that the multiplication 'coef x vector' yields results that
           are consistent with individual scalar multiplication"""
        bitsPerCoef = (1<<l)
        coefPerByte = (libnc.BitsPerByte / bitsPerCoef)

        s = 256
        n = s*coefPerByte

        vref = libnc.u8array(s)
        v = libnc.u8array(s)

        for i in range(256):
            vref[i] = i

        for coef in range(1<<bitsPerCoef):
            libnc.lc_vector_mul(coef, vref.cast(), s, l, v.cast())
            for i in range(n):
                before = libnc.lc_vector_get(vref.cast(), s, l, i)
                after = libnc.lc_vector_get(v.cast(), s, l, i)
                product = libnc.lc_mul(coef, before, l)
                self.assertEqual(after, product)

        for i in range(256):
            self.assertEqual(vref[i], i)
            

    def test_checkVectorMul(self):
        for l in range(libnc.MaxLog2NbBitCoef+1):
            self.checkVectorMul(l)


    def test_checkVectorAdd(self):
        setList = ([set(), set(range(256)), set(range(0,256,2)), 
                    set(range(1,256,2)), set(range(0,256,4))] 
                   + [set([i]) for i in range(32)]
                   + [set([7*i,9*j]) for i in range(8) for j in range(8)])

        size = (max([max(s) for s in setList if len(s)>0])+7)/8
        v1 = libnc.u8array(size)
        v2 = libnc.u8array(size)
        w = libnc.u8array(size)
        sw_ptr = libnc.new_u16ptr()

        def setVector(v, valueSet):
            libnc.u8array_set(v.cast(), 0, size)
            for i in valueSet:
                libnc.lc_vector_set(v.cast(), size, 0,  i, 1)
            if len(valueSet) == 0: return 0
            return (max(valueSet)//8)+1

        def getVector(v, s):
            result = set()
            for i in range(s*8):
                if libnc.lc_vector_get(v.cast(), size, 0,  i):
                    result.add(i)
            return result

        for set1 in setList:
            for set2 in setList:
                s1 = setVector(v1, set1)
                s2 = setVector(v2, set2)
                libnc.lc_vector_add(v1.cast(),s1, v2.cast(),s2, 
                                    w.cast(), sw_ptr)
                sw = libnc.u16ptr_value(sw_ptr)
                self.assertEqual(sw, max(s1,s2))
                result = getVector(w, sw)
                expectedResult = set1.symmetric_difference(set2)
                self.assertEqual(result, expectedResult)


class TestCodedPacket(unittest.TestCase):
    def setUp(self):
        self.P = {}
        for l in range(libnc.MaxLog2NbBitCoef+1):
            nbHeaderCoef = (libnc.macro_COEF_HEADER_SIZE*8) // (1 << l)
            n = nbHeaderCoef
            self.P[l] = libnc.makeCodedPacketList(l, 4*n)

    def checkPacketAdd(self, l):
        P = self.P[l]
        current = P[0].clone()
        for i in range(len(P)-1):
            q = P[i] + P[i+1]
            current += q
            current.adjust()
        same = libnc.coded_packet_is_similar(current.content, P[-1].content)
        self.assertTrue(same)

    def test_packetAdd(self):
        for l in range(libnc.MaxLog2NbBitCoef+1):
            self.checkPacketAdd(l)

    def checkPacketSimilar(self, l):
        P = self.P[l]
        for i in range(len(P)):
            for j in range(len(P)):
                same = libnc.coded_packet_is_similar(P[i].content,P[j].content)
                self.assertEqual( same, i==j )

    def test_packetSimilar(self):
        for l in range(libnc.MaxLog2NbBitCoef+1):
            self.checkPacketSimilar(l)

    def test_decode(self):
        baseList = self.P[3]
        window = 4
        codedPacketList = libnc.generateLinearCombList(
            baseList, 2*len(baseList), window, 1)
        decodedList, posToBase, baseToPos = libnc.decode(codedPacketList)
        for basePos, packetPos in baseToPos.iteritems():
            p = decodedList[packetPos]
            if p.content.coef_pos_min == p.content.coef_pos_max:
                print basePos, p, repr(p.getData())
                same = libnc.coded_packet_is_similar(
                    p.content, baseList[basePos].content)
                self.assertTrue(same)


class TestPacketSet(unittest.TestCase):

    def checkSimpleCombDecoding(self, l):
        """Test if the packet_set code can correctly decode the set:
        {          c1. P1 + c2. P2 ,
          c3. P0          + c4. P2 ,
          c5. P0 + c6. P1 + c7. P2 } 
        with all permutations of coefficient positions
        """
        
        class DecodingRecorder:
            def __init__(self):
                self.decodedList = []
            def notifyPacketDecoded(self, packetId):
                self.decodedList.append(packetId)

        n = 1<<(1<<l)

        coefList = [3,7,9,11,13,17,19]
        coefList = [ x % n for x in coefList ]
        c1,c2,c3,c4,c5,c6,c7 = coefList

        for iList in itertools.permutations([0,1,2]):
            pktList = libnc.makeCodedPacketList(l, 3)
            recorder = DecodingRecorder()
            pktSet = libnc.allocCPacketSet(l, recorder)

            stat = libnc.new_reductionStat()

            i0,i1,i2 = iList
            pc0 = c1*pktList[i1].clone() + c2*pktList[i2].clone()
            pc1 = c3*pktList[i0].clone() + c4*pktList[i2].clone()
            pc2 = (c5*pktList[i0].clone() + c6*pktList[i1].clone() 
                   + c7*pktList[i2].clone())

            for p in [pc0, pc1, pc2]:
                packetId = libnc.packet_set_add(pktSet, p.content, stat)
                assert packetId != libnc.macro_PACKET_ID_NONE
                #pprint.pprint( eval(libnc.packet_set_pyrepr(pktSet)) )
                #print "-" * 50

            assert stat.decoded == 3
            assert sorted(recorder.decodedList) == range(3)

            for j in range(3):
                packetId = libnc.packet_set_get_id_of_pos(pktSet, j)
                decoded = libnc.packet_set_get_coded_packet(pktSet, packetId)
                #print libnc.coded_packet_pyrepr(decoded)
                self.assertTrue( libnc.coded_packet_was_decoded(decoded) )
                self.assertTrue( libnc.coded_packet_is_similar(
                        decoded, pktList[j].content) )

            libnc.freeCPacketSet(pktSet)
            libnc.delete_reductionStat(stat)
        

    def test_simpleCombDecoding(self):
        for l in range(libnc.MaxLog2NbBitCoef+1):
            self.checkSimpleCombDecoding(l)


    def checkDecodingPacketList(self, packetList, initialPacketList):
        """Check that decoding the packetList yields initialPacketList
        the packetList must be exactly of size initialPacketList"""
        assert len(packetList) == len(initialPacketList)
        if len(packetList) == 0:
            return # nothing to check
        l = packetList[0].getL()
        packetSet = libnc.PacketSet(l)
        for i,p in enumerate(packetList):
            self.assertEqual(len(packetSet), i)
            self.assertEqual(packetSet.stat.decoded, 0)
            packetId, decodedPacketList = packetSet.add(p)
            self.assertNotEqual(packetId, None)
        
        for i in range(len(initialPacketList)):
            initialPacket = initialPacketList[i]
            decodedPacket = packetSet.getPacketForCoefPos(i)
            self.assertTrue(decodedPacket.isSimilar(initialPacket))
            
        self.assertEqual(packetSet.stat.decoded, len(packetList))
        #print "l", packetSet.stat.decoded

    def checkDecodingWithCauchyMatrixComb(self, l):
        fieldSize = (1<<(1<<l))
        maxNbCoef = 1<<libnc.log2_window_size(l)
        m = min(maxNbCoef, fieldSize//2)
        initialPacketList = libnc.makeCodedPacketList(l, m)
        
        coefList = range(0, 2*m)

        for seed in range(8):
            random.seed(seed)
            random.shuffle(coefList)
        
            packetList = libnc.makeCauchyMatrixComb(initialPacketList, coefList)
            self.checkDecodingPacketList(packetList, initialPacketList)

    def test_decodingDenseComb(self):
        for l in range(libnc.MaxLog2NbBitCoef+1):
            self.checkDecodingWithCauchyMatrixComb(l)

TestLinearCoding = None
TestCodedPacket = None
#TestPacketSet = None

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
