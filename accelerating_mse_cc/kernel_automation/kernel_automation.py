def create_kernel(kernel_count, metric):
    if metric not in ["mse", "cc", "psnr", "scc", "rmse"]:
        print("The metric you chose is not valid: " + metric)
    if metric in ["mse", "psnr", "rmse"]:
        return create_kernel_mse(kernel_count)
    elif metric == "scc":
        return create_kernel_scc(kernel_count)
    else:
        return create_kernel_cc

def create_kernel_mse(kernel_count):
    preamble = '''#include "my_kernel_1.h"
#include "common.h"
#include "aie_api/aie.hpp"
#include "aie_api/aie_adf.hpp"
#include "aie_api/utils.hpp"

#define vector_size 32 // Size of the vector unit, used for reading from input stream
#define num_partitions 20 // Number of partitions for partial accumulation

#define read_type uint8 // type of the input stream
#define func_type int // type used for doing the computations
#define write_type float // type of the output stream
#define kernel_count ''' + str(kernel_count) + '''

//API REFERENCE for STREAM: 
// https://docs.amd.com/r/ehttps://docs.amd.com/r/en-US/ug1079-ai-engine-kernel-coding/Reading-and-Advancing-an-Input-Streamn-US/ug1079-ai-engine-kernel-coding/Reading-and-Advancing-an-Input-Stream

inline aie::vector<func_type,vector_size> convert_to_func_type(aie::vector<read_type,vector_size> vec) {
    aie::accum<acc32, vector_size> acc;
    acc.from_vector(vec, 0);
    return acc.to_vector<func_type>();
}

void my_kernel_function (input_stream<read_type>* restrict input_1, input_stream<read_type>* restrict input_2, output_stream<write_type>* restrict output) {
    aie::vector<read_type, vector_size/2> size_vec1 = readincr_v<vector_size/2>(input_1);
    aie::vector<read_type, vector_size/2> size_vec2 = readincr_v<vector_size/2>(input_2);

    if(aie::equal(size_vec1, size_vec2)){
        printf("\\n\\nThe two images have the same size\\n\\n");
    }
    else{
        printf("\\n\\nWarning: The number of pixels of the two images is different, it has not yet been implemented...\\n");
        return;
    }

    unsigned int size1 = 0;
    unsigned int multiplier = 1;
    int first_zero = 0;
    for (int i = vector_size/2 - 1; i >= 0; i--){
        if (size_vec1[i] != 0){
            first_zero = i;
            break;
        }
    }
    for (int i = 0; i <= first_zero; i++){
        size1 += multiplier * size_vec1[i];
        multiplier *= 10;
    }

    if (size1 % vector_size != 0) 
        printf("\\n\\nWarning: The number of pixel is not divisible by 32, the image will be truncated...\\n");

    unsigned long long int partial_sums[num_partitions] = {0};

    aie::vector<func_type, vector_size> diff;
    // Process each vector with partial accumulators
    int loop_count = size1 / (vector_size * kernel_count);
    for (int i = 0; i < loop_count; i++) 
    chess_loop_range(4, )
    chess_prepare_for_pipelining
    {
        diff = aie::sub(convert_to_func_type(readincr_v<vector_size>(input_1)), convert_to_func_type(readincr_v<vector_size>(input_2))); // compute difference vectorized
        partial_sums[i % num_partitions] += aie::reduce_add(aie::mul(diff, diff).to_vector<func_type>()); // add to partial sums, after conversion from accumulator to vector
    }

    unsigned long long int final_sum = 0; // final sum for the MSE
    for (int i = 0; i < num_partitions; i++) 
    chess_loop_range(num_partitions, )
    {
        final_sum += partial_sums[i]; // "reduce" phase of map-reduce, combining all the partial sums
    }

    write_type res = (write_type) final_sum / size1;
    aie::vector<write_type, 4> mse;
    mse.set(res, 0);
    writeincr(output, mse); // divide and write to get the real MSE
}
'''

    function_template = '''void sum_kernels ({inputs}output_stream<write_type>* restrict output){{
    {mse_declarations}
    {mse_sum}
    aie::vector<write_type, 4> mse;
    mse.set(mse_tot, 0);
    writeincr(output, mse);
}}'''

    # Generate the input arguments based on the kernel count
    input_args = ', '.join([f'input_stream<float>* restrict kernel_{i}' for i in range(1, kernel_count + 1)])

    # Generate the MSE declarations
    mse_declarations = '\n    '.join(
        [f'float mse{i} = readincr_v<4>(kernel_{i})[0];' for i in range(1, kernel_count + 1)])

    # Generate the MSE summation
    mse_sum = ' + '.join([f'mse{i}' for i in range(1, kernel_count + 1)])
    mse_sum = f'float mse_tot = {mse_sum};'

    # Combine all parts into the function template
    sum_kernels_function = function_template.format(inputs=input_args + ', ', mse_declarations=mse_declarations,
                                                    mse_sum=mse_sum)

    # Combine the preamble and the generated sum_kernels function
    full_code = preamble + '\n' + sum_kernels_function

    return full_code

