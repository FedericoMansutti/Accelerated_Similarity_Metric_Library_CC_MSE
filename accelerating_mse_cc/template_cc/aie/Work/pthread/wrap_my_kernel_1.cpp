#include "adf.h"
#include "../../src/my_kernel_1.cpp"
void b0_kernel_wrapper(x86sim::stream_internal * arg0, x86sim::stream_internal * arg1, x86sim::stream_internal * arg2)
{
  auto _arg0 = (input_stream_uint8 *)(arg0);
  auto _arg1 = (input_stream_uint8 *)(arg1);
  auto _arg2 = (output_stream_int32 *)(arg2);
  return my_kernel_function(_arg0, _arg1, _arg2);
}
void b1_kernel_wrapper(x86sim::stream_internal * arg0, x86sim::stream_internal * arg1, x86sim::stream_internal * arg2)
{
  auto _arg0 = (input_stream_int32 *)(arg0);
  auto _arg1 = (input_stream_int32 *)(arg1);
  auto _arg2 = (output_stream_float *)(arg2);
  return sum_kernels(_arg0, _arg1, _arg2);
}
