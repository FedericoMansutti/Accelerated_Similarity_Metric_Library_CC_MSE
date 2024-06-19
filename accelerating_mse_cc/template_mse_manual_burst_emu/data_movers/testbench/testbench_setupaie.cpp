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

#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ap_axi_sdata.h>
#include <cmath>
#include "../setup_aie.cpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "../common/common.h"

#define max_pixel_value 256
#define write_type uint8_t

void read_from_stream(int *buffer, hls::stream<int> &stream, size_t size) {
    for (unsigned int i = 0; i < size; i++) {
        buffer[i] = stream.read();
    }
}

int main(int argc, char* argv[]) {
    srand(time(0));

    hls::stream<stream_type> s_1;
    hls::stream<stream_type> s_2;
    int size1 = 512;
    int size2 = 512;
    int depth = 1;

    size1 *= depth;
    size2 *= depth;
    int loop_size = size1/read_size;

    stream_type input_1[loop_size];
    stream_type input_2[loop_size];

    stream_type ap_1;
	stream_type ap_2;

    for (int i = 0; i < loop_size; i++) {
            input_1[i].range(7, 0) = (write_type) rand() % max_pixel_value;
            input_1[i].range(15, 8) = (write_type) rand() % max_pixel_value;
            input_1[i].range(23, 16) = (write_type) rand() % max_pixel_value;
            input_1[i].range(31, 24) = (write_type) rand() % max_pixel_value;

            input_1[i].range(39, 32) = (write_type) rand() % max_pixel_value;
            input_1[i].range(47, 40) = (write_type) rand() % max_pixel_value;
            input_1[i].range(55, 48) = (write_type) rand() % max_pixel_value;
            input_1[i].range(63, 56) = (write_type) rand() % max_pixel_value;

            input_1[i].range(71, 64) = (write_type) rand() % max_pixel_value;
            input_1[i].range(79, 72) = (write_type) rand() % max_pixel_value;
            input_1[i].range(87, 80) = (write_type) rand() % max_pixel_value;
            input_1[i].range(95, 88) = (write_type) rand() % max_pixel_value;

            input_1[i].range(103, 96) = (write_type) rand() % max_pixel_value;
            input_1[i].range(111, 104) = (write_type) rand() % max_pixel_value;
            input_1[i].range(119, 112) = (write_type) rand() % max_pixel_value;
            input_1[i].range(127, 120) = (write_type) rand() % max_pixel_value;
    }
    for (int i = 0; i < loop_size; i++) {
            input_2[i].range(7, 0) = (write_type) rand() % max_pixel_value;
            input_2[i].range(15, 8) = (write_type) rand() % max_pixel_value;
            input_2[i].range(23, 16) = (write_type) rand() % max_pixel_value;
            input_2[i].range(31, 24) = (write_type) rand() % max_pixel_value;

            input_2[i].range(39, 32) = (write_type) rand() % max_pixel_value;
            input_2[i].range(47, 40) = (write_type) rand() % max_pixel_value;
            input_2[i].range(55, 48) = (write_type) rand() % max_pixel_value;
            input_2[i].range(63, 56) = (write_type) rand() % max_pixel_value;

            input_2[i].range(71, 64) = (write_type) rand() % max_pixel_value;
            input_2[i].range(79, 72) = (write_type) rand() % max_pixel_value;
            input_2[i].range(87, 80) = (write_type) rand() % max_pixel_value;
            input_2[i].range(95, 88) = (write_type) rand() % max_pixel_value;

            input_2[i].range(103, 96) = (write_type) rand() % max_pixel_value;
            input_2[i].range(111, 104) = (write_type) rand() % max_pixel_value;
            input_2[i].range(119, 112) = (write_type) rand() % max_pixel_value;
            input_2[i].range(127, 120) = (write_type) rand() % max_pixel_value;
    }

    

    setup_aie(size1, size2, input_1, input_2, s_1, s_2);
    std::cout << "\n\nstart\n\n";
    
    std::ofstream file_1;
    std::ofstream file_2;
    file_1.open("../../aie/data/in_plio_source_1.txt");
    file_2.open("../../aie/data/in_plio_source_2.txt");
    if (file_1.is_open() && file_2.is_open()) {
        for (int i = 0; i < size1 / read_size + 1; i++){
            ap_1 = (stream_type) s_1.read();

            file_1 << (int) ap_1.range(7, 0) << " ";
            file_1 << (int) ap_1.range(15, 8) << " ";
            file_1 << (int) ap_1.range(23, 16) << " ";
            file_1 << (int) ap_1.range(31, 24) << " ";

            file_1 << (int) ap_1.range(39, 32) << " ";
            file_1 << (int) ap_1.range(47, 40) << " ";
            file_1 << (int) ap_1.range(55, 48) << " ";
            file_1 << (int) ap_1.range(63, 56) << " ";

            file_1 << (int) ap_1.range(71, 64) << " ";
            file_1 << (int) ap_1.range(79, 72) << " ";
            file_1 << (int) ap_1.range(87, 80) << " ";
            file_1 << (int) ap_1.range(95, 88) << " ";

            file_1 << (int) ap_1.range(103, 96) << " ";
            file_1 << (int) ap_1.range(111, 104) << " ";
            file_1 << (int) ap_1.range(119, 112) << " ";
            file_1 << (int) ap_1.range(127, 120) << " ";

            file_1 << std::endl;
        }

        for (int i = 0; i < size2 / read_size + 1; i++){
            ap_2 = (stream_type) s_2.read();

            file_2 << (int) ap_2.range(7, 0) << " ";
            file_2 << (int) ap_2.range(15, 8) << " ";
            file_2 << (int) ap_2.range(23, 16) << " ";
            file_2 << (int) ap_2.range(31, 24) << " ";

            file_2 << (int) ap_2.range(39, 32) << " ";
            file_2 << (int) ap_2.range(47, 40) << " ";
            file_2 << (int) ap_2.range(55, 48) << " ";
            file_2 << (int) ap_2.range(63, 56) << " ";

            file_2 << (int) ap_2.range(71, 64) << " ";
            file_2 << (int) ap_2.range(79, 72) << " ";
            file_2 << (int) ap_2.range(87, 80) << " ";
            file_2 << (int) ap_2.range(95, 88) << " ";

            file_2 << (int) ap_2.range(103, 96) << " ";
            file_2 << (int) ap_2.range(111, 104) << " ";
            file_2 << (int) ap_2.range(119, 112) << " ";
            file_2 << (int) ap_2.range(127, 120) << " ";

            file_2 << std::endl;
        }
        std::cout << "\n\n end \n\n";

        file_1.close();
        file_2.close();
    } else {
        std::cout << "Error opening file" << std::endl;
    }
    return 0;
}