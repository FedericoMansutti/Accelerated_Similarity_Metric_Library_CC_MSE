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
#include "../sink_from_aie.cpp"
#include <cmath>

#define read_size 16 // size of each line of the file (number of pixel)
#define output_size 1 // size of the output we write to the output file (number of values)

float cc(int size, float *img_ref, float *img_float){
    unsigned long long int num;
    unsigned long long int denom_1 = 0;
    unsigned long long int denom_2 = 0;
    float cc;
    for (int i = 0; i < size; i++){
        num += img_ref[i] * img_float[i];
        denom_1 += img_ref[i] * img_ref[i];
        denom_2 += img_float[i] * img_float[i];  
    }
    cc = (float) (num * num) / (denom_1 * denom_2);
    return cc;
}

int main(int argc, char *argv[]) { 
    // This testbech will test the sink_from_aie kernel
    // The kernel will receive a stream of data from the AIE
    // and will write it into memory

    // I will create a stream of data
    hls::stream<float> s;
    //two files containing two images
    std::ifstream file_in_1;
    std::ifstream file_in_2;
    file_in_1.open("../../aie/data/in_plio_source_1.txt");
    file_in_2.open("../../aie/data/in_plio_source_2.txt");
    if (!file_in_1 || !file_in_2) {
        std::cerr << "Unable to open file ../../aie/data/in_plio_source_1.txt or file ../../aie/data/in_plio_source_2.txt";
        return 1;
    }

    int input_size1 = 0;
    int input_size2 = 0;
    unsigned int multiplier = 1;
    int temp;
    //calculates image size of image1 and image2
    for (int i = 0; i < read_size; i++){
        file_in_1 >> temp;
        input_size1 += multiplier * temp;
        file_in_2 >> temp;
        input_size2 += multiplier * temp;
        multiplier *= 10;
    }

    for (int i = 1; i < read_size; i++){
        file_in_1 >> temp;
        file_in_2 >> temp;
    }

    // I create the buffer to write into memory
    float *buffer = new float[output_size];
    float *real_values = new float[output_size];
    float *img_ref = new float[input_size1];
    float *img_float = new float[input_size2];

    // I have to read the output of AI Engine from the file. 
    // Otherwise, I have no input for my testbench
    std::ifstream file_out;
    file_out.open("../../aie/x86simulator_output/data/out_plio_sink_1.txt");
    if (!file_out) {
        std::cerr << "Unable to open file ../../aie/x86simulator_output/out_plio_sink.txt";
        return 1;
    }

    for (int i = 0; i < output_size; i++) {
        float x;
        file_out >> x;
        s.write(x);
    }

    sink_from_aie(s, buffer, output_size);

    for (int i = 0; i < input_size1; i++){
        file_in_1 >> img_ref[i];
    }   
    for (int i = 0; i < input_size2; i++){
        file_in_2 >> img_float[i];
    } 

    real_values[0] = cc(input_size2, img_ref, img_float);
   

    // if the kernel is correct, it will contains the expected data.
    // I can print them, for example, to check that they are equal to the output of AIE
    std::cout << std::endl << std::endl;
    bool error = false;
    for (unsigned int i = 0; i < output_size; i++) {
        if (abs(buffer[i] - real_values[i]) >= 0.01){
            error = true;
            std::cout << "value of cc is wrong, iteration number: " << i << " --> got " << buffer[i] << " expected " << real_values[i] << std::endl;
        }
        else{
            std::cout << "value of cc is right, iteration number: " << i << ", value = " << buffer[i] << std::endl;
        }
    }
    if(!error){
        std::cout << "Test Passed!" << std::endl;
    }
    else{
        std::cout << "Test NOT Passed!" << std::endl;
    }
    std::cout << std::endl << std::endl;
    delete[] buffer;
    delete[] real_values;
    delete[] img_ref;
    delete[] img_float;

    // Note that: you may also have a code that runs the AI Engine from your kernel, and so a testbench
    // that simulates the entire application flow. It is useful, but still I would suggest to use single kernel testbench too.
}