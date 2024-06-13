#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include "experimental/xrt_kernel.h"
#include "experimental/xrt_uuid.h"
#include "../common/common.h"

// For hw emulation, run in sw directory: source ./setup_emu.sh -s on

#define DEVICE_ID 0


// every top function input that must be passed from the host to the kernel must have a unique index starting from 0

// args indexes for setup_aie kernel
#define arg_setup_aie_size1 0
#define arg_setup_aie_size2 1
#define arg_setup_aie_input_1 2
#define arg_setup_aie_input_2 3

// args indexes for sink_from_aie kernel
#define arg_sink_from_aie_output 1
#define arg_sink_from_aie_size 2

bool get_xclbin_path(std::string& xclbin_file);
std::ostream& bold_on(std::ostream& os);
std::ostream& bold_off(std::ostream& os);

int check_result(int* input_1, int* input_2, float* output, int size) {
    int sum = 0;
    float mse;
    for (int i = 0; i < size; i++){
        sum += (input_1[i] - input_2[i]) * (input_1[i] - input_2[i]);
    }
    mse = (float) sum / size;
    if (mse != output[0]){
        std::cout << "Error for MSE --> expected: " << mse << " got:  " << output[0] << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "MSE Test passed!" << std::endl;
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    int size1 = 512;
    int size2 = 512;
    int depth1 = 10;
    int depth2 = 10;

    size1 = size1 * depth1;
    size2 = size2 * depth2;

    int img_ref[size1];
    int img_float[size2];
    for (int i = 0; i < size1; i++){
        img_ref[i] = 2;
    }
    for (int i = 0; i < size2; i++){
        img_float[i] = 3;
    }


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
    xrt::bo buffer_sink_from_aie = xrt::bo(device, sizeof(float), xrt::bo::flags::normal, bank_output); 

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
    run_sink_from_aie.set_arg(arg_sink_from_aie_size, 1);

    // write data into the input buffer
    buffer_setup_aie_1.write(img_ref);
    buffer_setup_aie_1.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    buffer_setup_aie_2.write(img_float);
    buffer_setup_aie_2.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    // run the kernel
    run_sink_from_aie.start();
    run_setup_aie.start();

    // wait for the kernel to finish
    run_setup_aie.wait();
    run_sink_from_aie.wait();

    // read the output buffer
    buffer_sink_from_aie.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    float output_buffer[1];
    buffer_sink_from_aie.read(output_buffer);

    // ---------------------------------CONFRONTO PER VERIFICARE L'ERRORE--------------------------------------
        
    // Here there should be a code for checking correctness of your application, like a software application

    //return check_result(img_ref, img_float, output_buffer, size1);
    std::cout << output_buffer[0] << std::endl;
    return 0;

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