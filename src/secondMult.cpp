#include "../include/matrix_operations.h"
#include "../include/PALISADEContainer.h"
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>

using namespace lbcrypto;

int main(int argc, char* argv[]){
    size_t p;
    size_t n;
    std::string container = "container";

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

    PALISADEContainer pc(container,true);

    size_t N = pc.context->GetCyclotomicOrder() >>2;
    size_t B = N/2;


    std::ifstream transpose("ctexts/transpose.ctext");
    std::ifstream dependent("ctexts/dependent.ctext");

    ctext_matrix xT(p,std::vector<ctext_typ>((floor(n/B) ? floor(n/B) : 1)));

    //reading in xT
    for(int row=0;row<p;row++){
        for(int col=0;col<(floor(n/B) ? floor(n/B) : 1);col++){
            Serial::Deserialize(xT[row][col],transpose,SerType::BINARY);
        }
    }

    

    ctext_matrix y((floor(n/B) ? floor(n/B) : 1), std::vector<ctext_typ>(1));

    for(int index =0; index < (floor(n/B) ? floor(n/B) : 1); index++){
        Serial::Deserialize(y[index][0],dependent,SerType::BINARY);
    }

    ctext_matrix quotient = matrix_mult(pc,xT,y);

    std::ofstream out("ctexts/product_right.ctext");

    for(int i=0; i<quotient.size(); i++){
        for(int j=0;j<quotient[0].size();j++){
            Serial::Serialize(quotient[i][j],out,SerType::BINARY);
        }
    }


}
