# Use NVIDIA CUDA 12.2.0 as the base image (Ubuntu 20.04)
FROM nvidia/cuda:12.8.0-devel-ubuntu20.04

# Set working directory
WORKDIR /mau/mau_profile-0.2

# Set environment variables for non-interactive apt-get
ENV DEBIAN_FRONTEND=noninteractive

# Install prerequisites for adding the LLVM repository
RUN apt-get update && apt-get install -y \
    software-properties-common \
    wget \
    gnupg \
    && rm -rf /var/lib/apt/lists/*


# Add LLVM repository (focal = Ubuntu 20.04)
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
    && echo "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-16 main" >> /etc/apt/sources.list.d/llvm.list

# Update package list and install basic dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-7 \
    g++-7 \
    curl \
    curl \
    python3 \
    python3-dev \
    vim \
    libc6-dev \
    libstdc++-10-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Clang, LLVM, and related tools (version 16)
RUN apt-get update && apt-get install -y \
    clang-16 \
    clang++-16 \
    lld-16 \
    llvm-16 \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

# Install CMake manually (version 3.20.0 as per your script)
RUN curl -LO https://github.com/Kitware/CMake/releases/download/v3.20.0/cmake-3.20.0-linux-x86_64.sh \
    && chmod +x cmake-3.20.0-linux-x86_64.sh \
    && ./cmake-3.20.0-linux-x86_64.sh --skip-license --prefix=/usr/local \
    && rm cmake-3.20.0-linux-x86_64.sh

# Set up symbolic links for Clang and LLVM tools
RUN ln -sf /usr/bin/clang-16 /usr/bin/clang \
    && ln -sf /usr/bin/clang++-16 /usr/bin/clang++ \
    && ln -sf /usr/bin/llvm-dis-16 /usr/bin/llvm-dis \
    && ln -sf /usr/bin/llc-16 /usr/bin/llc \
    && ln -sf /usr/bin/llvm-link-16 /usr/bin/llvm-link

# Set environment variables for compilers
ENV CC=/usr/bin/gcc-7
ENV CXX=/usr/bin/g++-7

# Optionally set CUDA path (already included in the base image at /usr/local/cuda-12.2)
ENV CUDAPATH=/usr/local/cuda-12.2

# Ensure Python3 is the default Python
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 1

# Verify installations
RUN gcc-7 --version \
    && g++-7 --version \
    && clang-16 --version \
    && cmake --version \
    && python --version \
    && nvcc --version

# Set default command
CMD ["bash"]
