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

vector<vector<double>> multiply(vector<vector<double>>& m1, vector<vector<double>>& m2);

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

	// Read in X' times X data and decrypt
	std::string ctr = "container";
	PALISADEContainer pc(ctr, true);
	vector<vector<double>> prod_left(p);
	std::ifstream left_f("ctexts/product_left.ctext");

	for (int i = 0; i < p; i++) {
		for (int j = 0; j < p; j++) {
			Plaintext pt;
			Ciphertext<DCRTPoly> ct;
			Serial::Deserialize(ct, left_f, SerType::BINARY);
			pc.context->Decrypt(pc.sk, ct, &pt);
			std::vector<double> tmp = pt->GetRealPackedValue();
			double value = 0;
			for (int i = 0; i < tmp.size(); i++) {
				value += tmp[i];
			}
			prod_left[i].push_back(value);
		}
	}

	// Read in X' times y data and decrypt
	vector<vector<double>> prod_right(p);
	std::ifstream right_f("ctexts/product_right.ctext");

	for (int i = 0; i < p; i++) {
		Plaintext pt;
		Ciphertext<DCRTPoly> ct;
		Serial::Deserialize(ct, right_f, SerType::BINARY);
		pc.context->Decrypt(pc.sk, ct, &pt);
		std::vector<double> tmp = pt->GetRealPackedValue();
		double value = 0;
		for (int i =0; i < tmp.size(); i++) {
			value += tmp[i];
		}
		prod_right[i].push_back(value);
	}

	
	// Run inverse algorithm on X'X
	inverse(prod_left, p);
	
	// Multiply (X'X)^-1 and (X'y) to get beta vector
	vector<vector<double>> res = multiply(prod_left, prod_right);

	// Write p x 1 beta vector to beta.txt
	FILE* beta = fopen("result/beta.txt", "w+");
	if (!beta) {
		fprintf(stderr, "Unable to open beta.txt: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	for (int i = 0; i < res.size(); i++) {
		fprintf(beta, "%f\n", res[i][0]);
	}
	fclose(beta);
		
	return 0;
}

vector<vector<double>> multiply(vector<vector<double>>& m1, vector<vector<double>>& m2) {


	vector<vector<double>> result(m1.size(), vector<double>(m2[0].size()));
	for (int i = 0; i < m1.size(); i++) {
		for (int j = 0; j < m2[0].size(); j++) {
			for (int k = 0; k < m1[0].size(); k++) {
				result[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
	return result;
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
