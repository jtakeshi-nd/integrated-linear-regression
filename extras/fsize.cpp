// g++ -I /usr/local/include/palisade/binfhe -I /usr/local/include/palisade/cereal -I /usr/local/include/palisade/pke -I /usr/local/include/palisade/core -I /usr/local/include/palisade ./fsize.cpp -L /usr/local/lib -l PALISADEabe -l PALISADEbinfhe -l PALISADEcore -l PALISADEpke -l PALISADEsignature -Wl,-rpath=/usr/local/lib -pthread -fopenmp -O3
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
  string ofname = "out.dat";
  unsigned int depth = 1;
  unsigned int m = 8192;
  PALISADEContainer pc(depth, m);
  vector<double> user_vals(pc.slot_count(), 1.0f);
  Plaintext pt = pc.context->MakeCKKSPackedPlaintext(user_vals);
  ctext_typ ct = pc.context->Encrypt(pc.public_key(), pt);
  ofstream ofs(ofname);
  Serial::Serialize(ct, ofs, SerType::BINARY);
  cout << "Ring deg.: " << pc.ring_dimension() << endl;
  auto paramsQ = pc.context->GetElementParams()->GetParams();
  cout << "Number of moduli: " << paramsQ.size() << endl;
  for(const auto & x : paramsQ){
    auto xval = x->GetModulus();
    cout << '\t' << xval << ' ' << xval.GetMSB() << endl;   
  }

  return 0;
}
