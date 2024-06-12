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

#define max_pixel_value 256

void read_from_stream(int *buffer, hls::stream<int> &stream, size_t size) {
    for (unsigned int i = 0; i < size; i++) {
        buffer[i] = stream.read();
    }
}

int main(int argc, char* argv[]) {
    srand(time(0));

    hls::stream<stream_type> s_1;
    hls::stream<stream_type> s_2;
    int size1 = 64;
    int size2 = 64;
    int depth = 3;

    size1 *= depth;
    size2 *= depth;

    int *input_1 = new int[size1];
    int *input_2 = new int[size2];
    for (int i = 0; i < size1; i++) {
        input_1[i] = rand() % max_pixel_value; 
    }
    for (int i = 0; i < size2; i++) {
        input_2[i] = rand() % max_pixel_value; 
    }

    setup_aie(size1, size2, input_1, input_2, s_1, s_2);
    std::cout << "\n\nstart\n\n";
    
    std::ofstream file_1;
    std::ofstream file_2;
    file_1.open("../../aie/data/in_plio_source_1.txt");
    file_2.open("../../aie/data/in_plio_source_2.txt");
    if (file_1.is_open() && file_2.is_open()) {
        stream_type ap_1;
	    stream_type ap_2;
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

        for (int i = 0; i < size2 / read_size + 2; i++){
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

        file_1.close();
        file_2.close();
    } else {
        std::cout << "Error opening file" << std::endl;
    }

    delete[] input_1;
    delete[] input_2;
    return 0;
}