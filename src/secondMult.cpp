#include "../include/matrix_operations.h"
#include "../include/PALISADEContainer.h"
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <chrono>

using namespace lbcrypto;
typedef std::chrono::high_resolution_clock clk;

int main(int argc, char* argv[]){
    auto beginning = clk::now();
    size_t p;
    size_t n;
    std::string container = "container"; //default name for directory container PALISADEConainer

    for(int i=1;i<argc;i++){
        if(argv[i][0]=='-'){
            switch(argv[i][1]){
                case 'n': 
                    n = atoi(argv[++i]);
                break;
                case 'p':
                    p = atoi(argv[++i]);
                break;
                case 'c':
                    container = argv[++i];
                break;
                default:
                    exit(EXIT_FAILURE);
                break;
            }

        }
        else
            exit(EXIT_FAILURE);
    }

    //read in container created in makeData
    auto start = clk::now();
    PALISADEContainer pc(container,true);
    auto end = clk::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    std::cout << duration.count() << std::endl;

    size_t N = pc.context->GetCyclotomicOrder() >>2;
    size_t B = N/2;

    //hard coded names: could change
    std::ifstream transpose("ctexts/transpose.ctext");
    std::ifstream dependent("ctexts/dependent.ctext");

    ctext_matrix xT(p,std::vector<ctext_typ>(ceil((n*1.0)/B)));

    //reading in xT
    start = clk::now();
    for(int col=0;col<ceil((n*1.0)/B);col++){
        for(int row=0;row<p;row++){
            Serial::Deserialize(xT[row][col],transpose,SerType::BINARY);
        }
        transpose.clear();
        transpose.seekg(0,std::ios::beg);
    }
    end = clk::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    std::cout << duration.count() << std::endl;

    
    //reading in y
    ctext_matrix y(ceil((1.0*n)/B), std::vector<ctext_typ>(1));
    start = clk::now();
    for(int i=0;i<ceil((n*1.0)/B);i++){
        Serial::Deserialize(y[i][0],dependent,SerType::BINARY);
        dependent.clear();
        dependent.seekg(0,std::ios::beg);
    }
    end = clk::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    std::cout << duration.count() << std::endl;

    start = clk::now();
    //calculating the product
    ctext_matrix product = matrix_mult(pc,xT,y);
    end = clk::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    std::cout << duration.count() << std::endl;

    std::ofstream out("ctexts/product_right.ctext");

    start = clk::now();
    //writing to file
    for(int i=0; i<product.size(); i++){
        for(int j=0;j<product[0].size();j++){
            Serial::Serialize(product[i][j],out,SerType::BINARY);
        }
    }
    end = clk::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
    std::cout << duration.count() << std::endl;
    //total time to run execution
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end-beginning);
    std::cout << duration.count() << std::endl;


}
