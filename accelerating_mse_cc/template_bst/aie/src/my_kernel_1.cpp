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

inline aie::vector<func_type,vector_size> convert_to_func_type(aie::vector<read_type,vector_size> vec) {
    aie::accum<acc32, vector_size> acc;
    acc.from_vector(vec, 0);
    return acc.to_vector<func_type>();
}

//API REFERENCE for STREAM: 
// https://docs.amd.com/r/ehttps://docs.amd.com/r/en-US/ug1079-ai-engine-kernel-coding/Reading-and-Advancing-an-Input-Streamn-US/ug1079-ai-engine-kernel-coding/Reading-and-Advancing-an-Input-Stream

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

    aie::vector<func_type, vector_size> partial_num[num_partitions]; // init partial sums which contains the "map" phase of map-reduce pattern
    for (int i = 0; i < num_partitions; i++)
    chess_loop_range(num_partitions, )
    {
        partial_num[i] = aie::zeros<func_type, vector_size>(); // all the elements must be zeros, at the start
    }
    aie::vector<func_type, vector_size> partial_denom[num_partitions]; // init partial sums which contains the "map" phase of map-reduce pattern
    for (int i = 0; i < num_partitions; i++)
    chess_loop_range(num_partitions, )
    {
        partial_denom[i] = aie::zeros<func_type, vector_size>(); // all the elements must be zeros, at the start
    }

    int mean_1 = static_cast<func_type>(readincr_v<vector_size>(input_1)[0]);

    aie::vector<func_type, vector_size> vec_1;
    aie::vector<func_type, vector_size> vec_2;
    aie::vector<func_type, vector_size> vec_3;
    int index;
    for(int i = 0; i < size1 / vector_size; i++)
    chess_loop_range(4, )
    chess_prepare_for_pipelining
    {
        index = i % num_partitions;
        vec_1 = aie::add(convert_to_func_type(readincr_v<vector_size>(input_1)), -mean_1);
        vec_2 = convert_to_func_type(readincr_v<vector_size>(input_2));
        vec_3 = aie::mul(vec_1, vec_1).to_vector<func_type>();

        partial_num[index] = aie::add(partial_num[index], aie::mul(vec_2, vec_3).to_vector<func_type>());
        partial_denom[index] = aie::add(partial_denom[index], vec_3);
    }

    unsigned long long int final_num = 0;
    unsigned long long int final_denom = 0;
    for (int i = 0; i < num_partitions; i++) 
    chess_loop_range(num_partitions, )
    {
        final_num += aie::reduce_add(partial_num[i]); // "reduce" phase of map-reduce, combining all the partial sums
        final_denom += aie::reduce_add(partial_denom[i]);
    }
    write_type res = (write_type) (final_num) / (final_denom);
    aie::vector<write_type, 4> bst;
    bst.set(res, 0);
    writeincr(output, bst);
}