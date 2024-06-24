import shutil
import os
import argparse
from kernel_1_automation import generate_kernel_code
from graph_automation import create_graph

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
        'aie/src/my_kernel_1.cpp': generate_kernel_code(kernel_count),
        # Add more files as needed
    }

    # Step 1: Copy the entire directory
    copy_directory(args.src_directory, args.dst_directory)

    # Step 2: Overwrite the content of specific files
    overwrite_files(args.dst_directory, files_to_overwrite)

if __name__ == "__main__":
    main()
