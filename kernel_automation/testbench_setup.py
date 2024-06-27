def create_testbench_setup(kernel_count):
    file_count = kernel_count * 2
    stream_count = file_count

    cpp_code = f"""
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ap_axi_sdata.h>
#include <cmath>
#include "../setup_aie.cpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "../common/common.h"

#define max_pixel_value 256
#define write_type uint8_t

int main(int argc, char* argv[]) {{
    srand(time(0));

"""
    # Generate hls::stream definitions
    for i in range(1, stream_count + 1):
        cpp_code += f"    hls::stream<stream_type> s_{i};\n"

    cpp_code += """

    int size = 512;
    int depth = 1;

    size *= depth;
    int loop_size = size/read_size;

    stream_type input_1[loop_size];
    stream_type input_2[loop_size];

    stream_type ap;

    for (int i = 0; i < loop_size; i++) {
            input_1[i].range(7, 0) = (write_type) rand() % max_pixel_value;
            input_1[i].range(15, 8) = (write_type) rand() % max_pixel_value;
            input_1[i].range(23, 16) = (write_type) rand() % max_pixel_value;
            input_1[i].range(31, 24) = (write_type) rand() % max_pixel_value;

            input_1[i].range(39, 32) = (write_type) rand() % max_pixel_value;
            input_1[i].range(47, 40) = (write_type) rand() % max_pixel_value;
            input_1[i].range(55, 48) = (write_type) rand() % max_pixel_value;
            input_1[i].range(63, 56) = (write_type) rand() % max_pixel_value;

            input_1[i].range(71, 64) = (write_type) rand() % max_pixel_value;
            input_1[i].range(79, 72) = (write_type) rand() % max_pixel_value;
            input_1[i].range(87, 80) = (write_type) rand() % max_pixel_value;
            input_1[i].range(95, 88) = (write_type) rand() % max_pixel_value;

            input_1[i].range(103, 96) = (write_type) rand() % max_pixel_value;
            input_1[i].range(111, 104) = (write_type) rand() % max_pixel_value;
            input_1[i].range(119, 112) = (write_type) rand() % max_pixel_value;
            input_1[i].range(127, 120) = (write_type) rand() % max_pixel_value;
    }
    for (int i = 0; i < loop_size; i++) {
            input_2[i].range(7, 0) = (write_type) rand() % max_pixel_value;
            input_2[i].range(15, 8) = (write_type) rand() % max_pixel_value;
            input_2[i].range(23, 16) = (write_type) rand() % max_pixel_value;
            input_2[i].range(31, 24) = (write_type) rand() % max_pixel_value;

            input_2[i].range(39, 32) = (write_type) rand() % max_pixel_value;
            input_2[i].range(47, 40) = (write_type) rand() % max_pixel_value;
            input_2[i].range(55, 48) = (write_type) rand() % max_pixel_value;
            input_2[i].range(63, 56) = (write_type) rand() % max_pixel_value;

            input_2[i].range(71, 64) = (write_type) rand() % max_pixel_value;
            input_2[i].range(79, 72) = (write_type) rand() % max_pixel_value;
            input_2[i].range(87, 80) = (write_type) rand() % max_pixel_value;
            input_2[i].range(95, 88) = (write_type) rand() % max_pixel_value;

            input_2[i].range(103, 96) = (write_type) rand() % max_pixel_value;
            input_2[i].range(111, 104) = (write_type) rand() % max_pixel_value;
            input_2[i].range(119, 112) = (write_type) rand() % max_pixel_value;
            input_2[i].range(127, 120) = (write_type) rand() % max_pixel_value;
    }

    """

    # Setup AIE function call with the correct number of streams
    cpp_code += f"setup_aie(size, size, input_1, input_2, "
    cpp_code += ", ".join([f"s_{i}" for i in range(1, stream_count + 1)])
    cpp_code += ");\n"

    cpp_code += """    std::cout << "\\n\\nstart\\n\\n";

"""

    # Generate ofstream definitions and open statements
    for i in range(1, file_count + 1):
        cpp_code += f"    std::ofstream file_{i};\n"
        cpp_code += f"    file_{i}.open(\"../../aie/data/in_plio_source_{i}.txt\");\n"

    cpp_code += """
    if ("""
    cpp_code += " && ".join([f"file_{i}.is_open()" for i in range(1, file_count + 1)])
    cpp_code += """) {
        for (int i = 0; i < loop_size/kernel_count + 1; i++){
"""
    # Generate read and write operations for each stream
    for i in range(1, stream_count + 1):
        cpp_code += f"""
            ap = (stream_type) s_{i}.read();
            file_{i} << (int) ap.range(7, 0) << " ";
            file_{i} << (int) ap.range(15, 8) << " ";
            file_{i} << (int) ap.range(23, 16) << " ";
            file_{i} << (int) ap.range(31, 24) << " ";
            file_{i} << (int) ap.range(39, 32) << " ";
            file_{i} << (int) ap.range(47, 40) << " ";
            file_{i} << (int) ap.range(55, 48) << " ";
            file_{i} << (int) ap.range(63, 56) << " ";
            file_{i} << (int) ap.range(71, 64) << " ";
            file_{i} << (int) ap.range(79, 72) << " ";
            file_{i} << (int) ap.range(87, 80) << " ";
            file_{i} << (int) ap.range(95, 88) << " ";
            file_{i} << (int) ap.range(103, 96) << " ";
            file_{i} << (int) ap.range(111, 104) << " ";
            file_{i} << (int) ap.range(119, 112) << " ";
            file_{i} << (int) ap.range(127, 120) << " ";
            file_{i} << std::endl;
        """

    cpp_code += """
        }
        std::cout << "\\n\\n end \\n\\n";
"""
    # Generate file close statements
    for i in range(1, file_count + 1):
        cpp_code += f"        file_{i}.close();\n"

    cpp_code += """
    } else {
        std::cout << "Error opening file" << std::endl;
    }
    return 0;
}
"""
    return cpp_code