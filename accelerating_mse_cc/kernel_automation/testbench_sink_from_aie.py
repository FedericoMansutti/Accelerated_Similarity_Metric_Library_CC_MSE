def test_sink_from_aie(kernel_count, metric):
    if metric not in ["mse", "cc", "psnr"]:
        print("You chose an invalid metric: " + metric)
        return
    if metric == "mse":
        return build_mse(kernel_count)
    elif metric == "psnr":
        return build_psnr(kernel_count)
    else:
        return build_cc(kernel_count)

def build_mse(kernel_count):
    # Original code parts
    includes = '''#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ap_axi_sdata.h>
#include "../sink_from_aie.cpp"
#include <cmath>

#define read_size 16 // size of each line of the file (number of pixel)
#define output_size 4 // size of the output we write to the output file (number of values)
#define kernel_count ''' + str(kernel_count) + '''

float mse(int size, float *img_ref, float *img_float){
    unsigned long long int tot = 0;
    for (int i = 0; i < size; i++){
        tot += (img_ref[i] - img_float[i]) * (img_ref[i] - img_float[i]);
    }
    return (float) tot / size;
}

int main(int argc, char *argv[]) { 
    printf("\\n\\nstart the SINK\\n\\n");
    // This testbech will test the sink_from_aie kernel
    // The kernel will receive a stream of data from the AIE
    // and will write it into memory

    // I will create a stream of data
    hls::stream<float> s;
'''

    # File stream declarations
    file_declarations = []
    for i in range(1, kernel_count * 2 + 1):
        file_declarations.append(f'    std::ifstream file_in_{i};')
    file_declarations = '\n'.join(file_declarations)

    open_files = []
    for i in range(1, kernel_count * 2 + 1):
        open_files.append(f'    file_in_{i}.open("../../aie/data/in_plio_source_{i}.txt");')
    open_files = '\n'.join(open_files)

    check_files = []
    check_conditions = ' || '.join([f'!file_in_{i}' for i in range(1, kernel_count * 2 + 1)])
    check_files.append(f'    if ({check_conditions}) {{')
    check_files.append('        std::cerr << "Unable to open input files";')
    check_files.append('        return 1;')
    check_files.append('    }')
    check_files = '\n'.join(check_files)

    remaining_code = '''    int input_size = 0;
    unsigned int multiplier = 1;
    int temp;
    //calculates image size of image1 and image2
    for (int i = 0; i < read_size; i++){
        file_in_1 >> temp;
        printf("%d ", temp);
        input_size += multiplier * temp;
        multiplier *= 10;
        // consume useless size
'''

    consume_useless_size = []
    for i in range(2, kernel_count * 2 + 1):
        consume_useless_size.append(f'        file_in_{i} >> temp;')
    consume_useless_size = '\n'.join(consume_useless_size)

    remaining_code += consume_useless_size + '\n    }\n'

    remaining_code += '''    // I create the buffer to write into memory
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
'''

    loop = [f'''for (int i = {i} * (input_size / kernel_count); i < {i + 1} * (input_size / kernel_count); i++){{
        file_in_{i * 2 + 1} >> img_ref[i];
        file_in_{i * 2 + 2} >> img_float[i];
    }}''' for i in range(kernel_count)]
    loop = "\n\t".join(loop)

    remaining_code_2 = f'''
    {loop}

    real_values[0] = mse(input_size, img_ref, img_float);
   

    // if the kernel is correct, it will contains the expected data.
    // I can print them, for example, to check that they are equal to the output of AIE
    std::cout << std::endl << std::endl;
    bool error = false;
    if (abs(buffer[0] - real_values[0]) / real_values[0] > 0.01){{
        error = true;
        std::cout << "value of mse is wrong, iteration number: " << 0 << " --> got " << buffer[0] << " expected " << real_values[0] << std::endl;
    }}
    else{{
        std::cout << "value of mse is right, iteration number: " << 0 << ", value = " << buffer[0] << std::endl;
    }}
    if(!error){{
        std::cout << "Test Passed!" << std::endl;
    }}
    else{{
        std::cout << "Test NOT Passed!" << std::endl;
    }}
    std::cout << std::endl << std::endl;
    delete[] buffer;
    delete[] real_values;
    delete[] img_ref;
    delete[] img_float;

    // Note that: you may also have a code that runs the AI Engine from your kernel, and so a testbench
    // that simulates the entire application flow. It is useful, but still I would suggest to use single kernel testbench too.
}}
'''

    # Combining all parts together
    cpp_code = f'{includes}\n{file_declarations}\n{open_files}\n{check_files}\n{remaining_code}\n{remaining_code_2}'

    return cpp_code


