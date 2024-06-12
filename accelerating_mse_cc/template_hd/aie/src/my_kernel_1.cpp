#include "my_kernel_1.h"
#include "common.h"
#include "aie_api/aie.hpp"
#include "aie_api/aie_adf.hpp"
#include "aie_api/utils.hpp"

#define vector_size 16 // Size of the vector unit, used for reading from input stream
#define num_partitions 20 // Number of partitions for partial accumulation

#define read_type uint8 // type of the input stream
#define func_type int // type used for doing the computations
#define write_type float // type of the output stream

//API REFERENCE for STREAM: 
// https://docs.amd.com/r/ehttps://docs.amd.com/r/en-US/ug1079-ai-engine-kernel-coding/Reading-and-Advancing-an-Input-Streamn-US/ug1079-ai-engine-kernel-coding/Reading-and-Advancing-an-Input-Stream

inline aie::vector<func_type,vector_size> convert_to_func_type(aie::vector<read_type,vector_size> vec) {
    aie::accum<acc32, vector_size> acc;
    acc.from_vector(vec, 0);
    return acc.to_vector<func_type>();
}

void my_kernel_function (input_stream<read_type>* restrict input_1, input_stream<read_type>* restrict input_2, output_stream<write_type>* restrict output) {
    aie::vector<read_type, vector_size> size_vec1 = readincr_v<vector_size>(input_1);
    aie::vector<read_type, vector_size> size_vec2 = readincr_v<vector_size>(input_2);
    
    if(aie::equal(size_vec1, size_vec2)){
        printf("\n\nThe two images have the same size\n\n");
    }
    else{
        printf("\n\nWarning: The number of pixels of the two images is different, it has not yet been implemented...\n");
        return;
    }
    
    unsigned int size1 = 0;
    unsigned int multiplier = 1;
    int first_zero = 0;
    for (int i = vector_size - 1; i >= 0; i--){
        if (size_vec1[i] != 0){
            first_zero = i;
            break;
        }
    }
    for (int i = 0; i <= first_zero; i++){
        size1 += multiplier * size_vec1[i];
        multiplier *= 10;
    }

    if (size1 % vector_size != 0) printf("\n\nWarning: The number of pixel is not divisible by 16, the image will be truncated...\n");

    unsigned long long int partial_sums[num_partitions] = {0}; // init partial sums which contains the "map" phase of map-reduce pattern


    // Process each vector with partial accumulators
    for (int i = 0; i < size1 / vector_size; i++) 
    chess_loop_range(4, )
    chess_prepare_for_pipelining
    {
        partial_sums[i % num_partitions] += aie::reduce_add(aie::abs(aie::add(convert_to_func_type(readincr_v<vector_size>(input_1)), aie::neg(convert_to_func_type(readincr_v<vector_size>(input_2))))));
    }
    unsigned long long int hd = 0; // final sum for the MSE
    for (int i = 0; i < num_partitions; i++) 
    chess_loop_range(num_partitions, )
    {
        hd += partial_sums[i]; // "reduce" phase of map-reduce, combining all the partial sums
    }
    writeincr(output, (write_type) hd / size1); // divide and write to get the real hamming distance ratio
}