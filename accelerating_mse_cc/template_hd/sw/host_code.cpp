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

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include "experimental/xrt_kernel.h"
#include "experimental/xrt_uuid.h"
#include "../common/common.h"
#include <ctime>
#include <math.h>

// For hw emulation, run in sw directory: source ./setup_emu.sh -s on

#define DEVICE_ID 0


// every top function input that must be passed from the host to the kernel must have a unique index starting from 0

// args indexes for setup_aie kernel
#define arg_setup_aie_size1 0
#define arg_setup_aie_size2 1
#define arg_setup_aie_coeff1 2
#define arg_setup_aie_coeff2 3
#define arg_setup_aie_input_1 4
#define arg_setup_aie_input_2 5

// args indexes for sink_from_aie kernel
#define arg_sink_from_aie_output 1
#define arg_sink_from_aie_size 2

#define max_pixel_value_1 256
#define max_pixel_value_2 100

bool get_xclbin_path(std::string& xclbin_file);
std::ostream& bold_on(std::ostream& os);
std::ostream& bold_off(std::ostream& os);

int check_result(int* input_1, int* input_2, float* output, int size) {
    std::chrono::high_resolution_clock::time_point start, end;
    std::chrono::nanoseconds time;
    start = std::chrono::high_resolution_clock::now();
    int coeff1 = get_coefficent(input_1, size);
    int coeff2 = get_coefficent(input_2, size);
    float sum = 0.0;
    for (int i = 0; i < size; i++){
        if ((int) input_1[i] / coeff1 != (int) input_2[i] / coeff2)
            sum++;
    }
    sum /= size;
    end = std::chrono::high_resolution_clock::now();
    time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "SW Time Taken: " << time.count() << " ns, ";
    if (abs(sum - output[0]) > 0.01){
        std::cout << "Error for HD --> expected: " << sum << " got:  " << output[0] << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Test passed!" << std::endl << std::endl;
    return EXIT_SUCCESS;
}

int get_coefficent(int *arr, int size){
	int max = 0;
	for (int i = 0; i < size; i++){
		if (arr[i] > size)
			max = arr[i];
	}
	return (max + 1) / 2;
}

int main(int argc, char *argv[]) {
    int size1 = 16 * 1000;
    int size2 = 16 * 1000;
    int depth1 = 10;
    int depth2 = 10;
    int output_size = 4;

    size1 = size1 * depth1;
    size2 = size2 * depth2;

    int img_ref[size1];
    int img_float[size2];

//------------------------------------------------LOADING XCLBIN------------------------------------------    
    std::string xclbin_file;
    if (!get_xclbin_path(xclbin_file))
        return EXIT_FAILURE;

    // Load xclbin
    std::cout << "1. Loading bitstream (" << xclbin_file << ")... ";
    xrt::device device = xrt::device(DEVICE_ID);
    xrt::uuid xclbin_uuid = device.load_xclbin(xclbin_file);
    std::cout << "Done" << std::endl;
//----------------------------------------------INITIALIZING THE BOARD------------------------------------------

    // create kernel objects
    xrt::kernel krnl_setup_aie  = xrt::kernel(device, xclbin_uuid, "setup_aie");
    xrt::kernel krnl_sink_from_aie  = xrt::kernel(device, xclbin_uuid, "sink_from_aie");

    // get memory bank groups for device buffer - required for axi master input/ouput
    xrtMemoryGroup bank_output  = krnl_sink_from_aie.group_id(arg_sink_from_aie_output);
    xrtMemoryGroup bank_input_1  = krnl_setup_aie.group_id(arg_setup_aie_input_1);
    xrtMemoryGroup bank_input_2  = krnl_setup_aie.group_id(arg_setup_aie_input_2);

    // create device buffers - if you have to load some data, here they are
    xrt::bo buffer_setup_aie_1 = xrt::bo(device, size1 * sizeof(int), xrt::bo::flags::normal, bank_input_1);
    xrt::bo buffer_setup_aie_2 = xrt::bo(device, size2 * sizeof(int), xrt::bo::flags::normal, bank_input_2); 
    xrt::bo buffer_sink_from_aie = xrt::bo(device, output_size * sizeof(float), xrt::bo::flags::normal, bank_output); 

    // create runner instances
    xrt::run run_setup_aie = xrt::run(krnl_setup_aie);
    xrt::run run_sink_from_aie = xrt::run(krnl_sink_from_aie);

    // set setup_aie kernel arguments
    run_setup_aie.set_arg(arg_setup_aie_size1, size1);
    run_setup_aie.set_arg(arg_setup_aie_size2, size2);
    run_setup_aie.set_arg(arg_setup_aie_input_1, buffer_setup_aie_1);
    run_setup_aie.set_arg(arg_setup_aie_input_2, buffer_setup_aie_2);

    // set sink_from_aie kernel arguments
    run_sink_from_aie.set_arg(arg_sink_from_aie_output, buffer_sink_from_aie);
    run_sink_from_aie.set_arg(arg_sink_from_aie_size, output_size);

    float output_buffer[output_size];

    int num_tests = 100;
    float sum_hw = 0;
    float sum_squared_hw = 0;
    float sum_sw = 0;
    float sum_squared_sw = 0;
    int res = EXIT_SUCCESS;
    std::chrono::high_resolution_clock::time_point start, end;
    std::chrono::nanoseconds time;
    for (int j = 0; j < num_tests; j++){
        for (int i = 0; i < size1; i++){
            img_ref[i] = rand() % max_pixel_value_1; 
        }
        for (int i = 0; i < size2; i++){
            img_float[i] = rand() % max_pixel_value_2; 
        }

        run_setup_aie.set_arg(arg_setup_aie_coeff1, get_coefficent(img_ref, size1));
        run_setup_aie.set_arg(arg_setup_aie_coeff2, get_coefficent(img_float, size2));
        // write data into the input buffer
        buffer_setup_aie_1.write(img_ref);
        buffer_setup_aie_1.sync(XCL_BO_SYNC_BO_TO_DEVICE);

        buffer_setup_aie_2.write(img_float);
        buffer_setup_aie_2.sync(XCL_BO_SYNC_BO_TO_DEVICE);

        // run the kernel
        run_sink_from_aie.start();
        run_setup_aie.start();
        start = std::chrono::high_resolution_clock::now();

        // wait for the kernel to finish
        run_setup_aie.wait();
        run_sink_from_aie.wait();
        end = std::chrono::high_resolution_clock::now();

        time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        sum_hw += time.count() / 1000000; // convert to milliseconds 
        sum_squared_hw += (time.count() / 1000000) * (time.count() / 1000000);

        // read the output buffer
        buffer_sink_from_aie.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
        buffer_sink_from_aie.read(output_buffer);

        start = std::chrono::high_resolution_clock::now();
        std::cout << "\nTest number " << j + 1 << ", HW Time taken: " << time.count() << " ns --> ";
        if (check_result(img_ref, img_float, output_buffer, size2) == EXIT_FAILURE)
            res = EXIT_FAILURE;
        end = std::chrono::high_resolution_clock::now();

        time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        sum_sw += time.count() / 1000000; // convert to milliseconds 
        sum_squared_sw += (time.count() / 1000000) * (time.count() / 1000000);
    }
    // compute the variance as Var(X) = E(X ** 2) - E(X) ** 2
    std::cout << "\nResults on " << num_tests << " tests: ";
    std::cout << "\n\nMean HW time: " << sum_hw / num_tests << " milliseconds, HW Standard Deviation: " << sqrt((sum_squared_hw / num_tests) - (sum_hw / num_tests) * (sum_hw / num_tests)) << "\n";
    std::cout << "Mean SW time: " << sum_sw / num_tests << " milliseconds, SW Standard Deviation: " << sqrt((sum_squared_sw / num_tests) - (sum_sw / num_tests) * (sum_sw / num_tests)) << "\n";
    std::cout << "\nSpeedUp factor: " << (sum_sw / num_tests) / (sum_hw / num_tests) << "\n";
    std::cout << "\nF-ratio: " << sqrt((sum_squared_sw / num_tests) - (sum_sw / num_tests) * (sum_sw / num_tests)) / sqrt((sum_squared_hw / num_tests) - (sum_hw / num_tests) * (sum_hw / num_tests)) << "\n\n";



    // ---------------------------------CONFRONTO PER VERIFICARE L'ERRORE--------------------------------------
        
    // Here there should be a code for checking correctness of your application, like a software application

    return res;
}


bool get_xclbin_path(std::string& xclbin_file) {
    // Judge emulation mode accoring to env variable
    char *env_emu;
    if (env_emu = getenv("XCL_EMULATION_MODE")) {
        std::string mode(env_emu);
        if (mode == "hw_emu")
        {
            std::cout << "Program running in hardware emulation mode" << std::endl;
            xclbin_file = "overlay_hw_emu.xclbin";
        }
        else
        {
            std::cout << "[ERROR] Unsupported Emulation Mode: " << mode << std::endl;
            return false;
        }
    }
    else {
        std::cout << bold_on << "Program running in hardware mode" << bold_off << std::endl;
        xclbin_file = "overlay_hw.xclbin";
    }

    std::cout << std::endl << std::endl;
    return true;
}

std::ostream& bold_on(std::ostream& os)
{
    return os << "\e[1m";
}

std::ostream& bold_off(std::ostream& os)
{
    return os << "\e[0m";
}