def build_psnr(kernel_count):
    # Original code parts
    includes = '''#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ap_axi_sdata.h>
#include "../sink_from_aie.cpp"
#include <cmath>

#define read_size 16 // size of each line of the file (number of pixel)
#define output_size 4 // size of the output we write to the output file (number of values)
#define kernel_count ''' + str(kernel_count) + '''

float psnr(int size, float *img_ref, float *img_float){
    unsigned long long int tot = 0;
    for (int i = 0; i < size; i++){
        tot += (img_ref[i] - img_float[i]) * (img_ref[i] - img_float[i]);
    }
    return from_mse_to_psnr((float) tot / size);
}

int main(int argc, char *argv[]) { 
    printf("\\n\\nstart the SINK\\n\\n");
    // This testbech will test the sink_from_aie kernel
    // The kernel will receive a stream of data from the AIE
    // and will write it into memory

    // I will create a stream of data
    hls::stream<float> s;
'''

    # File stream declarations
    file_declarations = []
    for i in range(1, kernel_count * 2 + 1):
        file_declarations.append(f'    std::ifstream file_in_{i};')
    file_declarations = '\n'.join(file_declarations)

    open_files = []
    for i in range(1, kernel_count * 2 + 1):
        open_files.append(f'    file_in_{i}.open("../../aie/data/in_plio_source_{i}.txt");')
    open_files = '\n'.join(open_files)

    check_files = []
    check_conditions = ' || '.join([f'!file_in_{i}' for i in range(1, kernel_count * 2 + 1)])
    check_files.append(f'    if ({check_conditions}) {{')
    check_files.append('        std::cerr << "Unable to open input files";')
    check_files.append('        return 1;')
    check_files.append('    }')
    check_files = '\n'.join(check_files)

    remaining_code = '''    int input_size = 0;
    unsigned int multiplier = 1;
    int temp;
    //calculates image size of image1 and image2
    for (int i = 0; i < read_size; i++){
        file_in_1 >> temp;
        printf("%d ", temp);
        input_size += multiplier * temp;
        multiplier *= 10;
        // consume useless size
'''

    consume_useless_size = []
    for i in range(2, kernel_count * 2 + 1):
        consume_useless_size.append(f'        file_in_{i} >> temp;')
    consume_useless_size = '\n'.join(consume_useless_size)

    remaining_code += consume_useless_size + '\n    }\n'

    remaining_code += '''    // I create the buffer to write into memory
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
'''

    loop = [f'''for (int i = {i} * (input_size / kernel_count); i < {i + 1} * (input_size / kernel_count); i++){{
        file_in_{i * 2 + 1} >> img_ref[i];
        file_in_{i * 2 + 2} >> img_float[i];
    }}''' for i in range(kernel_count)]
    loop = "\n\t".join(loop)

    remaining_code_2 = f'''
    {loop}

    real_values[0] = psnr(input_size, img_ref, img_float);
   

    // if the kernel is correct, it will contains the expected data.
    // I can print them, for example, to check that they are equal to the output of AIE
    std::cout << std::endl << std::endl;
    bool error = false;
    if (abs(buffer[0] - real_values[0]) / real_values[0] > 0.01){{
        error = true;
        std::cout << "value of psnr is wrong, iteration number: " << 0 << " --> got " << buffer[0] << " expected " << real_values[0] << std::endl;
    }}
    else{{
        std::cout << "value of psnr is right, iteration number: " << 0 << ", value = " << buffer[0] << std::endl;
    }}
    if(!error){{
        std::cout << "Test Passed!" << std::endl;
    }}
    else{{
        std::cout << "Test NOT Passed!" << std::endl;
    }}
    std::cout << std::endl << std::endl;
    delete[] buffer;
    delete[] real_values;
    delete[] img_ref;
    delete[] img_float;

    // Note that: you may also have a code that runs the AI Engine from your kernel, and so a testbench
    // that simulates the entire application flow. It is useful, but still I would suggest to use single kernel testbench too.
}}
'''

    # Combining all parts together
    cpp_code = f'{includes}\n{file_declarations}\n{open_files}\n{check_files}\n{remaining_code}\n{remaining_code_2}'

    return cpp_code

