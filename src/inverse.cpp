#include "../include/PALISADEContainer.h"
#include "../include/matrix_operations.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
typedef std::chrono::high_resolution_clock clk;

void cofactors(vector<vector<double>>& m, vector<vector<double>>& cofs, int p, int q, int n);
double determinant(vector<vector<double>>& m, int n, int p);
void adjoint(vector<vector<double>>& m, vector<vector<double>>& a, int p);
void inverse(vector<vector<double>>& m, int p);

vector<vector<double>> multiply(vector<vector<double>>& m1, vector<vector<double>>& m2);

int main(int argc, char *argv[]) {
	auto beginning = clk::now();

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

	//read in PALISADEContainer made in makeData
	std::string ctr = "container";

	auto start = clk::now();
	PALISADEContainer pc(ctr, true);
	auto end = clk::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	std::cout << duration.count() << std::endl;

	// Read in XT times X data and decrypt
	vector<vector<double>> prod_left(p);
	std::ifstream left_f("ctexts/product_left.ctext");

	start = clk::now();
	for (int i = 0; i < p; i++) {
		for (int j = 0; j < p; j++) {
			Plaintext pt;
			Ciphertext<DCRTPoly> ct;
			Serial::Deserialize(ct, left_f, SerType::BINARY);
			pc.context->Decrypt(pc.sk, ct, &pt);
			std::vector<double> tmp = pt->GetRealPackedValue();
			double value = 0;
			for (int i = 0; i < tmp.size(); i++) { //have to sum all slots to complete matrix multiplication
				value += tmp[i];
			}
			prod_left[i].push_back(value);
		}
	}

	end = clk::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	std::cout << duration.count() << std::endl;

	// Read in XT times y data and decrypt
	vector<vector<double>> prod_right(p);
	std::ifstream right_f("ctexts/product_right.ctext");
	start = clk::now();
	for (int i = 0; i < p; i++) {
		Plaintext pt;
		Ciphertext<DCRTPoly> ct;
		Serial::Deserialize(ct, right_f, SerType::BINARY);
		pc.context->Decrypt(pc.sk, ct, &pt);
		std::vector<double> tmp = pt->GetRealPackedValue();
		double value = 0;
		for (int i =0; i < tmp.size(); i++) { //have to sum all slots to complete matrix multiplication
			value += tmp[i];
		}
		prod_right[i].push_back(value);
	}
	end = clk::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	std::cout << duration.count() << std::endl;

	
	// Run inverse algorithm on X'X
	start = clk::now();
	inverse(prod_left, p);
	end = clk::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	std::cout << duration.count() << std::endl;
	
	// Multiply (X'X)^-1 and (X'y) to get beta vector
	start = clk::now();
	vector<vector<double>> res = multiply(prod_left, prod_right);
	end = clk::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	std::cout << duration.count() << std::endl;

	// Write p x 1 beta vector to beta.txt
	FILE* beta = fopen("result/beta.txt", "w+");
	start = clk::now();
	if (!beta) {
		fprintf(stderr, "Unable to open beta.txt: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	for (int i = 0; i < res.size(); i++) {
		fprintf(beta, "%f\n", res[i][0]);
	}

	fclose(beta);
	end = clk::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	std::cout << duration.count() << std::endl;
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end-beginning);
	std::cout << duration.count() << std::endl;
		
	return 0;
}

vector<vector<double>> multiply(vector<vector<double>>& m1, vector<vector<double>>& m2) {
	//returns a matrix product of two matrices

	vector<vector<double>> result(m1.size(), vector<double>(m2[0].size()));
	#pragma omp parallel for collapse(2)
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
	//writes cofactor matrix of matrix m to cofs
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
	//returns determinant of matrix m
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
	//finds the adjoint matrix of matrix m and writes to a

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
	//finds the inverse of matrix m, exits silently if matrix is not invertible
	double det = determinant(m, p, p);
	if (det == 0) {
		return;
	}
	vector<vector<double>> a(p, vector<double>(p));
	adjoint(m, a, p);

	#pragma omp parallel for collapse(2)
	for (int i = 0; i < p; i++) {
		for (int j = 0; j < p; j++) {
			m[i][j] = a[i][j]/det;	
		}
	}
}
