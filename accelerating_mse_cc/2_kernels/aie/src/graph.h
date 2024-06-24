#pragma once
#include <adf.h>
#include "my_kernel_1.h"

using namespace adf;

class my_graph: public graph
{

private:
	// ------kernel declaration------

	kernel my_kernel_1;
	kernel my_kernel_2;

	kernel my_kernel_sum;
public:
	// ------Input and Output PLIO declaration------
	input_plio in_1;
	input_plio in_2;
	input_plio in_3;
	input_plio in_4;

	output_plio out_1;
	my_graph()
	{
		// ------kernel creation------
		my_kernel_1 = kernel::create(my_kernel_function);
		my_kernel_2 = kernel::create(my_kernel_function);

		my_kernel_sum = kernel::create(sum_kernels);
		// ------Input and Output PLIO creation------
		// I argument: a name, that will be used to refer to the port in the block design
		// II argument: the type of the PLIO that will be read/written. Test both plio_32_bits and plio_128_bits to verify the difference
		// III argument: the path to the file that will be read/written for simulation
		in_1 = input_plio::create("in_plio_1", plio_128_bits, "data/in_plio_source_1.txt");
		in_2 = input_plio::create("in_plio_2", plio_128_bits, "data/in_plio_source_2.txt");
		in_3 = input_plio::create("in_plio_3", plio_128_bits, "data/in_plio_source_3.txt");
		in_4 = input_plio::create("in_plio_4", plio_128_bits, "data/in_plio_source_4.txt");
		out_1 = output_plio::create("out_plio_1", plio_32_bits, "data/out_plio_sink_1.txt");

		// ------kernel connection------
		// it is possible to have stream or window. This is just an example. Try both to see the difference
		connect<stream>(in_1.out[0], my_kernel_1.in[0]);
		connect<stream>(in_2.out[0], my_kernel_1.in[1]);
		connect<stream>(in_3.out[0], my_kernel_2.in[0]);
		connect<stream>(in_4.out[0], my_kernel_2.in[1]);
		connect<stream>(my_kernel_1.out[0], my_kernel_sum.in[0]);
		connect<stream>(my_kernel_2.out[0], my_kernel_sum.in[1]);
		connect<stream>(my_kernel_sum.out[0], out_1.in[0]);

		// set kernel source and headers
		source(my_kernel_1)  = "src/my_kernel_1.cpp";
		headers(my_kernel_1) = {"src/my_kernel_1.h","../common/common.h"}
		source(my_kernel_2)  = "src/my_kernel_2.cpp";
		headers(my_kernel_2) = {"src/my_kernel_2.h","../common/common.h"}

		source(my_kernel_sum)  = "src/my_kernel_1.cpp";
		headers(my_kernel_sum) = {"src/my_kernel_1.h","../common/common.h"};

		// set ratio
		// 90% of the time the kernel will be executed. This means that 1 AIE will be able to execute just 1 Kernel
		runtime<ratio>(my_kernel_1) = 0.9;
		runtime<ratio>(my_kernel_2) = 0.9;
		runtime<ratio>(my_kernel_sum) = 0.9; 
	};

};
