#--------------------------------------------------------------*- python -*-
# Generate tables for a field of a given order
#---------------------------------------------------------------------------
# Author: Cedric Adjih
# Copyright 2013 Inria
# All rights reserved. Distributed only with permission.
#---------------------------------------------------------------------------

from sage import *   # import sage library
from sage.all_cmdline import * 
import sys

if len(sys.argv) < 2:
    sys.stderr.write("Syntax: sage %s <field order>\n" % sys.argv[0])
    sys.exit(1)

FieldOrder = int(sys.argv[1])

K = GF(FieldOrder, repr='int', names=('a',))
elemList = [x for x in K]

mulTable = {}
sumTable = {}
for x in elemList:
    for y in elemList:
        mulTable[(x,y)] = x*y
	sumTable[(x,y)] = x+y

invTable = {}
negTable = {}
for x in elemList:
    if x != 0:
        invTable[x] = 1/x
    negTable[x] = 0-x

assert elemList[0] == 0

expTable = {}
logTable = {}
if elemList[1] == 1: gen = elemList[2]
else: gen = elemList[1]
x = gen
expTable[0] = 1
for i in range(1, FieldOrder):
    expTable[i] = x
    logTable[x] = i
    x = gen * x

info = {
     "sumTable": sumTable,
     "mulTable": mulTable,
     "invTable": invTable,
     "negTable": negTable,
     "expTable": expTable,
     "logTable": logTable
}

f = open("table-gf%s.pydat" % FieldOrder, "w")
f.write(repr(info))
f.close()

#---------------------------------------------------------------------------
