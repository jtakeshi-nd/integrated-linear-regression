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


    std::ifstream inv("ctexts/inv.ctext");
    std::ifstream transpose("ctexts/transpose_col.ctext");
    std::ifstream dependent("ctexts/dependent_unpacked.ctext");

    ctext_matrix inverse(p,std::vector<ctext_typ>((floor(p/B) ? floor(p/B) : 1 )));

    for(int col=0; col<=floor((p*1.0)/B);col++){
        for(int row=0; row<p;row++){
            Serial::Deserialize(inverse[row][col],inv,SerType::BINARY);
        }
    }

    //reading in inverse of X*xT
    /*for(int j=0;j<p;j++){
        for(int i=0;i<p;i++){
            Serial::Deserialize(inverse[i][j],inv,SerType::BINARY);
        }
    } */
    std::cout << "read inverse" << std::endl;

    ctext_matrix xT(( (p/B) ? p/B : 1),std::vector<ctext_typ>(n));

    //reading in xT
    for(int row=0;row<(floor(p/B) ? floor(p/B) : 1);row++){
        for(int col=0;col<n;col++){
            Serial::Deserialize(xT[row][col],transpose,SerType::BINARY);
        }
    }


    std::cout << "read transpose" << std::endl;
    

    /* TODO: Read in dependent variable */
    /*ctext_matrix y((floor(n/B) ? floor(n/B) : 1), std::vector<ctext_typ>(1));

    for(int index =0; index < (floor(n/B) ? floor(n/B) : 1); index++){
        std::cout << index << std::endl;
        Serial::Deserialize(y[index][0],dependent,SerType::BINARY);
    } */

    ctext_matrix y(n,std::vector<ctext_typ>(1));

    for(int i=0; i<n; i++){
        Serial::Deserialize(y[i][0],dependent,SerType::BINARY);
    }

    std::cout << y.size() << " " << y[0].size() << std::endl;

    std::cout << "read y" << std::endl;

    ctext_matrix quotient = matrix_mult(pc,inverse,xT);

    /*for(int i=0; i<quotient.size();i++){
        for(int j=0; j<quotient[0].size(); j++){
            Plaintext ptx;
            pc.context->Decrypt(pc.sk,quotient[i][j],&ptx);
            std::vector<double> tmp = ptx->GetRealPackedValue();
            double value =0;
            for(int k=0; k<tmp.size();k++){
                value += tmp[k];
            }
            std::cout << value << " ";
        }
        std::cout << std::endl;
    } */

    ctext_matrix beta = matrix_mult(pc,quotient,y);

    for(int i=0; i<beta.size(); i++){
        Plaintext ptx; 
        pc.context->Decrypt(pc.sk,beta[i][0],&ptx);
        std::vector<double> tmp = ptx->GetRealPackedValue();
        double value=0;
        for(int j=0;j<tmp.size();j++){
            value += tmp[j];
        }
        std::cout << value << std::endl; 
    }

    std::ofstream out("ctexts/beta.ctext");
    for(int i=0; i<beta.size(); i++){
        for(int j=0;j<beta[0].size();j++){
            Serial::Serialize(beta[i][j],out,SerType::BINARY);
        }
    } 

    



}