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
#define kernel_count 2

//API REFERENCE for STREAM: 
// https://docs.amd.com/r/ehttps://docs.amd.com/r/en-US/ug1079-ai-engine-kernel-coding/Reading-and-Advancing-an-Input-Streamn-US/ug1079-ai-engine-kernel-coding/Reading-and-Advancing-an-Input-Stream

void my_kernel_function (input_stream<read_type>* restrict input_1, input_stream<read_type>* restrict input_2, output_stream<write_type>* restrict output) {
    aie::vector<read_type, vector_size/2> size_vec1 = readincr_v<vector_size/2>(input_1);
    aie::vector<read_type, vector_size/2> size_vec2 = readincr_v<vector_size/2>(input_2);

    if(aie::equal(size_vec1, size_vec2)){
        printf("\n\nThe two images have the same size\n\n");
    }
    else{
        printf("\n\nWarning: The number of pixels of the two images is different, it has not yet been implemented...\n");
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
        printf("\n\nWarning: The number of pixel is not divisible by the read size, the image will be truncated...\n");

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
    float res = (float) (num * num) / (denom_1 * denom_2);
    // pass the this result to the final kernel
    aie::vector<write_type, 4> cc;
    printf("\nkernel cc: %f\n", res);
    cc.set(res, 0);
    writeincr(output, cc);
}

void sum_kernels (input_stream<write_type>* restrict kernel_1, input_stream<write_type>* restrict kernel_2, output_stream<float>* restrict output){
    float k1 = readincr_v<4>(kernel_1)[0];
    float k2 = readincr_v<4>(kernel_2)[0];
    // put all the partial metrics togheter, and obtain the real metric
    float res = (k1 + k2) / 2;

    aie::vector<float, 4> cc;
    cc.set(res , 0);
    writeincr(output, cc);
}