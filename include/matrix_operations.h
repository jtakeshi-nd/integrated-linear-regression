#ifndef MATRIX_OP_H
#define MATRIX_OP_H

    #include <vector>
    #include <iostream>
    #include <palisade.h>
    #include <ciphertext-ser.h>
    #include <cryptocontext-ser.h>
    #include <pubkeylp-ser.h>
    #include <scheme/ckks/ckks-ser.h>
    #include "PALISADEContainer.h"

    using namespace lbcrypto;
    using ctext_typ = Ciphertext<DCRTPoly>;
    using ctext_matrix = std::vector<std::vector<ctext_typ>>;

    ctext_matrix  matrix_add(const PALISADEContainer& pc, const ctext_matrix& matA, const ctext_matrix& matB){
        //returns a new matrix that is the sum of the two matricies matA and matB
        ctext_matrix sum(matA.size(), std::vector<ctext_typ>(matA[0].size()));
        for(size_t i=0; i<matA.size(); i++){
            for(size_t j=0; j<matA.at(i).size(); j++){
                sum[i][j]= pc.context->EvalAdd(matA[i][j], matB[i][j]);
            }
        }

        return sum;
        
    };


    ctext_matrix matrix_sub(const PALISADEContainer& pc, const ctext_matrix& matA, const ctext_matrix& matB){
        //returns a new matrix that is the difference between matA and matB (matA-matB)
        ctext_matrix diff(matA.size(), std::vector<ctext_typ>(matA[0].size()));
        for(size_t i=0; i<matA.size(); i++){
            for(size_t j=0; j<matA[i].size(); j++){
                diff[i][j] = pc.context->EvalSub(matA[i][j], matB[i][j]);
            }
        }
        return diff;
    };

    ctext_matrix transpose(const PALISADEContainer& pc, const ctext_matrix& mat){
        //returns the transpose of the matrix
        ctext_matrix transpose(mat.size(), std::vector<ctext_typ>(mat[0].size()));

        for(size_t i=0; i<mat.size(); i++){
            for(size_t j=0; j<mat[i].size(); j++){
                transpose[j][i] = mat[i][j];
            }
        }
        return transpose;
    } 

    ctext_matrix matrix_mult(const PALISADEContainer&  pc,const ctext_matrix& matA, const ctext_matrix& matB){
        //returns the quotient of the matricies matA and matB
        ctext_matrix quotient(matA.size(), std::vector<ctext_typ>(matB[0].size()));

            for(size_t i=0; i< matA.size();i++){
                for(size_t j=0; j < matB[i].size(); j++){
                    ctext_typ sum = pc.context->EvalMult(matA[i][0], matB[0][j]);
                    for(size_t k=1; k<matA[0].size();k++){
                        pc.context->EvalAddInPlace(sum,pc.context->EvalMult(matA[i][k],matB[k][j]));
                    }
                    quotient[i][j] = sum;
                }
            }

        return quotient;

    };

    ctext_matrix packed_mult(const PALISADEContainer& pc, const ctext_matrix& matA, const ctext_matrix&matB){
        ctext_matrix quotient(matA.size(), std::vector<ctext_typ>(matB[0].size()));

        for(size_t i=0; i<matA.size();i++){
            for(size_t j=0;j<matB[0].size();j++){
                quotient[i][j] = pc.context->EvalMult(matA[i][0],matB[0][j]);
            }
        }

        return quotient;
    }

    ctext_matrix cofactor(const ctext_matrix& mat, size_t p, size_t q){
        //returns the matrix of cofactors as a new matrix
        ctext_matrix cofactor(mat.size(), std::vector<ctext_typ>(mat[0].size()));
        size_t i=0, j=0;

        for(size_t row=0; row<mat.size(); row++){
            for(size_t col=0; col<mat[0].size(); col++){
                if(row!=p && col!=q){
                    cofactor[i][j++] = mat[row][col];

                    if(j==mat.size()-1){
                        j=0;
                        ++i;
                    }
                }
            }
        }

        return cofactor;
        
    };

    ctext_typ determinant(const PALISADEContainer& pc, const ctext_matrix& mat, int n){
        //returns the determinant of a matrix as a ciphertext
        std::vector<double> temp = {0};
        Plaintext pdet = pc.context->MakeCKKSPackedPlaintext(temp);
        ctext_typ det = pc.context->Encrypt(pc.pk,pdet);
        if(n<=1){
            return mat[0][0];
        }

        double sign = 1;

        for(int i=0; i<n;i++){
            ctext_matrix cfact = cofactor(mat,0,i);

            temp = {sign};
            Plaintext psign = pc.context->MakeCKKSPackedPlaintext(temp);
            ctext_typ csign = pc.context->Encrypt(pc.pk,psign);
            csign->SetDepth(mat[0][i]->GetDepth());

            ctext_typ tmp = pc.context->EvalMult(csign,mat[0][i]);
            ctext_typ tdet = determinant(pc,cfact, n-1);
            tdet->SetDepth(tmp->GetDepth());

            ctext_typ mult = pc.context->EvalMult(tmp,tdet);
            det->SetDepth(mult->GetDepth());
            pc.context->EvalAddInPlace(det,mult);

            sign = -sign;
        }

        return det;
    };

    ctext_matrix adjoint(const PALISADEContainer& pc, const ctext_matrix& mat){
        //returns the adjoint matrix of a given matrix mat
        double sign = 1;
        ctext_matrix tmp(mat.size(), std::vector<ctext_typ>(mat[0].size()));
        ctext_matrix adjoint(mat.size(), std::vector<ctext_typ>(mat[0].size()));

        for(size_t i=0; i<mat.size(); i++){
            for(size_t j=0; j<mat[0].size(); j++){
                ctext_matrix tmp = cofactor(mat,i,j);

                sign = ((i+j)%2==0) ? 1:-1;
                std::vector<double> temp = {sign};
                Plaintext psign = pc.context->MakeCKKSPackedPlaintext(temp);
                ctext_typ csign = pc.context->Encrypt(pc.pk,psign);

                ctext_typ det = determinant(pc, tmp, mat.size()-1);
                csign->SetDepth(det->GetDepth());

                adjoint[i][j] = pc.context->EvalMult(csign,det);
            }
        }
        return adjoint;

    };


#endif