def create_kernel_scc(kernel_count):
    # Header
    cpp_code = '''
#include "my_kernel_1.h"
#include "common.h"
#include "aie_api/aie.hpp"
#include "aie_api/aie_adf.hpp"
#include "aie_api/utils.hpp"

#define vector_size 32 // Size of the vector unit, used for reading from input stream
#define num_partitions 20 // Number of partitions for partial accumulation

#define read_type uint8 // type of the input stream
#define func_type int // type used for doing the computations
#define write_type float // type of the output stream
#define kernel_count {kernel_count}
'''.format(kernel_count=kernel_count)
    
    # Split function
    cpp_code += '''
void split(unsigned long long int original, int &high, int &low) {
    high = static_cast<int>(original >> 32); // Get the higher 32 bits
    low = static_cast<int>(original & 0xFFFFFFFF); // Get the lower 32 bits
}

unsigned long long int recompose(int high, int low) {
    // recompose in a single unsigned long long int
    return (static_cast<unsigned long long int>(high) << 32) | (static_cast<unsigned long long int>(low) & 0xFFFFFFFF);
}
'''

    # my_kernel_function
    cpp_code += '''
void my_kernel_function (input_stream<read_type>* restrict input_1, input_stream<read_type>* restrict input_2, output_stream<int>* restrict output) {
    aie::vector<read_type, vector_size/2> size_vec1 = readincr_v<vector_size/2>(input_1);
    aie::vector<read_type, vector_size/2> size_vec2 = readincr_v<vector_size/2>(input_2);

    if(aie::equal(size_vec1, size_vec2)){
        printf("\\n\\nThe two images have the same size\\n\\n");
    }
    else{
        printf("\\n\\nWarning: The number of pixels of the two images is different, it has not yet been implemented...\\n");
        return;
    }
    // first of all, we reconstruct the size of the images
    unsigned int size1 = 0;
    unsigned int multiplier = 1;
    int first_zero = 0;
    for (int i = vector_size/2 - 1; i >= 0; i--){
        if (size_vec1[i] != 0){
            first_zero = i;
            break;
        }
    }
    for (int i = 0; i <= first_zero; i++){
        size1 += multiplier * size_vec1[i];
        multiplier *= 10;
    }

    if (size1 % vector_size != 0) 
        printf("\\n\\nWarning: The number of pixel is not divisible by the read size, the image will be truncated...\\n");

    // divide the computations in partial components
    unsigned long long int partial_num[num_partitions] = {0};
    unsigned long long int partial_denom_1[num_partitions] = {0};
    unsigned long long int partial_denom_2[num_partitions] = {0};

    aie::vector<read_type, vector_size> vec_1;
    aie::vector<read_type, vector_size> vec_2;
    // Process each vector with partial accumulators
    int loop_count = size1 / (vector_size * kernel_count);
    // in this loop, each iteration does not deppend on the 20 itarations before
    for (int i = 0; i < loop_count; i++) 
    chess_loop_range(4, )
    chess_prepare_for_pipelining
    {
        vec_1 = readincr_v<vector_size>(input_1);
        vec_2 = readincr_v<vector_size>(input_2);
        partial_num[i % num_partitions] += aie::reduce_add(aie::mul(vec_1, vec_2).to_vector<func_type>());
        partial_denom_1[i % num_partitions] += aie::reduce_add(aie::mul(vec_1, vec_1).to_vector<func_type>());
        partial_denom_2[i % num_partitions] += aie::reduce_add(aie::mul(vec_2, vec_2).to_vector<func_type>());
    }
    // now we put all the partial results togheter, so that we have the final result
    unsigned long long int num = 0; 
    unsigned long long int denom_1 = 0; 
    unsigned long long int denom_2 = 0; 
    for (int i = 0; i < num_partitions; i++) 
    chess_loop_range(num_partitions, )
    {
        num += partial_num[i];
        denom_1 += partial_denom_1[i];
        denom_2 += partial_denom_2[i];
    }
    //float res = (float) (num * num) / (denom_1 * denom_2);
    // pass the this result to the final kernel
    int high, low;

    aie::vector<int, 8> cc;
    split(num, high, low);
    cc.set(high, 0);
    cc.set(low, 1);

    split(denom_1, high, low);
    cc.set(high, 2);
    cc.set(low, 3);

    split(denom_2, high, low);
    cc.set(high, 4);
    cc.set(low, 5);

    writeincr(output, cc);
}
'''

    # sum_kernels function with dynamic handling of kernel_count
    cpp_code += '''
void sum_kernels ('''
    for i in range(1, kernel_count + 1):
        cpp_code += 'input_stream<int>* restrict kernel_{}, '.format(i)
    cpp_code += 'output_stream<write_type>* restrict output){\n'
    
    for i in range(1, kernel_count + 1):
        cpp_code += '    aie::vector<int, 8> k{0} = readincr_v<8>(kernel_{0});\n'.format(i)
    
    num = [f"recompose(k{i + 1}[0], k{i + 1}[1])" for i in range(kernel_count)]
    num = " + ".join(num)

    denom_1 = [f"recompose(k{i + 1}[2], k{i + 1}[3])" for i in range(kernel_count)]
    denom_1 = " + ".join(denom_1)

    denom_2 = [f"recompose(k{i + 1}[4], k{i + 1}[5])" for i in range(kernel_count)]
    denom_2 = " + ".join(denom_2)

    cpp_code += f'''
    // put all the partial metrics together, and obtain the real metric
    unsigned long long int num = {num};
    unsigned long long int denom_1 = {denom_1};
    unsigned long long int denom_2 = {denom_2};

    float res = (float) (num * num) / (denom_1 * denom_2);

    aie::vector<float, 4> cc;
    cc.set(res , 0);
    writeincr(output, cc);
'''
    return cpp_code + "\n}"


