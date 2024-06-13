#include "my_kernel_1.h"
#include "common.h"
#include "aie_api/aie.hpp"
#include "aie_api/aie_adf.hpp"
#include "aie_api/utils.hpp"

//API REFERENCE for STREAM: 
// https://docs.amd.com/r/ehttps://docs.amd.com/r/en-US/ug1079-ai-engine-kernel-coding/Reading-and-Advancing-an-Input-Streamn-US/ug1079-ai-engine-kernel-coding/Reading-and-Advancing-an-Input-Stream
#define func_type int
#define vector_size 16
#define read_type uint8_t

inline aie::vector<func_type,vector_size> convert_to_func_type(aie::vector<read_type,vector_size> vec) {
    aie::accum<acc32, vector_size> acc;
    acc.from_vector(vec, 0);
    return acc.to_vector<func_type>();
}

void my_kernel_function (input_stream<uint8_t>* restrict input, output_stream<float>* restrict output)
{
    // read from one stream and write to another
    // read 4 vector uint8_t from input stream and write to output stream
    aie::vector<uint8_t,16> x = readincr_v<16>(input); // 4 uint8_t = 32 bit -> 32-bit wide stream operation.

    int tot_num = 0;
    int shifted = 0;
    for (int i = 0; i < 4; i++)
    {

        printf("x[%d]: %d\n", i, (int)x[i]);
        shifted = (int)x[i];
        tot_num|=shifted<<(8*i);
    }

    printf("tot_num: %d\n", tot_num);
    int sum = 0;
    int loop_count = tot_num/2;

    for (int i = 0; i < loop_count; i++)
    {
        aie::vector<func_type,16> x1 = convert_to_func_type(readincr_v<16>(input));
        aie::vector<func_type,16> x2 = convert_to_func_type(readincr_v<16>(input)); // 1 Float = 32 bit. 32 x 4 = 128 bit -> 128-bit wide stream operation.
        sum += aie::reduce_add(x1) + aie::reduce_add(x2);
    }
    writeincr(output,(float) sum);
}