#!/usr/bin/env bash

# /home/weimin/build/src/standalone-ptxsema /home/kenun/wasmfuzz/ethfuzz/repo/test/contracts/integer_overflow_add.hex --fsanitize=ibsan -txs-len=1
LIFTER=$1
EVM_RET=1 $LIFTER $2 -o ./bytecode.ll --hex --dump $3 # lift hex to PTXIR

if [ $? -ne 0 ]; then
    echo "Fail to lift the bytecode at {$2} to IR"
    exit 1
fi


#link
llvm-as ./bytecode.ll -o main.bc && \
llvm-link main.bc ../../rt.o.bc -o kernel.bc && \
llc -mcpu=sm_86 ./kernel.bc -o kernel.ptx
# for debug 
llvm-dis kernel.bc -o kernel.ll

if [ $? -ne 0 ]; then
    echo "Fail to compile the IR to the PTX code"
    exit 1
fi

exit 0
