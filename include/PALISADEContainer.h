#ifndef PALISADECONTAINER_H
#define PALISADECONTAINER_H

//#include <iostream>
#include <sstream>
#include <cmath>

//Make sure you get the include directories right, or this will error
#include <palisade.h>
#include <palisade/pke/palisade.h>
#include <ciphertext-ser.h>
#include <cryptocontext.h>
#include <cryptocontext-ser.h>
#include <pubkeylp-ser.h>
#include <scheme/bgvrns/bgvrns-ser.h>
//#include <utils/serialize-binary.h>
#include <utils/serial.h>

using namespace lbcrypto;

#define CONTEXT_FILE "context"
#define PUBKEY_FILE "pubkey"
#define EMKEY_FILE "emkey"
#define SECKEY_FILE "seckey"
#define ROTKEY_FILE "rotkey"

class PALISADEContainer{

public:
  /*
  static const unsigned int DEGREE_DEFAULT = 8192;
  static const unsigned int PLAIN_MOD_SIZE_DEFAULT = 20;
  static const unsigned int PLAIN_DEFAULT = 1 << PLAIN_MOD_SIZE_DEFAULT;
  */
  static const constexpr double SIGMA = 3.2;
  static const SecurityLevel SECURITY = HEStd_128_classic;
  static const constexpr SerType::SERBINARY & SERIAL_FORMAT = SerType::BINARY; //SerType is not recognized...
  static const unsigned int BATCHSIZE_DEFAULT = 64;
  //static const unsigned int M_DEFAULT = 32768;

  //Change these 2 together
  static const constexpr unsigned int PLAIN_DEFAULT = 65537; //This must be prime!
  //TODO fix this up - linker error with static const apparently
  //const unsigned int PLAIN_DEFAULT_MIN_PRIMROOT = 6;
  static const constexpr unsigned int DEPTH_DEFAULT = 4;

  static const unsigned int SCALE_FACTOR_BITS = 50;

  CryptoContext<DCRTPoly> context = nullptr;
  LPPublicKey<DCRTPoly> pk = nullptr;
  LPPrivateKey<DCRTPoly> sk = nullptr;

  bool _has_secret_key = false;
  
  unsigned int _plain_modulus = 0;
  unsigned int depth = 0;

  //CKKS constructor - quick+dirty impl., not parameterized
  //Default for m should be 8192
  PALISADEContainer(/* unsigned int m, */ unsigned int depth_in /*, unsigned int p*/){
    depth = depth_in;
    _plain_modulus = 0; //Not used in CKKS
    //unsigned int batchSize = m >> 2;
    uint32_t batchSize = 8; //Will be overwritten later, so any value can be used

    //Simpler method based on https://gitlab.com/palisade/palisade-release/-/blob/master/src/pke/examples/simple-real-numbers.cpp
    this->context = CryptoContextFactory<DCRTPoly>::genCryptoContextCKKS(
          depth, SCALE_FACTOR_BITS, batchSize, SECURITY);
    context->GetEncodingParams()->SetBatchSize(this->ring_dimension()/2);


    context->Enable(ENCRYPTION);
    context->Enable(SHE);
    context->Enable(LEVELEDSHE);
    

    LPKeyPair<DCRTPoly> keyPair = context->KeyGen();
    if (!keyPair.good()) {
      std::cerr << "Key generation failed!" << std::endl;
      exit(1);
    }
    pk = keyPair.publicKey;
    sk = keyPair.secretKey;

    
    context->EvalMultKeysGen(sk);
    this->_has_secret_key = true;

  }

  PALISADEContainer(unsigned int depth_in, unsigned int m /*, unsigned int p*/){
    depth = depth_in;
    _plain_modulus = 0; //Not used in CKKS
    unsigned int batchSize = m / 4;
    
    unsigned int primes = depth+1; //ok for thousands of additions, not millions

    context =
      CryptoContextFactory<DCRTPoly>::genCryptoContextCKKSWithParamsGen(
          m, primes, /*numPrimes*/
          SCALE_FACTOR_BITS, 10, /*relinWindow*/
          batchSize,           /*batch size*/
          OPTIMIZED, depth /*depth*/);


    context->Enable(ENCRYPTION);
    context->Enable(SHE);
    context->Enable(LEVELEDSHE);
    

    LPKeyPair<DCRTPoly> keyPair = context->KeyGen();
    if (!keyPair.good()) {
      std::cerr << "Key generation failed!" << std::endl;
      exit(1);
    }
    pk = keyPair.publicKey;
    sk = keyPair.secretKey;

    
    context->EvalMultKeysGen(sk);
    this->_has_secret_key = true;
  }

