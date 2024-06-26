import shutil
from time import sleep
import os
import argparse
from kernel_automation import create_kernel, create_kernel_header
from graph_automation import create_graph
from setup_aie_automation import setup_aie
from testbench_sink_from_aie import test_sink_from_aie
from testbench_setup import create_testbench_setup
from cfg_automation import build_cfg

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
    parser.add_argument("metric", help="The metric that you want to use")
    parser.add_argument("kernel_count", help="The number of kernels.")
    args = parser.parse_args()

    kernel_count = int(args.kernel_count)
    metric = args.metric
    # Example: Dictionary of files to overwrite with their new content
    files_to_overwrite = {
        'aie/src/graph.h': create_graph(kernel_count),
        'aie/src/my_kernel_1.cpp': create_kernel(kernel_count, metric),
        'aie/src/my_kernel_1.h': create_kernel_header(kernel_count, metric),
        'data_movers/testbench/testbench_setupaie.cpp': create_testbench_setup(kernel_count),
        'data_movers/setup_aie.cpp': setup_aie(kernel_count),
        'data_movers/testbench/testbench_sink_from_aie.cpp': test_sink_from_aie(kernel_count, metric),
        'hw/xclbin_overlay.cfg': build_cfg(kernel_count),
        # Add more files as needed
    }

    # Step 1: Copy the entire directory
    copy_directory(f"template_{metric}", f"template_{metric}_{kernel_count}_kernels")

    # Step 2: Overwrite the content of specific files
    overwrite_files(f"template_{metric}_{kernel_count}_kernels", files_to_overwrite)

    sleep(0.25)
    # finally, remove the files that we don't want to be copied in the new template
    os.system(f"rm -rf template_{metric}_{kernel_count}_kernels/aie/data/*")
    os.system(f"rm -rf template_{metric}_{kernel_count}_kernels/hw/overlay_hw.xclbin")
    os.system(f"rm -rf template_{metric}_{kernel_count}_kernels/hw/overlay_hw_emu.xclbin")

if __name__ == "__main__":
    main()
