#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ap_axi_sdata.h>
#include "../sink_from_aie.cpp"
#include <cmath>

#define read_size 16 // size of each line of the file (number of pixel)
#define output_size 4 // size of the output we write to the output file (number of values)
#define kernel_count 2

float mse(int size, float *img_ref, float *img_float){
    unsigned long long int tot = 0;
    for (int i = 0; i < size; i++){
        tot += (img_ref[i] - img_float[i]) * (img_ref[i] - img_float[i]);
    }
    return (float) tot / size;
}

int main(int argc, char *argv[]) { 
    printf("\n\nstart the SINK\n\n");
    // This testbech will test the sink_from_aie kernel
    // The kernel will receive a stream of data from the AIE
    // and will write it into memory

    // I will create a stream of data
    hls::stream<float> s;

    std::ifstream file_in_1;
    std::ifstream file_in_2;
    std::ifstream file_in_3;
    std::ifstream file_in_4;
    file_in_1.open("../../aie/data/in_plio_source_1.txt");
    file_in_2.open("../../aie/data/in_plio_source_2.txt");
    file_in_3.open("../../aie/data/in_plio_source_3.txt");
    file_in_4.open("../../aie/data/in_plio_source_4.txt");
    if (!file_in_1 || !file_in_2 || !file_in_3 || !file_in_4) {
        std::cerr << "Unable to open input files";
        return 1;
    }
    int input_size = 0;
    unsigned int multiplier = 1;
    int temp;
    //calculates image size of image1 and image2
    for (int i = 0; i < read_size; i++){
        file_in_1 >> temp;
        printf("%d ", temp);
        input_size += multiplier * temp;
        multiplier *= 10;
        // consume useless size
        file_in_2 >> temp;
        file_in_3 >> temp;
        file_in_4 >> temp;
    }
    // I create the buffer to write into memory
    float *buffer = new float[output_size];
    float *real_values = new float[output_size];
    float *img_ref = new float[input_size];
    float *img_float = new float[input_size];

    for(int i = 0; i < input_size; i++){
        img_ref[i] = 0.0;
        img_float[i] = 0.0;
    }

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


    std::ifstream files[] = {file_in_1, file_in_2, file_in_3, file_in_4};


    for (int j = 0; j < kernel_count; j++){
        for (int i = j * (input_size / kernel_count); i < (j + 1) * (input_size / kernel_count); i++){
            files[2 * j + 1] >> img_ref[i];
            files[2 * j + 2] >> img_float[i];
        }   
    }

    real_values[0] = mse(input_size, img_ref, img_float);
   

    // if the kernel is correct, it will contains the expected data.
    // I can print them, for example, to check that they are equal to the output of AIE
    std::cout << std::endl << std::endl;
    bool error = false;
    if (abs(buffer[0] - real_values[0]) / real_values[0] > 0.01){
        error = true;
        std::cout << "value of mse is wrong, iteration number: " << 0 << " --> got " << buffer[0] << " expected " << real_values[0] << std::endl;
    }
    else{
        std::cout << "value of mse is right, iteration number: " << 0 << ", value = " << buffer[0] << std::endl;
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
