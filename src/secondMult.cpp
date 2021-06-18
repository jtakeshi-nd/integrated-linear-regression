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


    std::ifstream inv("ctexts/inverse.ctext");
    std::ifstream transpose("ctexts/transpose.ctext");
    std::ifstream dependent("ctexts/dependent");

    ctext_matrix inverse(p,std::vector<ctext_typ>(p));

    //reading in inverse of X*xT
    for(int j=0;j<p;j++){
        for(int i=0;i<p;i++){
            Serial::Deserialize(inverse[i][j],inv,SerType::BINARY);
        }
    }

    ctext_matrix xT(p,std::vector<ctext_typ>(((n/B) ? n/B : 1)));

    //reading in xT
    for(int row=0;row<p;row++){
        for(int col=0;col<(floor(n/N)+1);col++){
            Serial::Deserialize(xT[row][col],transpose,SerType::BINARY);
        }
    }

    /* TODO: Read in dependent variable */

    ctext_matrix quotient = matrix_mult(pc,inverse,xT);

    ctext_matrix beta = matrix_mult(pc,quotient,y);

    std::ofstream out("ctexts/beta.ctext");
    for(int i=0; i<beta.size(); i++){
        for(int j=0;j<beta[0].size();j++){
            Serial::Serialize(beta[i][j],out,SerType::BINARY);
        }
    } 

    



}