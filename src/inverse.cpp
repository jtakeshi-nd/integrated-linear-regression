#include "../include/PALISADEContainer.h"
#include "../include/matrix_operations.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>

using namespace lbcrypto;

void cofactors(vector<vector<double>>& m, vector<vector<double>>& cofs, int p, int q, int n);

double determinant(vector<vector<double>>& m, int n, int p);

void adjoint(vector<vector<double>>& m, vector<vector<double>>& a, int p);

void inverse(vector<vector<double>>& m, int p);

int main(int argc, char *argv[]) {
	/*PALISADEContainer pc(1024, 1, 1024);

	std::vector<double> test = {1, 2, 3};

	Plaintext ptext = pc.context->MakeCKKSPackedPlaintext(test);

	ctext_typ ctext = pc.context->Encrypt(pc.pk, ptext);

	Plaintext decrypted;

	pc.context->Decrypt(pc.sk, ctext, &decrypted);

	std::vector<double> raw_data = decrypted->GetRealPackedValue();

	std::cout << raw_data << std::endl;*/

	// Parse Command Line Arguments 
	size_t n, p;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch(argv[i][1]) {
				case 'n':
					n = atoi(argv[++i]);
					break;
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

	std::cout << "Running" << std::endl;
	// Read in X tranpose times X data and decrypt
	std::string ctr = "container";
	PALISADEContainer pc(ctr, true);

	vector<vector<double>> data(p);
	std::ifstream mult("ctexts/quotient.ctext");
	for (int i = 0; i < p; i++) {
		for (int j = 0; j < p; j++) {
			Plaintext pt;
			Ciphertext<DCRTPoly> ct;
			Serial::Deserialize(ct, mult, SerType::BINARY);
			pc.context->Decrypt(pc.sk, ct, &pt);
			std::vector<double> tmp = pt->GetRealPackedValue();
			double value = 0;
			for (int i = 0; i < tmp.size(); i++) {
				value += tmp[i];
			}
			data[i].push_back(value);
		}
	}

	// Run inverse program


	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].size(); j++) {
			std::cout << data[i][j] << " ";
		}
		std::cout << std::endl;
	}


	// Encrypt inverted matrix and output to file


	return 0;
}

void cofactors(vector<vector<double>>& m, vector<vector<double>>& cofs, int p, int q, int n) {
	int i = 0;
	int j = 0;
	for (int r = 0; r < n; r++) {
		for (int c = 0; c < n; c++) {
			if (r != p && c != q) {
				cofs[i][j] = m[r][c];
				j++;
				if (j == n - 1) {
					j = 0; 
					i++;
				}
			}	

		}
	}
}	

double determinant(vector<vector<double>>& m, int n, int p) {
	double det = 0;
	if (n == 1) {
		return m[0][0];
	}

	vector<vector<double>> cofs(p, vector<double>(p));
	int sign = 1;
	for (int i = 0; i < n; i++) {
		cofactors(m, cofs, 0, i, n);
		det += sign * m[0][i] * determinant(cofs, n - 1, p);
		sign = -sign;
	}

	return det;
}

void adjoint(vector<vector<double>>& m, vector<vector<double>>& a, int p) {

	if (p == 1) {
		a[0][0] = 1;
		return;
	}
	int sign = 1;
	vector<vector<double>> cofs(p, vector<double>(p));

	for (int i = 0; i < p; i++) {
		for (int j = 0; j < p; j++) {
			cofactors(m, cofs, i, j, p);
			sign = ((i + j) % 2 == 0) ? 1 : -1;
			a[j][i] = sign * determinant(cofs, p - 1, p - 1);
		}
	}
}

void inverse(vector<vector<double>>& m, int p) {
	double det = determinant(m, p, p);
	if (det == 0) {
		return;
	}
	vector<vector<double>> a(p, vector<double>(p));
	adjoint(m, a, p);

	for (int i = 0; i < p; i++) {
		for (int j = 0; j < p; j++) {
			m[i][j] = a[i][j]/det;	
		}
	}
}
