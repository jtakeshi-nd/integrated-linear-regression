#include "../include/PALISADEContainer.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>

using namespace lbcrypto;

int main(int argc, char *argv[]) {

	// Parse Command Line Arguments 
	size_t p;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch(argv[i][1]) {
				case 'p':
					p = atoi(argv[++i]);
					break;
				default:
					exit(EXIT_FAILURE);
			}
		} else {
			exit(EXIT_FAILURE);
		}
	}

	// Read in p x 1 encrypted beta vector and write decrypted data to res.txt
	std::string ctr = "container";
	PALISADEContainer pc(ctr, true);
	std::ifstream beta("ctexts/beta.ctext");

	FILE *res = fopen("res.txt", "w+");
	if (!res) {
		fprintf(stderr, "Error opening res.txt: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < p; i++) {
		Plaintext pt;
		Ciphertext<DCRTPoly> ct;
		Serial::Deserialize(ct, beta, SerType::BINARY);
		pc.context->Decrypt(pc.sk, ct, &pt);
		std::vector<double> tmp = pt->GetRealPackedValue();
		double value=0;
		for (int i = 0; i < tmp.size(); i++) {
			value += tmp[i];
		}
		fprintf(res, "%f\n", value);
	} 

	fclose(res);

	return 0;
}
