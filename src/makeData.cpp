#include <palisade.h>
#include <ciphertext-ser.h>
#include <cryptocontext-ser.h>
#include <pubkeylp-ser.h>
#include <scheme/ckks/ckks-ser.h>
#include "../include/PALISADEContainer.h"
#include "../include/matrix_operations.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <random>
#include <chrono>
#include <vector>

using namespace lbcrypto;
void usage(int);
ctext_typ rotate(const PALISADEContainer&, const ctext_typ&, const int&);
void transpose(const PALISADEContainer&, const std::vector<std::vector<double>>&, const size_t&, const size_t&, const size_t&);
void original(const PALISADEContainer&, const std::vector<std::vector<double>>&, const size_t&, const size_t&, const size_t&);

int main(int argc, char* argv[]){
    size_t p = 10; //default value for number of parameters
    size_t n = 10000; //default value for number of data points
    std::string fileName("data.txt"); //default name for the output file
    size_t m = 8192;

    for(int i=1; i < argc; i++){
        if(argv[i][0] == '-')
            switch (argv[i][1]){
                case 'h':
                    usage(0);
                    break;
                case 'p':
                    p = atoi(argv[++i]);
                    break;
                case 'n':
                    n = atoi(argv[++i]);
                    break;
                case 'f':
                    fileName = argv[++i];
                    break;
                default:
                    usage(1);
                    break;
            }
        else
            usage(2);
    }

    auto d = std::chrono::high_resolution_clock::now().time_since_epoch();
    long unsigned int seed = d.count();
    std::mt19937 numbgen(seed); //initialize the random number generator with a seed
    size_t N = (m>>2)/2;

    PALISADEContainer pc(m,4,1024);
    std::vector<std::vector<double>> rawValues;

    for(int i=0; i<n; i++){ // create the raw data values to be batched into ciphertexts
        double tmp = numbgen()%10;
        rawValues.push_back({tmp});
        std::cout << rawValues[i][0] << " ";
        for(int j=1; j<p; j++){
            tmp = numbgen()%10;
            rawValues[i].push_back(tmp);
            std::cout << tmp << " ";
        }
        std::cout << std::endl;
    }

    transpose(pc,rawValues,p,n,N);
    std::cout << "transpose" << std::endl;
    original(pc,rawValues,p,n,N);
    std::cout << "original" << std::endl;

    std::vector<ctext_typ> y(floor(n/N) ? floor(n/N) : 1);

    std::ofstream yfile("ctexts/dependent.ctext");

    for(int i=0;i<(floor((i*1.0)/N) ? floor((i*1.0)/N) : 1); i++){
        std::vector<double> tmp(N,0);
        y[i] = pc.context->Encrypt(pc.pk,pc.context->MakeCKKSPackedPlaintext(tmp));
    }


    for(int i=0;i<n;i++){

        int index = floor((i*1.0)/N);
        std::vector<double> tmp(N,0);

        
        //prep the random value
        tmp[i%N] = (double)(numbgen()%10);
        std::cout << tmp[i%N] << std::endl;
        Plaintext ptx = pc.context->MakeCKKSPackedPlaintext(tmp);
        ctext_typ ctx = pc.context->Encrypt(pc.pk,ptx);

        //perform rotation
        //ctx = rotate(pc,ctx,i%N);
        pc.context->EvalAddInPlace(y[index],ctx);
    } 
    for(int i=0;i<(floor(n/N) ? floor(n/N) : 1) ;i++){
        Serial::Serialize(y[i],yfile,SerType::BINARY);
    }


    pc.serialize("container",true);


}

void original(const PALISADEContainer& pc, const std::vector<std::vector<double>>& rawValues, const size_t& p, const size_t& n, const size_t& N){
    ctext_matrix matrix(n,std::vector<ctext_typ>(p));

    for(int col=0; col<p;col++){
        for(int row=0; row<=floor((n*1.0)/N);row++){
            //initializing ciphertexts for x
            std::vector<double> tmp = {0};
            matrix[row][col] = pc.context->Encrypt(pc.pk,pc.context->MakeCKKSPackedPlaintext(tmp));
        }
    }


    for(int j=0; j<p;j++){ //create the ciphertexts to be batched 
        //batching in col
        for(int i=0; i<n;i++){
            

            int col = j;
            int row = floor((i*1.0)/N);

            std::vector<double> tmp(n,0);
            tmp[i] = rawValues[i][j];

            Plaintext ptx = pc.context->MakeCKKSPackedPlaintext(tmp);
            
            ctext_typ ctx = pc.context->Encrypt(pc.pk,ptx);

            //ctx = rotate(pc,ctx,i%N);
            pc.context->EvalAddInPlace(matrix[row][col],ctx);
        }
    }

    std::ofstream mat("ctexts/original.ctext");
    for(int col=0; col<p;col++){
        for(int row=0; row<=floor((n*1.0)/N);row++){
            Serial::Serialize(matrix[row][col],mat,SerType::BINARY);
        }
    }
}

void transpose(const PALISADEContainer& pc, const std::vector<std::vector<double>>& rawValues,const size_t& p, const size_t& n, const size_t& N){

    ctext_matrix matrix(p,std::vector<ctext_typ>(n));

    for(int row=0;row<p;row++){
        //initializing the ciphertexts for x^T
        for(int col=0;col<=floor((n*1.0)/N);col++){
            std::vector<double> tmp = {0};
            matrix[row][col] = pc.context->Encrypt(pc.pk,pc.context->MakeCKKSPackedPlaintext(tmp));
        }
    }


    for(int j=0; j<p;j++){ //create the ciphertexts to be batched 
        //batching in row
        for(int i=0; i<n;i++){
            
            int col = floor((i*1.0)/N);
            int row = j;

            std::vector<double> tmp(n,0);

            tmp[i] = rawValues[i][j];
            Plaintext ptx = pc.context->MakeCKKSPackedPlaintext(tmp);
            ctext_typ ctx = pc.context->Encrypt(pc.pk,ptx);

            //ctx = rotate(pc,ctx,i%N);
            pc.context->EvalAddInPlace(matrix[row][col],ctx);
        }

        
    }

    std::ofstream tpose("ctexts/transpose.ctext");
    for(int row=0; row<p;row++){
        for(int col=0; col<=floor((n*1.0)/N);col++){
            Serial::Serialize(matrix[row][col],tpose,SerType::BINARY);
        }
    }


}

void usage(int e){
    std::cout << "makeData: Produces synthetic data for the PALISADE linear regression library" << std::endl;
    std::cout << "Usage: ./makeData [options]" << std::endl;
    std::cout << "Options: " << std::endl;
    std::cout << "-h : help" << std::endl;
    std::cout << "-p [integer]: change the number of random parameters" << std::endl;
    std::cout << "-n [integer]: change the number of data points" << std::endl;
    std::cout << "-f [string]: name of the output file" << std::endl;
    if(e)
        exit(EXIT_FAILURE);
    else
        exit(EXIT_SUCCESS);
}