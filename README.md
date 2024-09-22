# Accelerated Similarity Metric Library - CC & MSE

## Team Information
**Team number:** AOHW-321  
**Project name:** Accelerated Similarity Metric Library - CC & MSE  
**University name:** Politecnico di Milano  
**Participant(s):** Davide Ettori, Federico Mansutti  
**Email:** davide.ettori@mail.polimi.it, federico.mansutti@mail.polimi.it  
**Supervisor name:** Prof. Davide Conficconi  
**Supervisor e-mail:** davide.conficconi@polimi.it  

## Links
- **Link to YouTube Video(s):** [Video Link](https://youtu.be/Sh6HkgVwAgA)
- **Link to project repository:** [GitHub Repository](https://github.com/FedericoMansutti/Accelerated_Similarity_Metric_Library_CC_MSE)

## Board and Software
**Board used:** Versal VCK5000 QDMA2022.2
**Software Version:** Vitis 2022.2  

## Project Description
Similarity metric computation is one of the most compute-intensive steps in several procedures related to Machine Learning, image analysis, Computer Vision, and many others. Among these applications, image registration stands as pivotal. Employed in medical imaging and robotics, it uses similarity metrics to guide the heuristic registration algorithm. In this context, Cross-Correlation (CC), Mean Square Error (MSE), Peak Signal-to-Noise Ratio (PSNR), Root Mean Squared Error (RMSE), and Squared Cross-Correlation (SCC) are promising, opening to high-performance solutions without significant accuracy drops. This project focuses on tailoring CC, MSE, PSNR, RMSE, and SCC procedures to unleash the benefits of cutting-edge Versal technologies.

While those metrics have been implemented with varying degrees of success on FPGA architectures, this is the first implementation, to the best of our knowledge, of MSE, CC, PSNR, RMSE, and SCC acceleration on a Versal machine with AI Engine that is able to generate various accelerators, letting the user choose the metric to use and the number of kernels on the AI Engine. By applying vector operations, map-reduce, axi burst, and multiple kernels, a speedup up to 5x was achieved compared to the software version.

## Archive Description
The folder `accelerating_mse_cc` is the root folder and contains all the templates. Each template has:
- an `aie` folder containing the code for the AI Engine,
- a `data_movers` folder containing the code for the Programmable Logic part,
- a `hw` folder containing the bitstreams, and
- a `sw` folder containing the host code in C++.

The `launch.py` file and the `kernel_automation` folder contain the automation part in Python, which will be explained later on. The `img_ref.txt` and `img_float.txt` files contain the two images which must be compared for calculating the metric. Finally, there are some `setup_all.sh` files, which are needed to source Vitis from your local path.

The base templates, already provided, are compiled for both HW and HW_EMU and they have the bitstreams inside the `hw` folder. If you want to run the basic templates, you can skip steps from 3 to 7.

## Usage Instructions
To use this library, follow these steps:

1. Download all the files from GitHub:
    ```sh
    git clone https://github.com/FedericoMansutti/Accelerated_Similarity_Metric_Library_CC_MSE
    ```
2. Enter this folder, which is the root, and open a terminal from here. All the commands below must be run from a terminal inside this folder.
3. Choose the metric (MSE, CC, PSNR, RMSE, SCC) and the number of kernels (1, 2) you want the accelerator to use. Then source Vitis from your local path (cpp and python if needed).
4. You can choose to launch the bitstreams already loaded in the hw folders in the base templates. If so skip to step 7. Otherwise, type:
    ```sh
    python3 main.py mse 2
    ```
   This creates the accelerator for MSE with 2 kernels (use the same pattern for other combinations).
5. Check that a folder named `template_{metric chosen}_{number of kernel chosen}_kernels` has been created in the root folder.
6. Choose if you want to run in HW, if you have a Versal machine, or in HW_EMU if you don't but still want to test the project. Keep in mind that HW_EMU is not accurate and may give inaccurate metrics.
7. Compile the bitstreams for HW or HW_EMU by typing:
    ```sh
    python3 launch.py compile_hw {metric chosen}_{number of kernel chosen}_kernels
    ```
   or
    ```sh
    python3 launch.py compile_hw_emu {metric chosen}_{number of kernel chosen}_kernels
    ```
   For example:
    ```sh
    python3 launch.py compile_hw mse_2_kernels
    ```
8. Fill the files `img_ref.txt` and `img_float.txt` with the reference image and floating image. You can provide any size (2D or 3D) at the start of the files using width, height, and depth. They are already filled with a clear example.
9. Once the compilation is finished, type:
    ```sh
    python3 launch.py run_hw {metric chosen}_{number of kernel chosen}_kernels
    ```
   or
    ```sh
    python3 launch.py run_hw_emu {metric chosen}_{number of kernel chosen}_kernels
    ```
   For the example above:
    ```sh
    python3 launch.py run_hw mse_2_kernels
    ```
10. **View the resulting metric** computed by the AI Engine accelerator, which will be shown in the terminal.
