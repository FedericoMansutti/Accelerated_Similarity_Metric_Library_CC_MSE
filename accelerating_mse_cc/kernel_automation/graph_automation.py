def create_graph(kernel_count):
    num_inputs = 2 * kernel_count  # Number of input_plio objects

    header_content = '''#pragma once
#include <adf.h>
#include "my_kernel_1.h"

using namespace adf;

class my_graph: public graph
{

private:
\t// ------kernel declaration------

'''
    kernel_declarations = ''.join([f'\tkernel my_kernel_{j};\n' for j in range(1, kernel_count + 1)])
    kernel_declarations += '''
\tkernel my_kernel_sum;
public:
\t// ------Input and Output PLIO declaration------
'''

    # Generate input_plio declarations
    input_declarations = ''.join([f'\tinput_plio in_{i};\n' for i in range(1, num_inputs + 1)])

    constructor_header = '''
\toutput_plio out_1;
\tmy_graph()
\t{
\t\t// ------kernel creation------
'''

    kernel_initializations = ''.join(
        [f'\t\tmy_kernel_{i} = kernel::create(my_kernel_function);\n' for i in range(1, kernel_count + 1)])

    input_creation = '''
\t\tmy_kernel_sum = kernel::create(sum_kernels);
\t\t// ------Input and Output PLIO creation------
\t\t// I argument: a name, that will be used to refer to the port in the block design
\t\t// II argument: the type of the PLIO that will be read/written. Test both plio_32_bits and plio_128_bits to verify the difference
\t\t// III argument: the path to the file that will be read/written for simulation
'''

    # Generate input_plio creation statements
    input_creation += ''.join(
        [f'\t\tin_{i} = input_plio::create("in_plio_{i}", plio_128_bits, "data/in_plio_source_{i}.txt");\n' for i in
         range(1, num_inputs + 1)])
    # Generate output_plio creation statements
    input_creation += ''.join(
        [f'\t\tout_1 = output_plio::create("out_plio_1", plio_32_bits, "data/out_plio_sink_1.txt");\n'])

    kernel_connections = '''
\t\t// ------kernel connection------
\t\t// it is possible to have stream or window. This is just an example. Try both to see the difference
'''
    kernel_connections_statements = ''
    # Generate kernel connection statements
    for j in range(1, kernel_count + 1):
        kernel_connections_statements += ''.join(
            [f'\t\tconnect<stream>(in_{2 * j - 1}.out[0], my_kernel_{j}.in[0]);\n'])
        kernel_connections_statements += ''.join([f'\t\tconnect<stream>(in_{2 * j}.out[0], my_kernel_{j}.in[1]);\n'])
    for j in range(1, kernel_count + 1):
        kernel_connections_statements += ''.join(
            [f'\t\tconnect<stream>(my_kernel_{j}.out[0], my_kernel_sum.in[{j - 1}]);\n'])

    kernel_connections_statements += ''.join([f'\t\tconnect<stream>(my_kernel_sum.out[0], out_1.in[0]);\n'])

    kernel_config = '''
\t\t// set kernel source and headers
'''
    for j in range(1, kernel_count + 1):
        kernel_config += ''.join([f'\t\tsource(my_kernel_{j})  = "src/my_kernel_{j}.cpp";\n'])
        kernel_config += ''.join([f'\t\theaders(my_kernel_{j}) = {{"src/my_kernel_{j}.h","../common/common.h"}}\n'])
    kernel_config += '''
\t\tsource(my_kernel_sum)  = "src/my_kernel_1.cpp";
\t\theaders(my_kernel_sum) = {"src/my_kernel_1.h","../common/common.h"};

\t\t// set ratio
\t\t// 90% of the time the kernel will be executed. This means that 1 AIE will be able to execute just 1 Kernel
'''
    for j in range(1, kernel_count + 1):
        kernel_config += '\n'.join([f'\t\truntime<ratio>(my_kernel_{j}) = 0.9;\n'])
    kernel_config += '''\t\truntime<ratio>(my_kernel_sum) = 0.9; 
\t};

};
'''

    # Combine all parts
    full_content = header_content + kernel_declarations + input_declarations + constructor_header + kernel_initializations + input_creation + kernel_connections + kernel_connections_statements + kernel_config

    return full_content
