Linear Regression - Integrated Homomorphic Encryption and Intel SGX with Graphene

1. make clean
2. make initialize
3. make

Generate random data with specified n (users) and p (feature values)
4. :~/integrated-linear-regression/bin$ ./makeData -n 3 -p 3

- This creates ctexts/dependent.ctext, ctexts/original.ctext, ctexts/tranpose.ctext
- Also creates PALISADEContainer


Run X (transpose) times X and write result to file
5. :~/integrated-linear-regression/bin$ ./specialMult -n 3 -p 3

- This creates left_product.ctext (will be read into SGX)


Run X (transpose) times y and write reuslt to file
6. :~/integrated-linear-regression/bin$ ./secondMult -n 3 -p 3 

- This creates right_product.ctext (will be read into SGX)


Compute inverse of left product (X'X) and multiply by right product (X'y) to get beta vector in SGX
7. :~/integrated-linear-regression/bin$ make SGX=1 -f mk_graphene inverse.manifest.sgx inverse.token pal_loader
8. :~/integrated-linear-regression/bin$ SGX=1 OMP_NUM_THREADS=1 ./pal_loader ./inverse -p 3


- Final result of linear regression (beta vector) ouputted to result/beta.txt
