The code needs a lot of improvement but it works. ill fix it in the future

# Image Processing Project

This project focuses on the conversion images from bitmap (BMP) format to grayscale, incorporating specific image rotation transformations based on the input parameter N. The N parameter, taken as a command-line argument, determines the transformation applied to the image: N = 0 for no transformation, N = 1 for horizontal rotation, and N = 2 for vertical rotation.

## Overview

The project encompasses several steps, including the implementation of serial code for reading and storing bitmap images, their conversion, and rotation. It also involves the use of parallel computing techniques like OpenMP and OpenMPI to accelerate processing. The resulting images are saved in different files for each implementation.

## Contents

- **Introduction**: Discusses the objective of creating image processing software for grayscale conversion and reversal.

## Methodology

### Serial Code Methodology

The software aims to convert BMP images to grayscale or reverse them horizontally or vertically. Using the BMP library, images are stored. The grayscale function performs pixel multiplication with the weight matrix and coefficient, storing modified pixels in a new BMP image. The last two primary algorithms deal with changing the image orientation horizontally and vertically by reading images inversely and placing pixels in a new BMP image.

### OpenMP Code Methodology

Parallelization using OpenMP targets speeding up software execution by simultaneously running routines. The BMP library, along with the OpenMP library, utilizes multiple threads to execute processes concurrently, reducing execution time.

### OpenMPI Code Methodology

Similarly aimed at reducing execution time, OpenMPI parallelization differs in segmenting the image into parts, each processed by separate processes and aggregated into matrices to construct the new image. The reversal process remains sequential and is not parallelized with the MPI protocol.
