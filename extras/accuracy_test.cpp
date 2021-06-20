// g++ -I /usr/local/include/palisade/binfhe -I /usr/local/include/palisade/cereal -I /usr/local/include/palisade/pke -I /usr/local/include/palisade/core -I /usr/local/include/palisade ./accuracy_test.cpp -L /usr/local/lib -l PALISADEabe -l PALISADEbinfhe -l PALISADEcore -l PALISADEpke -l PALISADEsignature -Wl,-rpath=/usr/local/lib -pthread -fopenmp -O3 -o accuracy_test

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <math.h>

using namespace std;


#include <ciphertext-ser.h>
#include <cryptocontext-ser.h>
#include <pubkeylp-ser.h>
#include <scheme/ckks/ckks-ser.h>
#include <palisade.h>
#include "../include/PALISADEContainer.h"

using ctext_typ = Ciphertext<DCRTPoly>;

int main(int argc, char ** argv){
  unsigned int num_additions = 10000000;
  
  unsigned int depth = 1;
  unsigned int m = 8192;
  PALISADEContainer pc(depth, m);
  vector<double> user_vals(pc.slot_count(), 0.1f);
  
  unsigned int total_sums = std::ceil((double)num_additions/pc.slot_count());
  
  Plaintext pt = pc.context->MakeCKKSPackedPlaintext(user_vals);
  ctext_typ ct0 = pc.context->Encrypt(pc.public_key(), pt);
  //ctext_typ ct1 = pc.context->Encrypt(pc.public_key(), pt);
  ctext_typ ct2 = pc.context->EvalMult(ct0, ct0);
  ctext_typ ct3 = ct2;
  
  for(unsigned int i = 0; i < total_sums/4; i++){
    ct3 += ct2;
  }
  ct3 += ct3;
  ct3 += ct3;
  
  Plaintext result_pt; 
  pc.context->Decrypt(pc.secret_key(), ct3, &result_pt);
  //const vector<double> & result = pt->GetPackedValue();
  cout << result_pt << endl;
  cout << "Total additions done: " << total_sums;

  return 0;
}