def build_cc(kernel_count):
        # Original code parts
    includes = '''#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ap_axi_sdata.h>
#include "../sink_from_aie.cpp"
#include <cmath>

#define read_size 16 // size of each line of the file (number of pixel)
#define output_size 4 // size of the output we write to the output file (number of values)
#define kernel_count ''' + str(kernel_count) + '''

float cc(int size, float *img_ref, float *img_float){
    unsigned long long int num = 0;
    unsigned long long int denom_1 = 0;
    unsigned long long int denom_2 = 0;
    for (int i = 0; i < size; i++){
        num += img_ref[i] * img_float[i];
        denom_1 += img_ref[i] * img_ref[i];
        denom_2 += img_float[i] * img_float[i];
    }
    return (float) (num * num) / (denom_1 * denom_2);
}

int main(int argc, char *argv[]) { 
    printf("\\n\\nstart the SINK\\n\\n");
    // This testbech will test the sink_from_aie kernel
    // The kernel will receive a stream of data from the AIE
    // and will write it into memory

    // I will create a stream of data
    hls::stream<float> s;
'''

    # File stream declarations
    file_declarations = []
    for i in range(1, kernel_count * 2 + 1):
        file_declarations.append(f'    std::ifstream file_in_{i};')
    file_declarations = '\n'.join(file_declarations)

    open_files = []
    for i in range(1, kernel_count * 2 + 1):
        open_files.append(f'    file_in_{i}.open("../../aie/data/in_plio_source_{i}.txt");')
    open_files = '\n'.join(open_files)

    check_files = []
    check_conditions = ' || '.join([f'!file_in_{i}' for i in range(1, kernel_count * 2 + 1)])
    check_files.append(f'    if ({check_conditions}) {{')
    check_files.append('        std::cerr << "Unable to open input files";')
    check_files.append('        return 1;')
    check_files.append('    }')
    check_files = '\n'.join(check_files)

    remaining_code = '''    int input_size = 0;
    unsigned int multiplier = 1;
    int temp;
    //calculates image size of image1 and image2
    for (int i = 0; i < read_size; i++){
        file_in_1 >> temp;
        printf("%d ", temp);
        input_size += multiplier * temp;
        multiplier *= 10;
        // consume useless size
'''

    consume_useless_size = []
    for i in range(2, kernel_count * 2 + 1):
        consume_useless_size.append(f'        file_in_{i} >> temp;')
    consume_useless_size = '\n'.join(consume_useless_size)

    remaining_code += consume_useless_size + '\n    }\n'

    remaining_code += '''    // I create the buffer to write into memory
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
'''

    loop = [f'''for (int i = {i} * (input_size / kernel_count); i < {i + 1} * (input_size / kernel_count); i++){{
        file_in_{i * 2 + 1} >> img_ref[i];
        file_in_{i * 2 + 2} >> img_float[i];
    }}''' for i in range(kernel_count)]
    loop = "\n\t".join(loop)

    remaining_code_2 = f'''
    {loop}

    real_values[0] = cc(input_size, img_ref, img_float);
   

    // if the kernel is correct, it will contains the expected data.
    // I can print them, for example, to check that they are equal to the output of AIE
    std::cout << std::endl << std::endl;
    bool error = false;
    if (abs(buffer[0] - real_values[0]) > 0.01){{
        error = true;
        std::cout << "value of cc is wrong, iteration number: " << 0 << " --> got " << buffer[0] << " expected " << real_values[0] << std::endl;
    }}
    else{{
        std::cout << "value of cc is right, iteration number: " << 0 << ", value = " << buffer[0] << std::endl;
    }}
    if(!error){{
        std::cout << "Test Passed!" << std::endl;
    }}
    else{{
        std::cout << "Test NOT Passed!" << std::endl;
    }}
    std::cout << std::endl << std::endl;
    delete[] buffer;
    delete[] real_values;
    delete[] img_ref;
    delete[] img_float;

    // Note that: you may also have a code that runs the AI Engine from your kernel, and so a testbench
    // that simulates the entire application flow. It is useful, but still I would suggest to use single kernel testbench too.
}}
'''

    # Combining all parts together
    cpp_code = f'{includes}\n{file_declarations}\n{open_files}\n{check_files}\n{remaining_code}\n{remaining_code_2}'

    return cpp_code