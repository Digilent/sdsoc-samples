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

void mmult_kernel(float in_A[A_NROWS][A_NCOLS],
                  float in_B[A_NCOLS][B_NCOLS],
                  float out_C[A_NROWS*B_NCOLS])
{
#pragma HLS INLINE self
#pragma HLS array_partition variable=in_A block factor=16 dim=2
#pragma HLS array_partition variable=in_B block factor=16 dim=1

  int index_a, index_b, index_d;

  for (index_a = 0; index_a < A_NROWS; index_a++) {
    for (index_b = 0; index_b < B_NCOLS; index_b++) {
#pragma HLS PIPELINE II=1
      float result = 0;
      for (index_d = 0; index_d < A_NCOLS; index_d++) {
        // multiply accumulate broken into individual operators
        // so that AutoESL can infer two FP operators
        float product_term = in_A[index_a][index_d] * in_B[index_d][index_b];
        result += product_term;
      }
      out_C[index_a * B_NCOLS + index_b] = result;
    }
  }
}

void mmult_accel (float in_A[A_NROWS*A_NCOLS],
                  float in_B[A_NCOLS*B_NCOLS],
                  float out_C[A_NROWS*B_NCOLS]) 
{
  int i, j;
  float a_buf[A_NROWS][A_NCOLS];
  float b_buf[A_NCOLS][B_NCOLS];

  // Transfer matrix A from multi-buffer into local RAM
  for(i=0; i<A_NROWS; i++) {
    for(j=0; j<A_NCOLS; j++) {
#pragma HLS PIPELINE II=1
      a_buf[i][j] = in_A[i * A_NCOLS + j];
    }
  }

  // Transfer matrix B from multi-buffer into local RAM
  for(i=0; i<A_NCOLS; i++) {
    for(j=0; j<B_NCOLS; j++) {
#pragma HLS PIPELINE II=1
      b_buf[i][j] = in_B[i * B_NCOLS + j];
    }
  }

  // Matrix multiply call
  mmult_kernel(a_buf, b_buf, out_C);
}

