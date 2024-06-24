import shutil
import os
import argparse
from kernel_1_automation import create_kernel, create_kernel_header
from graph_automation import create_graph
from setup_aie_automation import setup_aie
from testbench_sink_from_aie import test_sink_from_aie
from testbench_setup import create_testbench_setup

def copy_directory(src, dst):
    """
    Copies the directory from src to dst.
    """
    if os.path.exists(dst):
        shutil.rmtree(dst)
    shutil.copytree(src, dst)

def overwrite_files(dst, files_to_overwrite):
    """
    Overwrites the specified files in the dst directory.
    """
    for relative_path, new_content in files_to_overwrite.items():
        full_path = os.path.normpath(os.path.join(dst, relative_path))
        os.makedirs(os.path.dirname(full_path), exist_ok=True)
        with open(full_path, 'w') as file:
            file.write(new_content)

def main():
    parser = argparse.ArgumentParser(description="Copy a folder and overwrite specific files.")
    parser.add_argument("src_directory", help="The source directory to copy.")
    parser.add_argument("dst_directory", help="The destination directory.")
    parser.add_argument("kernel_count", help="The number of kernels.")
    args = parser.parse_args()

    kernel_count = int(args.kernel_count)
    # Example: Dictionary of files to overwrite with their new content
    files_to_overwrite = {
        'aie/src/graph.h': create_graph(kernel_count),
        'aie/src/my_kernel_1.cpp': create_kernel(kernel_count),
        'aie/src/my_kernel_1.h': create_kernel_header(kernel_count),
        'data_movers/testbench/testbench_setupaie.cpp': create_testbench_setup(kernel_count),
        'data_movers/setup_aie.cpp': setup_aie(kernel_count),
        'data_movers/testbench/testbench_sink_from_aie.cpp': test_sink_from_aie(kernel_count),
        # Add more files as needed
    }

    # Step 1: Copy the entire directory
    copy_directory(args.src_directory, args.dst_directory)

    # Step 2: Overwrite the content of specific files
    overwrite_files(args.dst_directory, files_to_overwrite)

if __name__ == "__main__":
    main()
