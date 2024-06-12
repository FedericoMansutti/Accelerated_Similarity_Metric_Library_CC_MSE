import os
import sys

if sys.argv[1] == "clean_all":
    os.system("cd template_mse && make clean && cd ../template_ncc && make clean && cd ../template_bst && make clean && cd ../template_hd && make clean && cd ../template_psnr && make clean && cd ../_template_original && make clean && cd ..")

if len(sys.argv) != 3:
    sys.exit("\n\nCommand not valid\n")

if sys.argv[1] == "run_testbench":
    os.system(f"cd template_{sys.argv[2]} && cd aie && make clean && make aie_compile_x86 && cd ../data_movers &&  make run_testbench_setupaie && cd ../aie && make aie_simulate_x86 && cd ../data_movers && make run_testbench_sink_from_aie && cd .. && cd ..")

elif sys.argv[1] == "simulate_x86":
    os.system(f"cd template_{sys.argv[2]} && cd aie && make clean && make aie_compile_x86 && make aie_simulate_x86 && cd .. && cd ..")

elif sys.argv[1] == "simulate_VLIW":
    os.system(f"cd template_{sys.argv[2]} && cd aie && make clean && make aie_compile && make aie_simulate && cd .. && cd ..")

elif sys.argv[1] == "complile_hw_emu":
    os.system(f"cd template_{sys.argv[2]} && make clean && make build_hw TARGET=hw_emu && cd ..")

elif sys.argv[1] == "complile_hw":
    os.system(f"cd template_{sys.argv[2]} && make clean && make build_hw TARGET=hw && cd ..")

elif sys.argv[1] == "run_hw_emu":
    os.system(f"cd template_{sys.argv[2]} && make build_sw && cd sw && chmod u+x ./setup_emu.sh && source ./setup_emu.sh -s on && ./setup_emu.sh -s on && ./host_overlay.exe && cd .. && cd ..")

elif sys.argv[1] == "run_hw":
    os.system(f"cd template_{sys.argv[2]} && make build_sw && cd sw && chmod u+x ./host_overlay.exe && ./host_overlay.exe && cd .. && cd ..")

elif sys.argv[1] == "clean":
    os.system(f"cd template_{sys.argv[2]} && make clean && cd ..")

elif sys.argv[1] == "source":
    os.system(f"source {sys.argv[2]}")

elif sys.argv[1] == "package_hw_emu":
    os.system(f"cd template_{sys.argv[2]} && make build_and_pack TARGET=hw_emu && cd ..")

elif sys.argv[1] == "package_hw":
    os.system(f"cd template_{sys.argv[2]} && make build_and_pack TARGET=hw && cd ..")

else:
    sys.exit("\n\nCommand not found\n")