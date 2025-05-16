#!/usr/bin/env python3
import sys
import json
import subprocess
import os

# from eth_abi.packed import encode_packed
from eth_abi import encode_abi
import sha3


def format_arg(types_str, args):
    is_int = lambda x: "int" in x

    rets = list()
    for i in range(0, len(args)):
        arg = args[i]
        # print(types_str)
        if arg.startswith("#x"):
            rets.append("0x" + arg[2:])
        elif is_int(types_str[i]):
            rets.append(int(arg))
        else:
            rets.append(arg)
            
    return rets


def gen_cmd(_report):
    if len(_report["tx_seq"]) <= 1:
        # print("[-] Smartest fails to generate exploits.")
        return None
    
    kernel_args = ["CUDA_DEVICE_ID=3", "~/build/tests/loader", "./kernel.ptx", "128"]
    for tx in _report['tx_seq'][1:]:        
        callvalue = tx["keywords"]["msg.value"]

        k = sha3.keccak_256()
        raw = "{}({})".format(tx["methodName"], ','.join(tx["argTypes"]))
        k.update(raw.encode())
        func_sig = k.hexdigest()[:8]

        calldata = func_sig + encode_abi(tx["argTypes"], format_arg(tx["argTypes"], tx["args"])).hex()
        kernel_args += [callvalue, calldata]
    
    kernel_args.append(_report["snapshot"])
    # print(*kernel_args)
    return kernel_args
    # subprocess.run(kernel_args)


def main():
    assert len(sys.argv) == 2, "./%s <path_to_smartest_dir>".format(sys.argv[0])

    pass_cnt = 0
    Total = 0
    
    with open("/home/weimin/wasmfuzz/ethfuzz/VeriSmart-public/verified_poc.csv", 'r') as f:
        candidates = f.readlines()

    for json_name in candidates:
        json_name = json_name.strip("\n")
        # if json_name != "2018-13626_13.json":
        #     continue

        with open(os.path.join(sys.argv[1], json_name), "r") as f:
            _report = json.load(f)
        if _report["kind"] != "IO" or _report["snapshot"] == "ERROR":
            continue
        
        print("===================== Process {} ============".format(json_name))        
 
        base_cmd = "~/build/src/standalone-ptxsema {} ".format("~/wasmfuzz/ethfuzz/VeriSmart-public/VeriSmart-benchmarks/built_cve/"+json_name.split("_")[0])
        base_cmd += "-o ./bytecode.ll --hex --dump "
        base_cmd += "--fsanitize=ibsan -txs-len={} --disable-instrument ".format(len(_report["tx_seq"]) - 1)
        base_cmd += "&& llvm-as ./bytecode.ll -o main.bc "
        base_cmd += "&& llvm-link main.bc /home/weimin/build/rt.o.bc --only-needed -o kernel.bc "
        base_cmd += "&& llc -mcpu=sm_86 ./kernel.bc -o kernel.ptx"

        try:
            loader_cmd = gen_cmd(_report)
        except:
           pass
        
        if loader_cmd == None:
            continue
        else:
            cmd  = base_cmd + "&& " + ' '.join(loader_cmd)
            
            # try three times
            status, ret = subprocess.getstatusoutput(cmd)
            if status != 0:
                status, ret = subprocess.getstatusoutput(cmd)
                if status != 0:
                    status, ret = subprocess.getstatusoutput(cmd)

            last_line = ret.split('\n')[-1]
            Total += 1
            if last_line != "01010101":
                # print("Failed")
                print("[CMD] ", cmd)
                # print("----------- Ret -------- \n" + ret + "\n" + "-"*50)
            else:
                pass_cnt += 1

    print("Passed {} / {}".format(pass_cnt, Total))


main()