#!/usr/bin/env python3
import subprocess
import csv

firstMultFile = open('firstMult.csv', 'w')
secondMultFile = open('secondMult.csv','w')
sgxFile = open('sgx.csv','w')

firstMultWriter = csv.writer(firstMultFile,csv.get_dialect('excel'))
secondMultWriter = csv.writer(secondMultFile,csv.get_dialect('excel'))
sgxWriter = csv.writer(sgxFile,csv.get_dialect('excel'))

dP = 8
dN = 5000

for i in range(2,13):
    row = [i,dN]
    subprocess.run(['./makeData','-p',f'{i}','-n',f'{dN}'])
    print(f'{i}, {dN} data created, running test')
    for j in range(10):
        result = subprocess.run(['./firstMult','-p',f'{i}','-n',f'{dN}'],stdout = subprocess.PIPE)
        firstMultRow = row + bytes.decode(result.stdout).strip().split('\n')
        firstMultWriter.writerow(firstMultRow)
        print(f'{i} {j} first mult complete, results:')
        print(result.stdout)

        result = subprocess.run(['./secondMult','-p',f'{i}','-n',f'{dN}'],stdout=subprocess.PIPE)
        secondMultRow = row + bytes.decode(result.stdout).strip().split('\n')
        secondMultWriter.writerow(secondMultRow)
        print(f'{i} {j} second mult complete, results:')
        print(result.stdout)

        result = subprocess.run(['./pal_loader','./inverse','-p',f'{i}','-n',f'{dN}'],stdout=subprocess.PIPE)
        sgxRow = row + bytes.decode(result.stdout).strip().split('\n')
        sgxWriter.writerow(sgxRow)
        print(f'{i} {j} inverse complete, results: ')
        print(result.stdout)

for i in range(1000,10000,1000):
    row = [dP,i]
    subprocess.run(['./makeData','-p',f'{dP}','-n',f'{i}'])
    print(f'{dP}, {i} data created, running test')
    for j in range(10):
        result = subprocess.run(['./firstMult','-p',f'{dP}','-n',f'{i}'],stdout = subprocess.PIPE)
        firstMultRow = row + bytes.decode(result.stdout).strip().split('\n')
        firstMultWriter.writerow(firstMultRow)
        print(f'{i} {j} first mult complete, results:')
        print(result.stdout)

        result = subprocess.run(['./secondMult','-p',f'{dP}','-n',f'{i}'],stdout=subprocess.PIPE)
        secondMultRow = row + bytes.decode(result.stdout).strip().split('\n')
        secondMultWriter.writerow(secondMultRow)
        print(f'{i} {j} second mult complete, results:')
        print(result.stdout)

        result = subprocess.run(['./pal_loader','./inverse','-p',f'{dP}','-n',f'{i}'],stdout=subprocess.PIPE)
        sgxRow = row + bytes.decode(result.stdout).strip().split('\n')
        sgxWriter.writerow(sgxRow)
        print(f'{i} {j} inverse complete, results: ')
        print(result.stdout)


        
