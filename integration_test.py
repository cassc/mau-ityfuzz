import glob
import subprocess
import os
import time


TIMEOUT_BIN = "timeout" if os.name == "posix" else "gtimeout"


def test_one(path):
    # cleanup
    os.system(f"rm -rf {path}/build")
    print(path)
    # compile with solc
    p = subprocess.run(
        " ".join(["solc", f"{path}/*.sol", "-o", f"{path}/",
                  "--bin", "--abi", "--overwrite", "--base-path", "."]),
        shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    if b"Error" in p.stderr or b"Error" in p.stdout:
        print(f"Error compiling {path}")
        return

    
    p = subprocess.run(
        " ".join(["~/build/sema/src/standalone-ptxsema", f"{path}/main.bin", "-o", f"{path}/bytecode.ll",
                  "--hex", "--dump"]),
        shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    if b"Error" in p.stderr or b"Error" in p.stdout:
        print(f"Error compiling {path}")
        return
    print("to ll ")
    
    p = subprocess.run(
        " ".join(["llvm-link", "~/build/rt.o.bc", f"{path}/bytecode.ll", "-o", f"{path}/kernel.bc"]),
        shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print("to bc ")

    p = subprocess.run(
        " ".join(["~/wasmfuzz/ethfuzz/repo/scripts/llc-16", "-mcpu=sm_86", f"{path}/kernel.bc", "-o", f"{path}/kernel.ptx"]),
        shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)  
    # print("to  ptx")
    # # run fuzzer and check whether the stdout has string success
    start_time = time.time()
    p = subprocess.run(" ".join([
        "LD_LIBRARY_PATH=/home/weimin/build/runner/", TIMEOUT_BIN, "3m", "./cli/target/release/cli", "-t", 
        f"'{path}/*'",  "-f", "--panic-on-bug", "--ptx-path", f"{path}/kernel.ptx",  "--gpu-dev", "1"]),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=True
    )

    if b"target hit" not in p.stderr and b"bug() hit" not in p.stdout:
        print(p.stderr.decode("utf-8"))
        print(p.stdout.decode("utf-8"))
        raise Exception(f"Failed to fuzz {path}")

    # # clean up
    os.system(f"rm -rf {path}/*.abi")
    os.system(f"rm -rf {path}/*.bin")
    os.system(f"rm -rf {path}/bytecode.ll")
    os.system(f"rm -rf {path}/kernel.bc")
    os.system(f"rm -rf {path}/kernel.ptx")

    print(f"=== Success: {path}, Finished in {time.time() - start_time}s")


def build_fuzzer():
    # build fuzzer
    os.chdir("cli")
    subprocess.run(["cargo", "build", "--release"])
    os.chdir("..")


import multiprocessing

if __name__ == "__main__":
    # build_fuzzer()
    with multiprocessing.Pool(3) as p:
        p.map(test_one, glob.glob("./tests/*/", recursive=True))
