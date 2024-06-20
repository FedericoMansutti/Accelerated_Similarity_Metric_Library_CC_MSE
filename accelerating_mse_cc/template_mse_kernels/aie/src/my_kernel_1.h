#pragma once
#include <adf.h>

#define stream_type float

#define read_type uint8 // type of the input stream
#define write_type float // type of the output stream

void my_kernel_function (input_stream<read_type>* restrict input_1, input_stream<read_type>* restrict input_2, output_stream<write_type>* restrict output);

void sum_kernels (input_stream<float>* restrict kernel_1, input_stream<float>* restrict kernel_2, output_stream<write_type>* restrict output);