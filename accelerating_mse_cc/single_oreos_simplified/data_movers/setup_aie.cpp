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

void setup_aie(int32_t size, uint8_t* input, hls::stream<ap_uint<BITWIDTH>>& s) {

	#pragma HLS interface m_axi port=input depth=100 offset=slave bundle=gmem0
	#pragma HLS interface axis port=s
	#pragma HLS interface s_axilite port=input bundle=control
	#pragma HLS interface s_axilite port=size bundle=control
	#pragma HLS interface s_axilite port=return bundle=control

	// size represents the number of elements. But the AI Engine uses the number of loops, and each
	// loop uses 4 elements. So we need to convert the number of elements to the number of loops.
	int32_t size_loop = size/VEC_SIZE;

	ap_uint<BITWIDTH> tmp;
	tmp.range(31,0) = size_loop;
	s.write(tmp);
	
	for (int j = 0; j < size_loop; j++) {
		tmp.range(7,0) = input[16*j];
		tmp.range(15,8) = input[16*j+1];
		tmp.range(23,16) = input[16*j+2];
		tmp.range(31,24) = input[16*j+3];
		tmp.range(39,32) = input[16*j+4];
		tmp.range(47,40) = input[16*j+5];
		tmp.range(55,48) = input[16*j+6];
		tmp.range(63,56) = input[16*j+7];
		tmp.range(71,64) = input[16*j+8];
		tmp.range(79,72) = input[16*j+9];
		tmp.range(87,80) = input[16*j+10];
		tmp.range(95,88) = input[16*j+11];
		tmp.range(103,96) = input[16*j+12];
		tmp.range(111,104) = input[16*j+13];
		tmp.range(119,112) = input[16*j+14];
		tmp.range(127,120) = input[16*j+15];
		s.write(tmp);
	}
}
}
