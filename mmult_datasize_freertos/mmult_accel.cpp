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

/**
 *
 * Design principles to achieve II = 1
 * 1. Stream data into local RAM for inputs (if multiple access required)
 * 2. Partition local RAMs for fully parallel access
 * 3. Pipeline the dot-product loop, to fully unroll it
 *
 */
void mmult_accel(float A[N*N], float B[N*N], float C[N*N], int M) 
{
  float A_tmp[N][N], B_tmp[N][N];
#pragma HLS array_partition variable=A_tmp complete dim=2
#pragma HLS array_partition variable=B_tmp complete dim=1
     
     for(int i=0; i<M; i++) {
          for(int j=0; j<M; j++) {
#pragma HLS PIPELINE
               A_tmp[i][j] = A[i*M+j];
               B_tmp[i][j] = B[i*M+j];
          }
     }
     
     for (int i = 0; i < M; i++) {
          for (int j = 0; j < M; j++) {
#pragma HLS PIPELINE
               float result = 0;
               for (int k = 0; k < M; k++) {
                    result += A_tmp[i][k] * B_tmp[k][j];
               }
               C[i*M+j] = result;
          }
     }
}
