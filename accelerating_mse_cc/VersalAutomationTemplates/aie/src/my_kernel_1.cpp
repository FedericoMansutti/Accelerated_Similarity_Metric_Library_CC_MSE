#include "my_kernel_1.h"
#include "common.h"
#include "aie_api/aie.hpp"
#include "aie_api/aie_adf.hpp"
#include "aie_api/utils.hpp"

//API REFERENCE for STREAM: 
// https://docs.amd.com/r/ehttps://docs.amd.com/r/en-US/ug1079-ai-engine-kernel-coding/Reading-and-Advancing-an-Input-Streamn-US/ug1079-ai-engine-kernel-coding/Reading-and-Advancing-an-Input-Stream

void my_kernel_function (input_stream<uint8_t>* restrict input1, input_stream<uint8_t>* restrict input2, output_stream<uint8_t>* restrict output)
{
    // read from one stream and write to another
    // read 4 vector uint8_t from input stream and write to output stream
    aie::vector<uint8_t,16> x = readincr_v<16>(input1); // 4 uint8_t = 32 bit -> 32-bit wide stream operation.

    int size = 0;
    int shifted = 0;
    for (int i = 0; i < 4; i++)
    {

        printf("x[%d]: %d\n", i, (int)x[i]);
        shifted = (int)x[i];
        size|=shifted<<(8*i);
    }

    printf("tot_num: %d\n", size);
    float sum = 0;

    for (int i = 0; i < size; i++)
    {
        aie::vector<uint8_t,16> x1 = readincr_v<16>(input1);
        aie::vector<uint8_t,16> x2 = readincr_v<16>(input2); // 1 Float = 32 bit. 32 x 4 = 128 bit -> 128-bit wide stream operation.
        
        writeincr(output,x1);
        writeincr(output,x2);
    }   
}