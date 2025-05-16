Source code in this directory is downloaded from https://github.com/Kenun99/Mau-Artifact/releases

Minor changes to Dockerfile to make the compilation work.

--------------------------------------------------------------------------------

# PTXSema

## Workflow
1. Lifting. Lift the bytecode of Ethereum smart contract to the LLVM Assembly.
2. Instrumentation. AFL searches the "__AFL_SHM_ID".
3. Transalation. Translate the IR to the PTX64.
4. Fuzzing
    1. load the PTX code as the kernel function
    2. use CUDA APIs to activate throusands of threads to identify bitmaps
    3. extend the queue with the generated bitmap
4. report crashes due to the triggered sanitzers.

## Build from the Source Code

1. Install CUDA 12. Check [Cuda Document](https://developer.nvidia.com/cuda-downloads).
   Once installed, check `$nvidia-smi` to see whether the GPU device is activated.

2. Download LLC-16.
Currently, MAU uses LLVM-13 to compile source code because it is the latest version that support BitNum(256) in LLVM IR.
We use LLC-16 rather than LLC-13 because LLC-16 fixes some bugs in the NVPTX backend.
You can download LLC from `https://github.com/llvm/llvm-project` and install in scripts/llc-16.
```bash
mv llc ./scripts/llc-16
```

3. Install gcc toolchain and cmake-3.20.0
```bash
apt-get update && apt-get install gcc-7 -y && apt-get install g++-7 curl python -y
export CC=/usr/bin/gcc-7
export CXX=/usr/bin/g++-7
bash ./scripts/install_cmake.sh --prefix /usr/local # install cmake 3.20.0
# link python if necessary
sudo ln -s /usr/bin/python27 /usr/bin/python
```

4. let build MAU from the source code.
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release     -DBUILD_SHARED_LIBS=OFF     -DCUDAPATH=/usr/local/cuda-12  -DCMAKE_VERBOSE_MAKEFILE=ON    ..
cmake --build . --install
```

## Run An Example

Assume we have the smart contract written in Solidity. Note that Mau does not need Solidity source code.

1. Compile Solidity to EVM bytecode.
```bash
cd ./test/storage/
solc *.sol -o . --bin --abi --overwrite --base-path ../../
```

2. Convert to NVPTX `build/mau <bytecode> <nvptx>`. For example:
```bash
./build/mau ./test/storage/storage.hex kernel.ptx
```

You can check `build/mau` and see each step:
1. Convert to SIMD LLVM ASM
`build/ptxsema ./test/storage/storage.hex -o ./bytecode.ll --hex --dump`

2. Link with runtime
`./build/deps/bin/llvm-link build/rt.o.bc ./bytecode.ll -o ./kernel.bc`

3. Check the kernel code. `llvm-dis kernel.bc -o kernel.ll`

4. Convert LLVM ASM to PTX code. `./scripts/llc-16 -mcpu=sm_86 kernel.bc -o kernel.ptx`

## Know Limitations
1. You may fail to build MAU when `-j` is used. Run `make` again can solve in most of cases.
2. The storage slot is emulated in CUDA memory. Set a larger value for storage if necessary.
3. Ensure your GPU devide have enough memory. Otherwise, CUDA API would raise `cudaErrorMemoryAllocation`. You can change your GPU devide by executing `export CUDA_DEVICE_ID=<Number>`
