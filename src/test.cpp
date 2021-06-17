#include "../include/PALISADEContainer.h"
#include "../include/matrix_operations.h"
#include <vector>

using namespace lbcrypto;

int main(){
    PALISADEContainer pc(1024,1,1024);

    std::vector<double> test = {1,2,3};

    Plaintext ptext = pc.context->MakeCKKSPackedPlaintext(test);

    ctext_typ ctext = pc.context->Encrypt(pc.pk,ptext);

    Plaintext decrypted;

    pc.context->Decrypt(pc.sk,ctext,&decrypted);

    std::cout << decrypted << std::endl;


}