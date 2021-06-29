Linear Regression - Integrated Homomorphic Encryption and Intel SGX with Graphene

Dependencies:
- PALISADE
  - [Github Repository](https://gitlab.com/ted537/palisade-development)
- Graphene
  - [Build Directions](https://graphene.readthedocs.io/en/latest/)

Build:
1. `make`
2. `cd bin`
3. `make SGX=1 -f mk_graphene inverse.manifest.sgx inverse.token pal_loader`

Generate random data with specified n (users) and p (feature values) <br/>
-   `./makeData -n [value] -p [value]`

    - This creates ctexts/dependent.ctext, ctexts/original.ctext, ctexts/tranpose.ctext
    - Also creates PALISADEContainer in container directory


Run X (transpose) times X and write result to file <br />
- `./firstMult -n [value] -p [value]`

    - This creates product_left.ctext (will be read into SGX)
    - Note: values of p and n must be <= the values specified in makeData


Run X (transpose) times y and write result to file <br />
-   `./secondMult -n [value] -p [value] `

    - This creates product_right.ctext (will be read into SGX)
    - Note: values of p and n must be <= the values specified in makeData


Compute inverse of left product (X'X) and multiply by right product (X'y) to get beta vector in SGX <br />
- `SGX=1 OMP_NUM_THREADS=1 ./pal_loader ./inverse -p [value] -n [value]` <br />


  - Final result of linear regression (beta vector) ouputted to result/beta.txt

To run overall test of linear regression and capture timing: <br />
- `./timing.py` <br />
    -  This will run a test of the linear regression program with varying p and n values
       -  p ranges from 2-12
       -  n ranges from 1,000,000 to 10,000,000 in 1,000,000 increments
    -  Results will be written to
       -  result/firstMult.csv
       -  result/secondMult.csv
       -  result/sgx.csv