def create_kernel_header(kernel_count, metric):
    if metric not in ["mse", "cc", "psnr"]:
        print("The metric you chose is not valid: " + metric)
    if metric in ["mse", "psnr"]:
        return create_header_mse_psnr(kernel_count)
    else:
        return create_header_cc(kernel_count)

def create_header_mse_psnr(kernel_count):
    # Define the initial part of the header
    header = """#pragma once
#include <adf.h>

#define stream_type float

#define read_type uint8 // type of the input stream
#define write_type float // type of the output stream

void my_kernel_function (input_stream<read_type>* restrict input_1, input_stream<read_type>* restrict input_2, output_stream<write_type>* restrict output);
"""

    # Start the sum_kernels function
    header += "\nvoid sum_kernels ("

    # Add input streams based on the number of kernels
    for i in range(1, kernel_count + 1):
        header += f"input_stream<float>* restrict kernel_{i}, "

    # Add the output stream and close the function declaration
    header += "output_stream<write_type>* restrict output);\n"

    return header

def create_header_cc(kernel_count):
    cpp_code = '''
#pragma once
#include <adf.h>

#define stream_type float

#define read_type uint8 // type of the input stream
#define write_type float // type of the output stream
#define kernel_count {kernel_count}
'''.format(kernel_count=kernel_count)
    
    # Function prototypes
    cpp_code += '''
void my_kernel_function (input_stream<read_type>* restrict input_1, input_stream<read_type>* restrict input_2, output_stream<int>* restrict output);

void sum_kernels ('''
    for i in range(1, kernel_count + 1):
        cpp_code += 'input_stream<int>* restrict kernel_{}, '.format(i)
    cpp_code += 'output_stream<write_type>* restrict output);\n'

    return cpp_code

