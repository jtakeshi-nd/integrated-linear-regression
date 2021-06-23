#!/usr/bin/env python3
import subprocess
import csv
import os

firstMultFile = open('result/firstMult.csv', 'w')
secondMultFile = open('result/secondMult.csv','w')
sgxFile = open('result/sgx.csv','w')

firstMultWriter = csv.writer(firstMultFile,csv.get_dialect('excel'))
secondMultWriter = csv.writer(secondMultFile,csv.get_dialect('excel'))
sgxWriter = csv.writer(sgxFile,csv.get_dialect('excel'))

dP = 8
dN = 5000000
subprocess.run(['./makeData','-p','12','-n','1000000'])
for i in range(2,13):
    row = [i,dN]
    print(f'{i}, {dN} data created, running test')
    for j in range(10):
        result = subprocess.run(['./firstMult','-p',f'{i}','-n',f'{dN}'],stdout = subprocess.PIPE)
        firstMultRow = row + bytes.decode(result.stdout).strip().split('\n')
        firstMultWriter.writerow(firstMultRow)
        print(f'{i} {j} first mult complete')

        result = subprocess.run(['./secondMult','-p',f'{i}','-n',f'{dN}'],stdout=subprocess.PIPE)
        secondMultRow = row + bytes.decode(result.stdout).strip().split('\n')
        secondMultWriter.writerow(secondMultRow)
        print(f'{i} {j} second mult complete')
        env = os.environ
        env['SGX'] = '1'
        env['OMP_NUM_THREADS']='1'
        
        result = subprocess.run(['./pal_loader','./inverse','-p',f'{i}','-n',f'{dN}'],stdout=subprocess.PIPE, env=env)
        sgxRow = row + bytes.decode(result.stdout).strip().split('\n')
        sgxWriter.writerow(sgxRow)
        print(f'{i} {j} inverse complete')

for i in range(1000000,10000001,1000000):
    row = [dP,i]
    print(f'{dP}, {i} data created, running test')
    for j in range(10):
        result = subprocess.run(['./firstMult','-p',f'{dP}','-n',f'{i}'],stdout = subprocess.PIPE)
        firstMultRow = row + bytes.decode(result.stdout).strip().split('\n')
        firstMultWriter.writerow(firstMultRow)
        print(f'{i} {j} first mult complete')

        result = subprocess.run(['./secondMult','-p',f'{dP}','-n',f'{i}'],stdout=subprocess.PIPE)
        secondMultRow = row + bytes.decode(result.stdout).strip().split('\n')
        secondMultWriter.writerow(secondMultRow)
        print(f'{i} {j} second mult complete')

        env = os.environ
        env['SGX'] = '1'
        env['OMP_NUM_THREADS']='1'
        result = subprocess.run(['SGX=1','OMP_NUM_THREADS=1','./pal_loader','./inverse','-p',f'{dP}','-n',f'{i}'],stdout=subprocess.PIPE,env=env)
        sgxRow = row + bytes.decode(result.stdout).strip().split('\n')
        sgxWriter.writerow(sgxRow)
        print(f'{i} {j} inverse complete')


        
