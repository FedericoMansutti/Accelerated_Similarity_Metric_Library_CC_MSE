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
#include <hls_math.h>
#include "../common/common.h"

#define vector_type int
#define pixel_type uint8_t
#define stream_type ap_uint<128>
#define read_size 16

extern "C" {

void setup_aie(int size, vector_type* input_1, vector_type* input_2,  hls::stream<stream_type>& s_1, hls::stream<stream_type>& s_2) {

	#pragma HLS interface m_axi port=input_1 depth=100 offset=slave bundle=gmem0
	#pragma HLS interface axis port=s_1
	#pragma HLS interface s_axilite port=input_1 bundle=control

	#pragma HLS interface m_axi port=input_2 depth=100 offset=slave bundle=gmem0
	#pragma HLS interface axis port=s_2
	#pragma HLS interface s_axilite port=input_2 bundle=control

	#pragma HLS interface s_axilite port=size bundle=control
	#pragma HLS interface s_axilite port=return bundle=control

	// size1 represents the pixels in image 1 while size2 represents the pixels in image2

	stream_type ap_1;
	stream_type ap_2;

	int32_t size_loop = size/read_size;

	// Array to store the four 8-bit integers
    unsigned char bytes[4];


	ap_1.range(31,0) = size_loop;
	
	s_1.write(ap_1);
	s_2.write(ap_1);
	
	for (int i = 0; i < size_loop; i++) {
		ap_1.range(7, 0) = (pixel_type) input_1[i * read_size + 0];
		ap_1.range(15, 8) = (pixel_type) input_1[i * read_size + 1];
		ap_1.range(23, 16) = (pixel_type) input_1[i * read_size + 2];
		ap_1.range(31, 24) = (pixel_type) input_1[i * read_size + 3];

		ap_1.range(39, 32) = (pixel_type) input_1[i * read_size + 4];
		ap_1.range(47, 40) = (pixel_type) input_1[i * read_size + 5];
		ap_1.range(55, 48) = (pixel_type) input_1[i * read_size + 6];
		ap_1.range(63, 56) = (pixel_type) input_1[i * read_size + 7];

		ap_1.range(71, 64) = (pixel_type) input_1[i * read_size + 8];
		ap_1.range(79, 72) = (pixel_type) input_1[i * read_size + 9];
		ap_1.range(87, 80) = (pixel_type) input_1[i * read_size + 10];
		ap_1.range(95, 88) = (pixel_type) input_1[i * read_size + 11];

		ap_1.range(103, 96) = (pixel_type) input_1[i * read_size + 12];
		ap_1.range(111, 104) = (pixel_type) input_1[i * read_size + 13];
		ap_1.range(119, 112) = (pixel_type) input_1[i * read_size + 14];
		ap_1.range(127, 120) = (pixel_type) input_1[i * read_size + 15];

		///

		ap_2.range(7, 0) = (pixel_type) input_2[i * read_size + 0];
		ap_2.range(15, 8) = (pixel_type) input_2[i * read_size + 1];
		ap_2.range(23, 16) = (pixel_type) input_2[i * read_size + 2];
		ap_2.range(31, 24) = (pixel_type) input_2[i * read_size + 3];

		ap_2.range(39, 32) = (pixel_type) input_2[i * read_size + 4];
		ap_2.range(47, 40) = (pixel_type) input_2[i * read_size + 5];
		ap_2.range(55, 48) = (pixel_type) input_2[i * read_size + 6];
		ap_2.range(63, 56) = (pixel_type) input_2[i * read_size + 7];

		ap_2.range(71, 64) = (pixel_type) input_2[i * read_size + 8];
		ap_2.range(79, 72) = (pixel_type) input_2[i * read_size + 9];
		ap_2.range(87, 80) = (pixel_type) input_2[i * read_size + 10];
		ap_2.range(95, 88) = (pixel_type) input_2[i * read_size + 11];

		ap_2.range(103, 96) = (pixel_type) input_2[i * read_size + 12];
		ap_2.range(111, 104) = (pixel_type) input_2[i * read_size + 13];
		ap_2.range(119, 112) = (pixel_type) input_2[i * read_size + 14];
		ap_2.range(127, 120) = (pixel_type) input_2[i * read_size + 15];

		s_1.write((stream_type) ap_1);
		s_2.write((stream_type) ap_2);
	}
}
}