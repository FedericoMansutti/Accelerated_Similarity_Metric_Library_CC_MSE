/*
MIT License

Copyright (c) 2023 Paolo Salvatore Galfano, Giuseppe Sorrentino

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include <ap_int.h>
#include <hls_stream.h>
#include <hls_math.h>
#include <ap_axi_sdata.h>
#include "../common/common.h"

#define VEC_SIZE 16
#define BITWIDTH sizeof(uint8_t) * 8 * VEC_SIZE
extern "C" {

void setup_aie(int32_t size, uint8_t* input1,  uint8_t* input2, hls::stream<ap_uint<BITWIDTH>>& s_1, hls::stream<ap_uint<BITWIDTH>>& s_2) {

	#pragma HLS interface m_axi port=input1 depth=100 offset=slave bundle=gmem0
	#pragma HLS interface m_axi port=input2 depth=100 offset=slave bundle=gmem0
	#pragma HLS interface axis port=s_1
	#pragma HLS interface axis port=s_2
	#pragma HLS interface s_axilite port=input1 bundle=control
	#pragma HLS interface s_axilite port=input2 bundle=control
	#pragma HLS interface s_axilite port=size bundle=control
	#pragma HLS interface s_axilite port=return bundle=control

	// size represents the number of elements. But the AI Engine uses the number of loops, and each
	// loop uses 4 elements. So we need to convert the number of elements to the number of loops.
	int32_t size_loop = size/VEC_SIZE;

	ap_uint<BITWIDTH> tmp1;
	ap_uint<BITWIDTH> tmp2;
	tmp1.range(31,0) = size_loop;
	s_1.write(tmp1);
	
	for (int j = 0; j < size_loop; j++) {
		tmp1.range(7,0) = input1[16*j];
		tmp1.range(15,8) = input1[16*j+1];
		tmp1.range(23,16) = input1[16*j+2];
		tmp1.range(31,24) = input1[16*j+3];
		tmp1.range(39,32) = input1[16*j+4];
		tmp1.range(47,40) = input1[16*j+5];
		tmp1.range(55,48) = input1[16*j+6];
		tmp1.range(63,56) = input1[16*j+7];
		tmp1.range(71,64) = input1[16*j+8];
		tmp1.range(79,72) = input1[16*j+9];
		tmp1.range(87,80) = input1[16*j+10];
		tmp1.range(95,88) = input1[16*j+11];
		tmp1.range(103,96) = input1[16*j+12];
		tmp1.range(111,104) = input1[16*j+13];
		tmp1.range(119,112) = input1[16*j+14];
		tmp1.range(127,120) = input1[16*j+15];
		s_1.write(tmp1);

		tmp2.range(7,0) = input2[16*j];
		tmp2.range(15,8) = input2[16*j+1];
		tmp2.range(23,16) = input2[16*j+2];
		tmp2.range(31,24) = input2[16*j+3];
		tmp2.range(39,32) = input2[16*j+4];
		tmp2.range(47,40) = input2[16*j+5];
		tmp2.range(55,48) = input2[16*j+6];
		tmp2.range(63,56) = input2[16*j+7];
		tmp2.range(71,64) = input2[16*j+8];
		tmp2.range(79,72) = input2[16*j+9];
		tmp2.range(87,80) = input2[16*j+10];
		tmp2.range(95,88) = input2[16*j+11];
		tmp2.range(103,96) = input2[16*j+12];
		tmp2.range(111,104) = input2[16*j+13];
		tmp2.range(119,112) = input2[16*j+14];
		tmp2.range(127,120) = input2[16*j+15];
		s_2.write(tmp2);
	}
}
}