def create_kernel_cc(kernel_count):
    # Header
    cpp_code = '''
#include "my_kernel_1.h"
#include "common.h"
#include "aie_api/aie.hpp"
#include "aie_api/aie_adf.hpp"
#include "aie_api/utils.hpp"

#define vector_size 32 // Size of the vector unit, used for reading from input stream
#define num_partitions 20 // Number of partitions for partial accumulation

#define read_type uint8 // type of the input stream
#define func_type int // type used for doing the computations
#define write_type float // type of the output stream
#define kernel_count {kernel_count}
'''.format(kernel_count=kernel_count)
    
    # Split function
    cpp_code += '''
void split(unsigned long long int original, int &high, int &low) {
    high = static_cast<int>(original >> 32); // Get the higher 32 bits
    low = static_cast<int>(original & 0xFFFFFFFF); // Get the lower 32 bits
}

unsigned long long int recompose(int high, int low) {
    // recompose in a single unsigned long long int
    return (static_cast<unsigned long long int>(high) << 32) | (static_cast<unsigned long long int>(low) & 0xFFFFFFFF);
}
'''

    # my_kernel_function
    cpp_code += '''
void my_kernel_function (input_stream<read_type>* restrict input_1, input_stream<read_type>* restrict input_2, output_stream<int>* restrict output) {
    aie::vector<read_type, vector_size/2> size_vec1 = readincr_v<vector_size/2>(input_1);
    aie::vector<read_type, vector_size/2> size_vec2 = readincr_v<vector_size/2>(input_2);

    if(aie::equal(size_vec1, size_vec2)){
        printf("\\n\\nThe two images have the same size\\n\\n");
    }
    else{
        printf("\\n\\nWarning: The number of pixels of the two images is different, it has not yet been implemented...\\n");
        return;
    }
    // first of all, we reconstruct the size of the images
    unsigned int size1 = 0;
    unsigned int multiplier = 1;
    int first_zero = 0;
    for (int i = vector_size/2 - 1; i >= 0; i--){
        if (size_vec1[i] != 0){
            first_zero = i;
            break;
        }
    }
    for (int i = 0; i <= first_zero; i++){
        size1 += multiplier * size_vec1[i];
        multiplier *= 10;
    }

    if (size1 % vector_size != 0) 
        printf("\\n\\nWarning: The number of pixel is not divisible by the read size, the image will be truncated...\\n");

    // divide the computations in partial components
    unsigned long long int partial_num[num_partitions] = {0};
    unsigned long long int partial_denom_1[num_partitions] = {0};
    unsigned long long int partial_denom_2[num_partitions] = {0};

    aie::vector<read_type, vector_size> vec_1;
    aie::vector<read_type, vector_size> vec_2;
    // Process each vector with partial accumulators
    int loop_count = size1 / (vector_size * kernel_count);
    // in this loop, each iteration does not deppend on the 20 itarations before
    for (int i = 0; i < loop_count; i++) 
    chess_loop_range(4, )
    chess_prepare_for_pipelining
    {
        vec_1 = readincr_v<vector_size>(input_1);
        vec_2 = readincr_v<vector_size>(input_2);
        partial_num[i % num_partitions] += aie::reduce_add(aie::mul(vec_1, vec_2).to_vector<func_type>());
        partial_denom_1[i % num_partitions] += aie::reduce_add(aie::mul(vec_1, vec_1).to_vector<func_type>());
        partial_denom_2[i % num_partitions] += aie::reduce_add(aie::mul(vec_2, vec_2).to_vector<func_type>());
    }
    // now we put all the partial results togheter, so that we have the final result
    unsigned long long int num = 0; 
    unsigned long long int denom_1 = 0; 
    unsigned long long int denom_2 = 0; 
    for (int i = 0; i < num_partitions; i++) 
    chess_loop_range(num_partitions, )
    {
        num += partial_num[i];
        denom_1 += partial_denom_1[i];
        denom_2 += partial_denom_2[i];
    }
    //float res = (float) (num * num) / (denom_1 * denom_2);
    // pass the this result to the final kernel
    int high, low;

    aie::vector<int, 8> cc;
    split(num, high, low);
    cc.set(high, 0);
    cc.set(low, 1);

    split(denom_1, high, low);
    cc.set(high, 2);
    cc.set(low, 3);

    split(denom_2, high, low);
    cc.set(high, 4);
    cc.set(low, 5);

    writeincr(output, cc);
}
'''

    # sum_kernels function with dynamic handling of kernel_count
    cpp_code += '''
void sum_kernels ('''
    for i in range(1, kernel_count + 1):
        cpp_code += 'input_stream<int>* restrict kernel_{}, '.format(i)
    cpp_code += 'output_stream<write_type>* restrict output){\n'
    
    for i in range(1, kernel_count + 1):
        cpp_code += '    aie::vector<int, 8> k{0} = readincr_v<8>(kernel_{0});\n'.format(i)
    
    num = [f"recompose(k{i + 1}[0], k{i + 1}[1])" for i in range(kernel_count)]
    num = " + ".join(num)

    denom_1 = [f"recompose(k{i + 1}[2], k{i + 1}[3])" for i in range(kernel_count)]
    denom_1 = " + ".join(denom_1)

    denom_2 = [f"recompose(k{i + 1}[4], k{i + 1}[5])" for i in range(kernel_count)]
    denom_2 = " + ".join(denom_2)

    cpp_code += f'''
    // put all the partial metrics together, and obtain the real metric
    unsigned long long int num = {num};
    unsigned long long int denom_1 = {denom_1};
    unsigned long long int denom_2 = {denom_2};

    float res = (float) (num * num) / (denom_1 * denom_2);
    float coeff = (float) 2 * ((int) num >= 0) - 1;

    aie::vector<float, 4> cc;
    cc.set(res , 0);
    cc.set(coeff, 1);
    writeincr(output, cc);
'''
    return cpp_code + "\n}"


