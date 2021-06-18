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
	inverse(data, p);

	// Encrypt inverted matrix and output to file
	std::ofstream inv("ctexts/inv.ctext");
	size_t N = pc.context->GetCyclotomicOrder() >>2;
	size_t B = N/2;

	ctext_matrix matrix(p,std::vector<ctext_typ>((floor(p/B) ? floor(p/B) : 1 )));

	for(int col=0; col<=floor((p*1.0)/B);col++){
        for(int row=0; row<p;row++){
            //initializing ciphertexts for x
            std::vector<double> tmp = {0};
            matrix[row][col] = pc.context->Encrypt(pc.pk,pc.context->MakeCKKSPackedPlaintext(tmp));
        }
    }

	for(int j=0; j<p;j++){ //create the ciphertexts to be batched 
        //batching in col
        for(int i=0; i<p;i++){
            

            int col = floor((j*1.0)/B);
            int row = i;

            std::vector<double> tmp(n,0);
            tmp[j] = data[i][j];

            Plaintext ptx = pc.context->MakeCKKSPackedPlaintext(tmp);
            
            ctext_typ ctx = pc.context->Encrypt(pc.pk,ptx);

            //ctx = rotate(pc,ctx,i%N);
            pc.context->EvalAddInPlace(matrix[row][col],ctx);
        }
    }

	for(int col=0; col<=floor((p*1.0)/B);col++){
        for(int row=0; row<p;row++){
            Serial::Serialize(matrix[row][col],inv,SerType::BINARY);
        }
    }
	
	/*for (int i = 0; i < p; i++) {
		for (int j = 0; j < p; j++) {
			std::vector<double> tmp = {data[i][j]};
			Plaintext pt = pc.context->MakeCKKSPackedPlaintext(tmp);
			Serial::Serialize(pc.context->Encrypt(pc.pk, pt), inv, SerType::BINARY);
		}
	} */

	std::cout << "Wrote inverse to ctexts/inverse.ctext" << std::endl;
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
