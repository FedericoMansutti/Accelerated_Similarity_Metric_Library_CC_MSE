Team number: AOHW-321
Project name: Accelerated Similarity Metric Library - CC & MSE
Link to YouTube Video(s): https://url
Link to project repository: https://github.com/FedericoMansutti/versal_accelerating_mse_cc
 
University name: Politecnico di Milano
Participant(s): Davide Ettori, Federico Mansutti
Email: davide.ettori@mail.polimi.it, federico.mansutti@mail.polimi.it
Supervisor name: Prof. Davide Conficconi
Supervisor e-mail: davide.conficconi@polimi.it
 
Board used: versal-vck5000
Software Version: Vitis 2022.2
Brief description of project:

Similarity metric computation is one of the most compute-intensive
steps in several procedures related to Machine Learning, image anal-
ysis, Computer Vision, and many others. Among these applications,
image registration stands as pivotal. Employed in medical imaging
and robotics, it uses similarity metrics to guide the heuristic registra-
tion algorithm. In this context, Cross-Correlation (CC), Mean Square
Error (MSE), Peak Signal-to-Noise Ratio (PSNR), Root Mean Squared Error (RMSE) and Squared Cross-Correlation (SCC)
are promising, opening to high-performant solutions without significant accu-
racy drops. This project focuses on tailoring CC, MSE, PSNR, RMSE and SCC
procedures to unleash the benefits of cutting-edge Versal technologies.
While those metrics have been implemented with varying de-
gree of success on FPGA architectures, this is the first implementation,
to the best of our knowledge, of MSE, CC, PSNR, RMSE and SCC acceleration on
a Versal machine with AI Engine. We present an AI Engine-based li-
brary for Similarity Metric Computation (AIESMC) for both 2D and
3D images. AIESMC is able to generate various accelerators, letting
the user choose the metric to use and the number of kernels on the
AI Engine. By applying vector operations, map-reduce, axi burst and
multiple kernels, a speedup up to 5x was achieved compared to the
SW version.

Description of archive (explain directory structure, documents and source files):
the folder 'accelerating_mse_cc' is the root folder and it will contain all the templates. Each template has an 'aie' folder containing the code for the AI Engine, a 'data_movers' folder containing the code for the Programmable Logic part, a 'hw' folder containing the bitstreams and a 'sw' folder containing the host code in cpp.
the 'launch.py' file and the 'kernel_automation' folder contains the automation part, in python, which will be explained later on
the 'img_ref.txt' and 'img_float.txt' files contains the 2 images which must be comapred for calculating the metric
finally, there are some 'setup_all.sh' files, which are needed to source Vitis from your local path


The base templates, already provided, are compiled for both HW and HW_EMU and they have the bitstreams inside the hw folder. If you want to run the basic templates you can skip steps from 3 to 7
To use this library you have to:
step 1:
    download all the files from github, typing 'git clone https://github.com/FedericoMansutti/versal_accelerating_mse_cc.git' from a termial inside a folder of your choice
step 2:
    enter in the 'accelerating_mse_cc', which is the root, and open a terminal from here. All the commands below must be run from a terminal inside this folder
step 3:
    choose the matric (MSE, CC, PSNR, RMSE, SCC) and the number of kernels (1, 2) you want the accelarator to use. Then source Vitis from your local path
step 4:
    type 'python3 kernel_automation/main.py mse 2' for creating the accelarator for MSE with 2 kernels (same pattern for other combinations)
step 5:
    check that a folder named template_{metric chosen}_{number of kernel chosen}_kernels has been created in the root folder
step 6: 
    choose if want to run in HW, if you have a versal machine, or in HW_EMU if you don't but you still want to test che project
step 7:
    complile the bitstreams for HW or HW_EMU by typing 'python3 launch.py {metric chosen}_{number of kernel chosen}_kernels compile_hw' or 'python3 launch.py {metric chosen}_{number of kernel chosen}_kernels compile_hw_emu'. In the example it should be 'python3 launch.py mse_2_kernels compile_hw'
step 8:
    fill the files 'img_ref.txt' and 'img_float.txt' with the reference image and floating image. You can provide any size (2D or 3D) at the start of the files using width, hegight and depth. They are already filled with a clear example
step 9:
    once the compilation is finished, type 'python3 launch.py {metric chosen}_{number of kernel chosen}_kernels run_hw' or 'python3 launch.py {metric chosen}_{number of kernel chosen}_kernels run_hw_emu'. In the example it should be 'python3 launch.py mse_2_kernels run_hw'
step 10:
    in the terminal will be shown the resulting metric computed by the AI Engine accelarator.