  //This will fail if called on a nonexistent or empty file
  PALISADEContainer(const std::string & containername, const bool load_sk){
		if(this->deserialize(containername, load_sk)){
      std::cerr << "ERROR: failed to load PALISADEContainer from file " << containername << std::endl;
      exit(0);
    }
    this->_has_secret_key = load_sk;
  }

  inline LPPublicKey<DCRTPoly> public_key() const {
    return pk;
  }

  inline LPPrivateKey<DCRTPoly> secret_key() const {
    return sk;
  }
  

  int serialize(const std::string & ofname, const bool save_sk){
    //Context
    std::string context_name = ofname + '/' + CONTEXT_FILE;
    if(!Serial::SerializeToFile(context_name, context, SERIAL_FORMAT)){
      std::cerr << "Failed to write context: " << context_name << std::endl;
      return 1;
    }
    //Public key 
    std::string pubkey_name = ofname + '/' + PUBKEY_FILE;
    if(!Serial::SerializeToFile(pubkey_name, pk, SERIAL_FORMAT)){
      std::cerr << "Failed to write pubkey: " << pubkey_name << std::endl;
      return 1;
    }
    
    //Relin. keys
    std::string rlk_file = ofname + '/' + EMKEY_FILE;
    std::ofstream os(rlk_file, std::ios::out | std::ios::binary);
    if(!os.is_open()){
      std::cerr << "ERROR: failed to open RLK file " << rlk_file << std::endl;
      return 1;
    }
    if(!this->context->SerializeEvalMultKey(os, SERIAL_FORMAT)){
      std::cerr << "Failed to write emkey: " << rlk_file << std::endl;
      return 1;
    }
    os.close();
    //Secret key always saved last
    if(save_sk){
      std::string seckey_name = ofname + '/' + SECKEY_FILE;
      if(!Serial::SerializeToFile(seckey_name, sk, SERIAL_FORMAT)){
        std::cerr << "Failed to write seckey: " << seckey_name << std::endl;
        return 1;
      }
    }
    //With separate files, the order doesn't matter
    return 0;
  }

  int deserialize(const std::string & fname, const bool load_sk){
    //Context
    std::string context_name = fname + '/' + CONTEXT_FILE;
    if(!Serial::DeserializeFromFile(context_name, context, SERIAL_FORMAT)){
      return 1;
    }
    //Public key 
    std::string pubkey_name = fname + '/' + PUBKEY_FILE;
    if(!Serial::DeserializeFromFile(pubkey_name, pk, SERIAL_FORMAT)){
      return 1;
    }
    
    //Relin. keys
    context->ClearEvalMultKeys();
    std::string rlk_file = fname + '/' + EMKEY_FILE;
    std::ifstream os(rlk_file, std::ios::in | std::ios::binary); //Bad naming
    if(!os.is_open()){
      return 1;
    }
    if(!this->context->DeserializeEvalMultKey(os, SERIAL_FORMAT)){
      return 1;
    }
    os.close();
    //Secret key always saved last, if all serialized to one file (not currently implemented)
    if(load_sk){
      std::string seckey_name = fname + '/' + SECKEY_FILE;
      if(!Serial::DeserializeFromFile(seckey_name, sk, SERIAL_FORMAT)){
        return 1;
      }
    }
    return 0;
  }
  
  inline unsigned int slot_count() const {
    return context->GetEncodingParams()->GetBatchSize();
  }

  //Returns t from batching
  inline long plain_modulus() const {
  	return context->GetEncodingParams()->GetPlaintextModulus();
  }

  inline long ring_dimension() const {
    return context->GetRingDimension();
  }

  inline long cyclotomic_order() const {
    return context->GetCyclotomicOrder();
  }

  inline bool has_secret_key() const {
    return this->_has_secret_key;
  }
  
};












































#endif
