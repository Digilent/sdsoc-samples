/*
(c) Copyright 2013 - 2016 Xilinx, Inc. All rights reserved. 

This file contains confidential and proprietary information of Xilinx, Inc. and
is protected under U.S. and international copyright and other intellectual
property laws.

DISCLAIMER 
This disclaimer is not a license and does not grant any rights to the materials
distributed herewith. Except as otherwise provided in a valid license issued to
you by Xilinx, and to the maximum extent permitted by applicable law: (1) THESE
MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS, AND XILINX HEREBY
DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY,
INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR
FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether
in contract or tort, including negligence, or under any other theory of
liability) for any loss or damage of any kind or nature related to, arising
under or in connection with these materials, including for any direct, or any
indirect, special, incidental, or consequential loss or damage (including loss
of data, profits, goodwill, or any type of loss or damage suffered as a result
of any action brought by a third party) even if such damage or loss was
reasonably foreseeable or Xilinx had been advised of the possibility of the
same.

CRITICAL APPLICATIONS
Xilinx products are not designed or intended to be fail-safe, or for use in any
application requiring fail-safe performance, such as life-support or safety
devices or systems, Class III medical devices, nuclear facilities, applications
related to the deployment of airbags, or any other applications that could lead
to death, personal injury, or severe property or environmental damage
(individually and collectively, "Critical Applications"). Customer assumes the
sole risk and liability of any use of Xilinx products in Critical Applications,
subject only to applicable laws and regulations governing limitations on product
liability.

THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT
ALL TIMES. 
*/

#include <iostream>
#include <sstream>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "mmult_accel.h"
#include "sds_lib.h"

#ifndef NUM_TESTS
#define NUM_TESTS 256
#endif

static void init_arrays(float *A,  float *B, float *C_sw, float *C, int M)
{
     for (int i = 0; i < M*M; i++) {
          A[i] = 1+i;
          B[i] = rand() % (M*M);
          C_sw[i] = 0.0;
          C[i] = 0.0;
     }
}

void mmult_golden(float *A,  float *B, float *C, int M)
{
     for (int row = 0; row < M; row++) {
          for (int col = 0; col < M; col++) {
               float result = 0.0;
               for (int k = 0; k < M; k++) {
                    result += A[row*M+k] * B[k*M+col];
               }
               C[row*M+col] = result;
          }
     }
}

static int result_check(float *C_sw, float *C, int M, int iteration)
{
  int res = 0;
  std::stringstream ss;
  ss << "====Golden/Expected/SW====" << std::endl;
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < M; j++ ){
      ss << C_sw[i*M+j] << ", ";
    }
    ss << std::endl;
  }
  ss << "====HW====" << std::endl;
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < M; j++ ){
      if ( C[i*M+j] != C_sw[i*M+j] ){
        ss << "*" << C[i*M+j] << ", ";
        res = 1;
      }
      else
        ss << C[i*M+j] << ", ";
    }
    ss << std::endl;
  }
  if ( res )
    std::cout << ss.str();

  return res;
}

int mmult_test(int M, float *A,  float *B, float *C_sw, float *C)
{
     int i;
     
     std::cout << "Testing " << NUM_TESTS << " iterations of " << M 
               << "x" << M << " floating point mmult..." << std::endl;
     
     init_arrays(A, B, C_sw, C, M);
     int result = 0;
     for (i = 0; !result && i < NUM_TESTS; i++) {
          mmult_golden(A, B, C_sw, M);
          mmult_accel(A, B, C, M);
          result = result_check(C_sw, C, M, i);
     }
     return result;
}

int main(int argc, char* argv[]){
     float *A, *B, *C_sw, *C;

     A = (float *)sds_alloc(N * N * sizeof(float));
     B = (float *)sds_alloc(N * N * sizeof(float));
     C = (float *)sds_alloc(N * N * sizeof(float));
     C_sw = (float *)sds_alloc(N * N * sizeof(float));
     
     if (!A || !B || !C || !C_sw) {
          if (A) sds_free(A);
          if (B) sds_free(B);
          if (C) sds_free(C);
          if (C_sw) sds_free(C_sw);
          return 2;
     }
     
     int test_passed = mmult_test(32, A, B, C_sw, C);
     std::cout << "TEST " << (test_passed ? "FAILED" : "PASSED") << std::endl;
     
     test_passed = mmult_test(16, A, B, C_sw, C);
     std::cout << "TEST " << (test_passed ? "FAILED" : "PASSED") << std::endl;
     
     sds_free(A);
     sds_free(B);
     sds_free(C);
     sds_free(C_sw);
     
     return (test_passed ? -1 : 0);
}

