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

#include <stdio.h>
#include <stdlib.h>

#include "mmult_accel.h"

#define NUM_TESTS 1024

#include "sds_lib.h"
#define TIME_STAMP_INIT  unsigned long long clock_start, clock_end;  clock_start = sds_clock_counter();  
#define TIME_STAMP_SW  { clock_end = sds_clock_counter(); printf("Average number of processor cycles for golden version: %llu \n", (clock_end-clock_start)/NUM_TESTS); clock_start = sds_clock_counter();  }
#define TIME_STAMP_ACCEL  { clock_end = sds_clock_counter(); printf("Average number of processor cycles for hardware version: %llu \n", (clock_end-clock_start)/NUM_TESTS); }

static void mmult_init(float *tin1Buf,  float *tin2Buf, float *toutBufSw, float *toutBufHw)
{
  int i, j;
  
  for (i = 0; i < A_NROWS; i++) {
    for (j = 0; j < A_NCOLS; j++) {
      tin1Buf[i * A_NCOLS + j] = 1+i*A_NCOLS+j;
    }
  }
  for (i = 0; i < B_NROWS; i++) {  
    for (j = 0; j < B_NCOLS; j++) {
      tin2Buf[i * B_NCOLS + j] = rand() % (A_NCOLS * B_NCOLS);
    }
  }
  for (i = 0; i < A_NROWS; i++) {
    for (j = 0; j < B_NCOLS; j++) {
      toutBufSw[i * B_NCOLS + j] = 0.0;
      toutBufHw[i * B_NCOLS + j] = 0.0;
    }
  }
}

void mmult_golden(float *in_arr,  float *in_arr2, float *out_arr)
{
  for (int row = 0; row < A_NROWS; row++) {
    for (int col = 0; col < B_NCOLS; col++) {
      float result = 0.0;
      for (int k = 0; k < A_NCOLS; k++) {
        result += in_arr[row*A_NCOLS+k] * in_arr2[k*B_NCOLS+col];
      }
      out_arr[row*A_NCOLS+col] = result;
    }
  }
}

static int mmult_result_check(float *toutBufSw, float *toutBufHw)
{
  int i;
  
  for (i = 0; i < A_NROWS * B_NCOLS; i++) {
    if (toutBufSw[i] != toutBufHw[i]) {
      printf("Mismatch: data index=%d d=%f, dout=%f\n", i, toutBufSw[i], toutBufHw[i]);
      return 0;
    }
  }
  return 1;
}

int mmult_test(float *tin1Buf,  float *tin2Buf, float *toutBufSw, float *toutBufHw)
{
  int i;
  
  printf("Testing mmult ...\n");
  
  mmult_init(tin1Buf, tin2Buf, toutBufSw, toutBufHw);

  TIME_STAMP_INIT

  for (i = 0; i < NUM_TESTS; i++) {
    mmult_golden(tin1Buf, tin2Buf, toutBufSw);
  }

  TIME_STAMP_SW

  for (i = 0; i < NUM_TESTS; i++)
    mmult_accel(tin1Buf, tin2Buf, toutBufHw);

  TIME_STAMP_ACCEL

  return mmult_result_check(toutBufSw, toutBufHw);
}


extern "C" int mmult_main(){
  int test_passed = 0;
  float *tin1Buf, *tin2Buf, *toutBufSw, *toutBufHw;
  
  printf("Testing with A_NROWS = A_NCOLS = B_NCOLS = B_NROWS = %d\n", A_NROWS);

  tin1Buf = (float *)sds_alloc(A_NROWS * A_NCOLS * sizeof(float));
  tin2Buf = (float *)sds_alloc(A_NCOLS * B_NCOLS * sizeof(float));
  toutBufHw = (float *)sds_alloc(A_NROWS * B_NCOLS * sizeof(float));
  toutBufSw = (float *)sds_alloc(A_NROWS * B_NCOLS * sizeof(float));

  if (!tin1Buf || !tin2Buf || !toutBufHw || !toutBufSw) {
    if (tin1Buf) sds_free(tin1Buf);
    if (tin2Buf) sds_free(tin2Buf);
    if (toutBufHw) sds_free(toutBufHw);
    if (toutBufSw) sds_free(toutBufSw);
    return 2;
  }

  test_passed = mmult_test(tin1Buf, tin2Buf, toutBufSw, toutBufHw);

  sds_free(tin1Buf);
  sds_free(tin2Buf);
  sds_free(toutBufHw);
  sds_free(toutBufSw);
  
  printf("TEST %s\n", test_passed ? "PASSED" : "FAILED");
  
  return (test_passed ? 0 : -1);
}

