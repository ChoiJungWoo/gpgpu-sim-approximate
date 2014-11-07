#!/usr/bin/env python
# encoding: utf-8
import math

appro_file = open('appro_results.log')
accrt_file = open('accrt_results.log')

#NMSE = {E((Pi - Mi)^2/(Pavg*Mavg))}/ N
#NMSE = sqrt{E[(Pi - Mi)^2/(Mavg)^2]/ N}

#line = appro_file.readline()

numL =   0

Bias=   []   #[0 for i in range(numSec) ]

Pi = []
Mi = []

numSec = 0

for line in appro_file:
    numSec = len(line.split(' '))
    newL = []
    for i in range(numSec):
        newL.append(float(line.split(' ')[i]))
    Pi.append(newL)
    numL+=1

for line in accrt_file:
    numSec = len(line.split(' '))
    newL = []
    for i in range(numSec):
        newL.append(float(line.split(' ')[i]))
    Mi.append(newL)

#print Pi
#print Mi
Pavg=   [0.0 for i in range(numSec) ]
Psum=   [0.0 for i in range(numSec) ]
Mavg=   [0.0 for i in range(numSec) ]
Msum=   [0.0 for i in range(numSec) ]

#for line1 in appro_file, line2 in accrt_file:
for i in range(numSec):
    newB = []
    for j in range(numL):
        newB.append(Pi[j][i] - Mi[j][i])
        Psum[i] += Pi[j][i]
        Msum[i] += Mi[j][i]
    Bias.append(newB)

#print Bias
#print Psum
#print Msum

for i in range(numSec):
    Pavg[i] = Psum[i]/numL
    Mavg[i] = Msum[i]/numL

print Pavg
print Mavg

#print numL
NMSE = [ 0.0 for i in range(numSec) ]

for i in range(numSec):
    for j in range(numL):
        NMSE[i] += Bias[i][j]*Bias[i][j]
    if Pavg[i] == 0.0 or Mavg[i] == 0.0:
        NMSE[i] = NMSE[i]/numL
    else:
        #NMSE[i] = NMSE[i]/(Pavg[i]*Mavg[i])/numL
        NMSE[i] = NMSE[i]/numL

for i in range(numSec):
    NMSE[i] = math.sqrt(NMSE[i])/Pavg[i];

for i in range(numSec):
    print NMSE[i]


