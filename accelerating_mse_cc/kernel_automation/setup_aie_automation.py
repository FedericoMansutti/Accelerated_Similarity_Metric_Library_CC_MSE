def setup_aie(kernel_count):
    # Original code parts
    includes = '''#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include "hls_burst_maxi.h"

#define vector_type int
#define pixel_type unsigned char
#define stream_type ap_uint<128>
#define read_size 16
#define burst_length 64
#define kernel_count ''' + str(kernel_count) + '''

extern "C" {
'''
    
    # Function definition
    func_def = '''void setup_aie(int size1, int size2, hls::burst_maxi<ap_uint<128>> input_1, hls::burst_maxi<ap_uint<128>> input_2, '''
    
    # Generate stream declarations
    stream_declarations = []
    for i in range(1, kernel_count * 2 + 1):
        stream_declarations.append(f'hls::stream<stream_type>& s_{i}')
    
    func_def += ', '.join(stream_declarations)
    func_def += ') {\n'

    # Interface pragmas
    interface_pragmas = '''\t#pragma HLS interface m_axi port=input_1 offset=slave bundle=gmem0 max_read_burst_length=256
\t#pragma HLS interface axis port=s_1
\t#pragma HLS interface s_axilite port=input_1 bundle=control

\t#pragma HLS interface m_axi port=input_2 offset=slave bundle=gmem0 max_read_burst_length=256
\t#pragma HLS interface axis port=s_2
\t#pragma HLS interface s_axilite port=input_2 bundle=control

\t#pragma HLS interface s_axilite port=size1 bundle=control
\t#pragma HLS interface s_axilite port=size2 bundle=control
\t#pragma HLS interface s_axilite port=return bundle=control
'''

    # Size variables
    size_vars = '''\t// size1 represents the pixels in image 1 while size2 represents the pixels in image2
\tint size1_copy = size1;
\tint size2_copy = size2;

\tstream_type ap_1;

\tint size_1_vec[read_size] = {0};

\t// repeat until size 1 has finished writing
\tint index = 0;
\twhile (size1_copy != 0){
\t\tsize_1_vec[index++] = size1_copy % 10;
\t\tsize1_copy /= 10;
\t}

\tap_1.range(7, 0) = (pixel_type) size_1_vec[0];
\tap_1.range(15, 8) = (pixel_type) size_1_vec[1];
\tap_1.range(23, 16) = (pixel_type) size_1_vec[2];
\tap_1.range(31, 24) = (pixel_type) size_1_vec[3];

\tap_1.range(39, 32) = (pixel_type) size_1_vec[4];
\tap_1.range(47, 40) = (pixel_type) size_1_vec[5];
\tap_1.range(55, 48) = (pixel_type) size_1_vec[6];
\tap_1.range(63, 56) = (pixel_type) size_1_vec[7];

\tap_1.range(71, 64) = (pixel_type) size_1_vec[8];
\tap_1.range(79, 72) = (pixel_type) size_1_vec[9];
\tap_1.range(87, 80) = (pixel_type) size_1_vec[10];
\tap_1.range(95, 88) = (pixel_type) size_1_vec[11];

\tap_1.range(103, 96) = (pixel_type) size_1_vec[12];
\tap_1.range(111, 104) = (pixel_type) size_1_vec[13];
\tap_1.range(119, 112) = (pixel_type) size_1_vec[14];
\tap_1.range(127, 120) = (pixel_type) size_1_vec[15];
'''

    # Stream writes
    stream_writes = []
    for i in range(1, kernel_count * 2 + 1):
        stream_writes.append(f'\ts_{i}.write((stream_type) ap_1);')
    stream_writes = '\n'.join(stream_writes) + "\n"

    # Buffer declarations
    buffer_declarations = []
    for i in range(1, kernel_count * 2 + 1):
        buffer_declarations.append(f'\tstream_type buf_{i}[burst_length/kernel_count];')
    buffer_declarations = '\n'.join(buffer_declarations) + "\n"

    # Loop variables
    loop_vars = '''\tint ap_count = size1 / read_size;
\tint burst_count = ap_count / (burst_length);
\tint remainder = ap_count % burst_length;
\tint start = 0;
'''

    # Burst loop
    burst_loop = '''\tfor (int i = 0; i < burst_count; i++){
\t\t#pragma HLS PIPELINE II=1

\t\tstart = i * burst_length;
\t\t// read burst!
\t\tinput_1.read_request(start, burst_length);
\t\tinput_2.read_request(start, burst_length);
\t\tfor (int j = 0; j < burst_length/kernel_count; j++){
\t\t\t#pragma HLS PIPELINE UNROLL
'''

    # Read buffers
    read_buffers = []
    for j in range(0, kernel_count * 2, 2):
        read_buffers.append(f'\t\t\tbuf_{j+1}[i] = input_1.read();')
        read_buffers.append(f'\t\t\tbuf_{j+2}[i] = input_2.read();')
    read_buffers = '\n'.join(read_buffers)

    # Write streams
    write_streams = []
    for j in range(1, kernel_count * 2 + 1):
        write_streams.append(f'\t\t\ts_{j}.write((stream_type) buf_{j}[i]);')
    write_streams = '\n'.join(write_streams)

    burst_loop += f'{read_buffers}\n\t\t}}\n\t\tfor (int j = 0; j < burst_length/kernel_count; j++){{\n{write_streams}\n\t\t}}\n\t}}\n'

    # Remainder processing
    remainder_processing = '''\tif (remainder == 0) return;
\tstart = burst_length * burst_count;
\tinput_1.read_request(start, remainder);
\tinput_2.read_request(start, remainder);
\tfor (int i = 0; i < remainder/kernel_count; i++){
\t\t#pragma HLS PIPELINE UNROLL
'''
    remainder_read = []
    for j in range(0, kernel_count * 2, 2):
        remainder_read.append(f'\t\tbuf_{j+1}[i] = input_1.read();')
        remainder_read.append(f'\t\tbuf_{j+2}[i] = input_2.read();')
    remainder_read = '\n'.join(remainder_read) + "\n\t}"
    
    remainder_write = []
    for j in range(1, kernel_count * 2 + 1):
        remainder_write.append(f'\t\ts_{j}.write((stream_type) buf_{j}[i]);')
    remainder_write = '\n'.join(remainder_write)
    
    remainder_processing += f'{remainder_read}\n\tfor (int i = 0; i < remainder/kernel_count; i++){{\n{remainder_write}\n\t}}\n}}\n}}\n'
    
    # Combine all parts
    cpp_code = includes + func_def + interface_pragmas + size_vars + stream_writes + buffer_declarations + loop_vars + burst_loop + remainder_processing
    
    return cpp_code
