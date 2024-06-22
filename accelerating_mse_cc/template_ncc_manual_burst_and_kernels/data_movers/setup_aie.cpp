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
#include <ap_axi_sdata.h>
#include "hls_burst_maxi.h"

#define vector_type int
#define pixel_type unsigned char
#define stream_type ap_uint<128>
#define read_size 16
#define burst_length 64
#define kernel_count 2

extern "C" {

void setup_aie(int size1, int size2, hls::burst_maxi<ap_uint<128>> input_1, hls::burst_maxi<ap_uint<128>> input_2,  hls::stream<stream_type>& s_1, hls::stream<stream_type>& s_2, 
hls::stream<stream_type>& s_3, hls::stream<stream_type>& s_4) {

	#pragma HLS interface m_axi port=input_1 offset=slave bundle=gmem0 max_read_burst_length=256 //256 int = 64 ap uint<128>
	#pragma HLS interface axis port=s_1
	#pragma HLS interface s_axilite port=input_1 bundle=control

	#pragma HLS interface m_axi port=input_2 offset=slave bundle=gmem0 max_read_burst_length=256
	#pragma HLS interface axis port=s_2
	#pragma HLS interface s_axilite port=input_2 bundle=control

	#pragma HLS interface s_axilite port=size1 bundle=control
	#pragma HLS interface s_axilite port=size2 bundle=control
	#pragma HLS interface s_axilite port=return bundle=control

	// size1 represents the pixels in image 1 while size2 represents the pixels in image2
	int size1_copy = size1;
	int size2_copy = size2;

	stream_type ap_1;

	int size_1_vec[read_size] = {0};

	//repeat until size 1 has finished writing
	int index = 0;
	while (size1_copy != 0){
		size_1_vec[index++] = size1_copy % 10;
		size1_copy /= 10;
	}

	ap_1.range(7, 0) = (pixel_type) size_1_vec[0];
	ap_1.range(15, 8) = (pixel_type) size_1_vec[1];
	ap_1.range(23, 16) = (pixel_type) size_1_vec[2];
	ap_1.range(31, 24) = (pixel_type) size_1_vec[3];

	ap_1.range(39, 32) = (pixel_type) size_1_vec[4];
	ap_1.range(47, 40) = (pixel_type) size_1_vec[5];
	ap_1.range(55, 48) = (pixel_type) size_1_vec[6];
	ap_1.range(63, 56) = (pixel_type) size_1_vec[7];

	ap_1.range(71, 64) = (pixel_type) size_1_vec[8];
	ap_1.range(79, 72) = (pixel_type) size_1_vec[9];
	ap_1.range(87, 80) = (pixel_type) size_1_vec[10];
	ap_1.range(95, 88) = (pixel_type) size_1_vec[11];

	ap_1.range(103, 96) = (pixel_type) size_1_vec[12];
	ap_1.range(111, 104) = (pixel_type) size_1_vec[13];
	ap_1.range(119, 112) = (pixel_type) size_1_vec[14];
	ap_1.range(127, 120) = (pixel_type) size_1_vec[15];

	///

	s_1.write((stream_type) ap_1);
	s_2.write((stream_type) ap_1);
	s_3.write((stream_type) ap_1);
	s_4.write((stream_type) ap_1);


	stream_type buf_1[burst_length/kernel_count];
	stream_type buf_2[burst_length/kernel_count];
	stream_type buf_3[burst_length/kernel_count];
	stream_type buf_4[burst_length/kernel_count];

	int ap_count = size1 / read_size;
	int burst_count = ap_count / (burst_length);
	int remainder = ap_count % burst_length;
	int start = 0;

	for (int i = 0; i < burst_count; i++){
		#pragma HLS PIPELINE II=1

		start = i * burst_length;
		//read burst!
		input_1.read_request(start, burst_length);
		input_2.read_request(start, burst_length);
		for (int j = 0; j < burst_length/kernel_count; j++){
			#pragma HLS PIPELINE UNROLL
			buf_1[j] = input_1.read();
			buf_2[j] = input_2.read();
			buf_3[j] = input_1.read();
			buf_4[j] = input_2.read();
		}

		for (int j = 0; j < burst_length/kernel_count; j++){
			s_1.write((stream_type) buf_1[j]);
			s_2.write((stream_type) buf_2[j]);
			s_3.write((stream_type) buf_3[j]);
			s_4.write((stream_type) buf_4[j]);
			

		}

	}
	start = burst_length * burst_count;
	input_1.read_request(start, remainder);
	input_2.read_request(start, remainder);
	for (int j = 0; j < remainder/kernel_count; j++){
		#pragma HLS PIPELINE UNROLL
		buf_1[j] = input_1.read();
		buf_2[j] = input_2.read();
		buf_3[j] = input_1.read();
		buf_4[j] = input_2.read();
	}

	for (int j = 0; j < remainder/kernel_count; j++){
		s_1.write((stream_type) buf_1[j]);
		s_2.write((stream_type) buf_2[j]);
		s_3.write((stream_type) buf_3[j]);
		s_4.write((stream_type) buf_4[j]);

	}
}
}