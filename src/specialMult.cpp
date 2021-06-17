#include "../include/matrix_operations.h"
#include "../include/PALISADEContainer.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>


int main(int argc, char* argv[]){
 
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
    PALISADEContainer pc(ctr,true);
    size_t N = (pc.context->GetCyclotomicOrder() >> 2);
    size_t B = N/2;

    std::ifstream original("ctexts/original.ctext");
    std::ifstream transpose("ctexts/transpose.ctext");
    std::ifstream dependent("ctexts/dependent.ctext");

    if(pack){

    }
    else{
        ctext_matrix x((n/B) ? n/B : 1,std::vector<ctext_typ>(p));
        ctext_matrix xT(p,std::vector<ctext_typ>(((n/B) ? n/B : 1)));

        for(int col=0;col<p;col++){
            for(int row=0;row<(floor(n/N)+1);row++){
                Serial::Deserialize(x[row][col],original,SerType::BINARY);
                
            }
        }

        for(int row=0;row<p;row++){
            for(int col=0;col<(floor(n/N)+1);col++){
                Serial::Deserialize(xT[row][col],transpose,SerType::BINARY);
            }
        }
        
        ctext_matrix quotient = matrix_mult(pc,xT,x);

        std::ofstream q("quotient.ctext");

        std::cout << quotient.size() << " " << quotient[0].size() << std::endl;
        for(int i=0;i<quotient.size();i++){
            for(int j=0;j<quotient[0].size();j++){
                Serial::Serialize(quotient[i][j],q,SerType::BINARY);
            }
        }




    }



    
}