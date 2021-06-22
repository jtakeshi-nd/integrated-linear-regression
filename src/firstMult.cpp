#include "../include/matrix_operations.h"
#include "../include/PALISADEContainer.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <chrono> 

typedef std::chrono::high_resolution_clock clk;

int main(int argc, char* argv[]){

    auto beginning = clk::now();
 
    size_t n;
    size_t p;
    std::string ctr = "container";
    bool pack = false;

    for(int i=1; i<argc;i++){
        if(argv[i][0]=='-'){
            switch(argv[i][1]){
                case 'n':
                    n = atoi(argv[++i]);
                break;
                case 'p':
                    p = atoi(argv[++i]);
                break;
                case 'c':
                    ctr = argv[++i];
                break;
                case 'z':
                    pack = true;
                break;
                default:
                    exit(EXIT_FAILURE);
            }
        }
        else
            exit(EXIT_FAILURE);
    }
    //read in container made by makeData
    auto start = clk::now();
    PALISADEContainer pc(ctr,true);
    size_t N = (pc.context->GetCyclotomicOrder() >> 2);
    size_t B = N/2; //need for batching
    auto end = clk::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    std::cout << duration.count() << std::endl;

    //hard coded names: could change
    std::ifstream original("ctexts/original.ctext");
    std::ifstream transpose("ctexts/transpose.ctext");

    if(pack){
        /* TODO: Write packing in the special Mult file
         * use the correct form of packing after Jon shows you */
    }
    else{

        ctext_matrix x(ceil((n*1.0)/B),std::vector<ctext_typ>(p));
        ctext_matrix xT(p,std::vector<ctext_typ>(ceil((n*1.0)/B)));

        //reading in the original X
        start = clk::now();
        for(int col=0;col<p;col++){
            for(int row=0;row<ceil((1.0*n)/B);row++){
                Serial::Deserialize(x[row][col],original,SerType::BINARY);
                
            }
        }
        end = clk::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
        std::cout << duration.count() << std::endl;

        //reading in xT        
        start = clk::now();
        for(int row=0;row<p;row++){
            for(int col=0;col<ceil((n*1.0)/B);col++){
                Serial::Deserialize(xT[row][col],transpose,SerType::BINARY);
            }
        }
        end = clk::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
        std::cout << duration.count() << std::endl;

        
        //calculate xT * X
        start = clk::now();
        ctext_matrix product = matrix_mult(pc,xT,x);
        end = clk::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
        std::cout << duration.count() << std::endl;
 


        /*ctext_matrix prod2 = matrix_mult_from_tranpose(pc,xT);
        std::cout << "HERE" <<std::endl;

        for(int i=0;i<prod2.size();i++){
            for(int j=0;j<prod2[0].size();j++){

                Plaintext ptx;
                pc.context->Decrypt(pc.sk,prod2[i][j],&ptx);
                double val =0;
                std::vector<double> tmp = ptx->GetRealPackedValue();
                for(int k=0; k<tmp.size(); k++){
                    val += tmp[k];
                }
                std::cout << val << " ";
            }
            std::cout << std::endl;
        } */



        std::ofstream q("ctexts/product_left.ctext");

        //writing product to file for next steps
        start = clk::now();
        for(int i=0;i<product.size();i++){
            for(int j=0;j<product[0].size();j++){
                Serial::Serialize(product[i][j],q,SerType::BINARY);
            }
        }
        end = clk::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
        std::cout << duration.count() << std::endl;
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end-beginning);
        std::cout << duration.count() << std::endl;
    }
  
}
