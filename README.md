Linear Regression - Integrated Homomorphic Encryption and Intel SGX with Graphene

1. make clean
2. make initialize
3. make

Generate random data with specified n (users) and p (feature values)
4. :~/integrated-linear-regression/graphene$ ./makeData -n 3 -p 3

- This creates ctexts/dependent.ctext, ctexts/original.ctext, ctexts/tranpose.ctext
- Also creates PALISADEContainer


Run X (transpose) times X and write result to file
5. :~/integrated-linear-regression/graphene$ ./specialMult -n 3 -p 3

- This creates quotient.ctext (will be read into SGX)


Run inverse algorithm on quotient.ctext
6. :~/integrated-linear-regression/graphene$ make SGX=1 -f mk_graphene inverse.manifest.sgx inverse.token pal_loader
   :~/integrated-linear-regression/graphene$ SGX=1 OMP_NUM_THREADS=1 ./pal_loader ./inverse -n 3 -p 3

- This decrypts quotient.ctext, finds inverse, encrypts, and write ciphertexts to ctexts/inv.ctext


7. 
