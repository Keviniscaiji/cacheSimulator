# Cache Simulator (CaSim)

CaSim is a versatile cache simulator designed to facilitate the testing and analysis of cache memory behavior in various scenarios. This software is containerized using Docker for ease of deployment and isolation. Users can choose to set up the environment through a direct download followed by an `npm install`, or build and run the container using Docker commands.

## Prerequisites

- Docker
- Node.js and npm (if choosing the npm install method)

## Installation and Setup

### Option 1: Using npm

If you prefer to run the simulator without Docker, ensure you have Node.js and npm installed on your system. Follow these steps:

1. Clone the repository or download the source code to your local machine.
2. Navigate to the project directory and run:
    ```bash
    npm install
    ```

### Option 2: Using Docker

For those who prefer a Docker-based setup, use the following commands:

1. Clone the repository or download the source code to your local machine.
2. Navigate to the project directory and build the Docker image:
    ```bash
    docker build -t casim .
    ```
3. Once the build is complete, run the container:
    ```bash
    docker run -p 80:80 -p 8080:8080 casim
    ```

This command runs the simulator and maps ports `80` and `8080` from the container to the host, allowing access to the application.

## Usage

After successfully launching the simulator, you can access it through your web browser:

- Navigate to `http://localhost`.

## Features

The Cache Simulator (CaSim) offers a sophisticated environment for simulating and analyzing cache memory performance, featuring customizable cache configurations and advanced cache management policies. Key features include:

- **Cache Policies**: Supports both Least Recently Used (LRU) and Least Frequently Used (LFU) cache replacement policies. These policies help in determining which cache entries to replace, making the simulator versatile for different types of cache behavior analysis.

- **Prefetching Strategies**: Incorporates advanced prefetching strategies to improve cache performance and reduce cache miss rates. The simulator allows users to select between "+1" (sequential) and "Strided" prefetching, enabling detailed experimentation on prefetching's impact on cache efficiency.

- **Insertion Strategies**: Features Multiple Insertion Policy (MIP) and Latest Insertion Policy (LIP) strategies for cache block insertion. These options offer users the flexibility to experiment with different approaches for inserting new blocks into the cache, providing insights into how insertion strategies affect cache performance.

- **Customizable Cache Configuration**: Users can tailor the cache simulator to their specific needs by editing two key parameters:
    - **Cache Block Size**: Allows for the adjustment of the size of individual cache blocks, accommodating a wide range of data sizes and application scenarios.
    - **Set Size**: Enables users to define the size of cache sets, facilitating experiments with different set-associative mapping strategies.

- **Prefetch Strategy Selection**: Offers the capability to choose the prefetching strategy that best suits the user's simulation objectives, providing a deeper understanding of how different prefetching techniques can optimize cache performance.

This simulator is designed for researchers, students, and professionals interested in cache memory systems, providing a comprehensive tool for exploring the intricacies of cache management, performance optimization techniques, and the effects of various cache configurations and policies.

