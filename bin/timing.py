#!/usr/bin/env python3
import subprocess
import csv
import os
'''Python script to test linear regression using PALISADE and Intex SGX with Graphene
    captures output of all the files (time to complete individual portions, and writes to csv files
    any errors will still be written to terminal'''

#output files
firstMultFile = open('result/firstMult.csv', 'w')
secondMultFile = open('result/secondMult.csv','w')
sgxFile = open('result/sgx.csv','w')

#csv writers
firstMultWriter = csv.writer(firstMultFile,csv.get_dialect('excel'))
secondMultWriter = csv.writer(secondMultFile,csv.get_dialect('excel'))
sgxWriter = csv.writer(sgxFile,csv.get_dialect('excel'))

#default values for p and n
dP = 8
dN = 5000000
subprocess.run(['./newMake','-p','12','-n','10000000']) #making the data, will take >24 hours
for i in range(2,13): #modifying p
    row = [i,dN]
    for j in range(10): #run each 10 times to get an average

        #perform first multiplication
        result = subprocess.run(['./firstMult','-p',f'{i}','-n',f'{dN}'],stdout = subprocess.PIPE)
        firstMultRow = row + bytes.decode(result.stdout).strip().split('\n')
        firstMultWriter.writerow(firstMultRow)
        print(f'{i} {j} first mult complete')

        #perform second multiplication
        result = subprocess.run(['./secondMult','-p',f'{i}','-n',f'{dN}'],stdout=subprocess.PIPE)
        secondMultRow = row + bytes.decode(result.stdout).strip().split('\n')
        secondMultWriter.writerow(secondMultRow)
        print(f'{i} {j} second mult complete')

        #have to modify environ variables to run with SGX using pal_loader
        env = os.environ
        env['SGX'] = '1'
        env['OMP_NUM_THREADS']='1'
        
        #running final SGX inverse operation
        result = subprocess.run(['./pal_loader','./inverse','-p',f'{i}','-n',f'{dN}'],stdout=subprocess.PIPE, env=env)
        sgxRow = row + bytes.decode(result.stdout).strip().split('\n')
        sgxWriter.writerow(sgxRow)
        print(f'{i} {j} inverse complete')

for i in range(1000000,10000001,1000000): #modifying n
    row = [dP,i]
    for j in range(10):
        #running first multiplication
        result = subprocess.run(['./firstMult','-p',f'{dP}','-n',f'{i}'],stdout = subprocess.PIPE)
        firstMultRow = row + bytes.decode(result.stdout).strip().split('\n')
        firstMultWriter.writerow(firstMultRow)
        print(f'{i} {j} first mult complete')

        #running second multiplication
        result = subprocess.run(['./secondMult','-p',f'{dP}','-n',f'{i}'],stdout=subprocess.PIPE)
        secondMultRow = row + bytes.decode(result.stdout).strip().split('\n')
        secondMultWriter.writerow(secondMultRow)
        print(f'{i} {j} second mult complete')

        #have to modify environment var to run with SGX using pal_loader
        env = os.environ
        env['SGX'] = '1'
        env['OMP_NUM_THREADS']='1'
        #running sgx inverse
        result = subprocess.run(['./pal_loader','./inverse','-p',f'{dP}','-n',f'{i}'],stdout=subprocess.PIPE,env=env)
        sgxRow = row + bytes.decode(result.stdout).strip().split('\n')
        sgxWriter.writerow(sgxRow)
        print(f'{i} {j} inverse complete')


        
