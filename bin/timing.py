#!/usr/bin/env python3
import subprocess
import csv
import os
import time
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
parallelWriter = csv.writer(open("result/parallel.csv",'w'),csv.get_dialect('excel'))

#default values for p and n
dP = 10
dN = 5000000
#subprocess.run(['./newMake','-p','12','-n','100000']) #making the data, will take >24 hours
for i in range(2,13): #modifying p
    row = [i,dN]
    for j in range(10): #run each 10 times to get an average
        env = os.environ
        env['OMP_NUM_THREADS']='21'

        #perform first multiplication
        start = time.time()
        result = subprocess.Popen(['./firstMult','-p',f'{i}','-n',f'{dN}'],stdout = subprocess.PIPE)
        
        env['OMP_NUM_THREADS']='10'
        #perform second multiplication
        result2 = subprocess.Popen(['./secondMult','-p',f'{i}','-n',f'{dN}'],stdout=subprocess.PIPE)

        result.wait()
        result2.wait()
        end = time.time()
        parallelWriter.writerow([end-start])
        firstMultRow = row + bytes.decode(result.stdout.read()).strip().split('\n')
        firstMultWriter.writerow(firstMultRow)

        secondMultRow = row + bytes.decode(result2.stdout.read()).strip().split('\n')
        secondMultWriter.writerow(secondMultRow)


        #have to modify environ variables to run with SGX using pal_loader
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
        env = os.environ
        #running first multiplication
        start = time.time()
        env['OMP_NUM_THREADS']='21'
        result = subprocess.Popen(['./firstMult','-p',f'{dP}','-n',f'{i}'],stdout = subprocess.PIPE)
        

        #running second multiplication
        env['OMP_NUM_THREADS']='10'
        result2 = subprocess.Popen(['./secondMult','-p',f'{dP}','-n',f'{i}'],stdout=subprocess.PIPE)

        result.wait()
        result2.wait()
        end = time.time()

        parallelWriter.writerow([end-start])
        firstMultRow = row + bytes.decode(result.stdout.read()).strip().split('\n')
        firstMultWriter.writerow(firstMultRow)

        secondMultRow = row + bytes.decode(result2.stdout.read()).strip().split('\n')
        secondMultWriter.writerow(secondMultRow)


        #have to modify environment var to run with SGX using pal_loader

        env['SGX'] = '1'
        env['OMP_NUM_THREADS']='1'
        #running sgx inverse
        result = subprocess.run(['./pal_loader','./inverse','-p',f'{dP}','-n',f'{i}'],stdout=subprocess.PIPE,env=env)
        sgxRow = row + bytes.decode(result.stdout).strip().split('\n')
        sgxWriter.writerow(sgxRow)
        print(f'{i} {j} inverse complete')


        