def create_kernel_header(kernel_count, metric):
    if metric not in ["mse", "cc", "psnr", "scc", "rmse"]:
        print("The metric you chose is not valid: " + metric)
    if metric in ["mse", "psnr", "rmse"]:
        return create_header_mse_psnr(kernel_count)
    else:
        return create_header_cc(kernel_count)

def create_header_mse_psnr(kernel_count):
    # Define the initial part of the header
    header = """#pragma once
#include <adf.h>

#define stream_type float

#define read_type uint8 // type of the input stream
#define write_type float // type of the output stream

void my_kernel_function (input_stream<read_type>* restrict input_1, input_stream<read_type>* restrict input_2, output_stream<write_type>* restrict output);
"""

    # Start the sum_kernels function
    header += "\nvoid sum_kernels ("

    # Add input streams based on the number of kernels
    for i in range(1, kernel_count + 1):
        header += f"input_stream<float>* restrict kernel_{i}, "

    # Add the output stream and close the function declaration
    header += "output_stream<write_type>* restrict output);\n"

    return header

def create_header_cc(kernel_count):
    cpp_code = '''
#pragma once
#include <adf.h>

#define stream_type float

#define read_type uint8 // type of the input stream
#define write_type float // type of the output stream
#define kernel_count {kernel_count}
'''.format(kernel_count=kernel_count)
    
    # Function prototypes
    cpp_code += '''
void my_kernel_function (input_stream<read_type>* restrict input_1, input_stream<read_type>* restrict input_2, output_stream<int>* restrict output);

void sum_kernels ('''
    for i in range(1, kernel_count + 1):
        cpp_code += 'input_stream<int>* restrict kernel_{}, '.format(i)
    cpp_code += 'output_stream<write_type>* restrict output);\n'

    return cpp_code

