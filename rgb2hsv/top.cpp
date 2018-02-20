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
#include <algorithm>
#include <stdlib.h>

#include "top.h"

int size = 19;
RGBcolor rgbColors[] = {{255,255,255},
			{127,127,127},
			{0,0,0},
			{255,0,0},
			{191,191,0},
			{0,127,0},
			{127,255,255},
			{127,127,255},
			{191,63,191},
			{160,163,36},
			{65,26,234},
			{29,172,65},
			{239,200,13},
			{179,47,228},
			{237,118,80},
			{254,248,135},
			{25,202,150},
			{53,37,152},
			{126,125,183}};

int main() {
  HSVcolor hsvColors[19];
  HSVcolor hsvColors_sw[19];
  RGBcolor outColors[19];
  RGBcolor outColors_sw[19];

  //print out initial RGB pixel values
  for(int i=0; i<size; i++) {
    printRGB(rgbColors[i]);
  }

  //convert pixels from RGB to HSV
  RgbToHsv(rgbColors,hsvColors);
  RgbToHsv_sw(rgbColors,hsvColors_sw);
    
  //print out converted HSV pixel values
  for(int i=0; i<size; i++) {
    printHSV(hsvColors[i]);
  }

  //convert pixels from HSV to RGB
  HsvToRgb(hsvColors,outColors);
  HsvToRgb_sw(hsvColors_sw,outColors_sw);

  //print out final RGB pixel values
  for(int i=0; i<size; i++) {
    printRGB(outColors[i]);
  }

  int error = 0;
  for(int i=0; i<size; i++) {
    printf("(orig/new) R:%d/%d, G:%d/%d, B:%d/%d\n",rgbColors[i].r,outColors[i].r,rgbColors[i].g,outColors[i].g,rgbColors[i].b,outColors[i].b);
    if ( !(outColors[i] == outColors_sw[i]) ){
      error = 1;
      printf("HW SW results mismatch!\n");
      break;
    }
  }

  return error;
}

void printRGB(RGBcolor c) {
  printf("RGB value is R:%d, G:%d, B:%d\n",c.r,c.g,c.b);
}

void printHSV(HSVcolor c) {
  printf("HSV value is H:%d, S:%d, V:%d\n",c.h,c.s,c.v);
}

void HsvToRgb_kernel(HSVcolor in[19], RGBcolor out[19])
{
#pragma HLS inline
  for(int i=0; i<19; i++) {
    unsigned char region, m, X0m, X1m;

    if (in[i].s == 0) {
      out[i].r = in[i].v;
      out[i].g = in[i].v;
      out[i].b = in[i].v;
    }
    else {
      unsigned degrees, uh = in[i].h, us = in[i].s, uv = in[i].v;
      region = uh / 43;
      degrees = (uh % 43) * 6; 
      
      m = (uv * (255 - us)) >> 8;
      X1m = (uv * (255 - ((us * degrees) >> 8))) >> 8;
      X0m = (uv * (255 - ((us * (255 - degrees)) >> 8))) >> 8;
      printf("region: %d  remainder: %d  p: %d  q: %d  t: %d\n",region, degrees, m,X1m,X0m);
      switch (region) {
      case 0:
	out[i].r = in[i].v; 
	out[i].g = X0m; 
	out[i].b = m;
	break;
      case 1:
	out[i].r = X1m; 
	out[i].g = in[i].v; 
	out[i].b = m;
	break;
      case 2:
	out[i].r = m; 
	out[i].g = in[i].v; 
	out[i].b = X0m;
	break;
      case 3:
	out[i].r = m; 
	out[i].g = X1m; 
	out[i].b = in[i].v;
	break;
      case 4:
	out[i].r = X0m; 
	out[i].g = m; 
	out[i].b = in[i].v;
	break;
      default:
	out[i].r = in[i].v; 
	out[i].g = m; 
	out[i].b = X1m;
	break;
      }
    }
  }
}

void HsvToRgb(HSVcolor in[19], RGBcolor out[19])
{
  HsvToRgb_kernel(in, out);
}

void HsvToRgb_sw(HSVcolor in[19], RGBcolor out[19])
{
  HsvToRgb_kernel(in, out);
}

void RgbToHsv_kernel(RGBcolor in[19], HSVcolor out[19])
{
#pragma HLS inline
  for(int i=0; i<19; i++) {
    unsigned char global_min, global_max;
    unsigned chroma;

    global_min = std::min(in[i].r, std::min(in[i].g, in[i].b));
    global_max = std::max(in[i].r, std::max(in[i].g, in[i].b));

    out[i].v = global_max;

    if (global_max == 0) {
      out[i].h = 0;
      out[i].s = 0;
    }
    else {
      chroma = global_max - global_min;
      out[i].s = (255 * chroma) / ((unsigned)out[i].v);
 
      if (out[i].s == 0) {
        out[i].h = 0;
      }
      else {
	if (global_max == in[i].r) {
	  unsigned diff = in[i].g - in[i].b;
	  out[i].h = (43 * diff) / chroma;
	}
	else if (global_max == in[i].g) {
	  unsigned diff = in[i].b - in[i].r;
	  out[i].h = 85 + (43 * diff) / chroma;
	}
	else {
	  unsigned diff = in[i].r - in[i].g;
	  out[i].h = 171 + (43 * diff) / chroma;
	}
      }
    }
  }
}

void RgbToHsv(RGBcolor in[19], HSVcolor out[19])
{
  RgbToHsv_kernel(in, out);
}

void RgbToHsv_sw(RGBcolor in[19], HSVcolor out[19])
{
  RgbToHsv_kernel(in, out);
}
