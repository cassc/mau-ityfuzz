# Test librunner.so

## TODO
- use Solidity test data `https://github.com/ethereum/solidity/tree/develop/test/libsolidity/semanticTests`

## Storage
```
contract Storage {
    address owner = msg.sender;
    bool public purchasingAllowed = true;
    uint256 public totalSupply = 0xffff;
    uint8 public decimals = 18;
    address public icoContractAddress = 0x5B38Da6a701c568545dCfcB03FcB875f56beddC4;

    uint public a = 1;
    function Storage(uint _a) {
        a = _a;
    }
    function set(uint256 _a) public returns(uint){
        a += _a;
        return a;
    }
}
```

Hex code at `repo/sema/tests/contracts/storage.hex`

1. AOT translation
2. linkage.
3. codegen.
4. run.

```bash
./build/ptxsema ./test/storage/storage.hex -o bytecode.ll --hex --dump
llvm-link ./build/rt.o.bc ./bytecode.ll -o ./kernel.bc
llvm-dis kernel.bc -o kernel.ll

./build/test/test-runner ./kernel.ptx ./test/storage/txs.json

llvm-as-16 kernel.ll -o kernel.bc
llc-16 -mcpu=sm_86 kernel.bc -o kernel.ptx
./build/test/test-runner ./kernel.ptx ./test/storage/txs.json
```

## Complex-conditions
```bash
./build/ptxsema ./test/complex-condition/main.bin -o ./bytecode.ll --hex --dump && llvm-link ./build/rt.o.bc ./bytecode.ll -o ./kernel.bc && llvm-dis kernel.bc -o kernel.ll && llc-16 -mcpu=sm_86 kernel.bc -o kernel.ptx && ./build/test/test-runner ./kernel.ptx ./test/complex-condition/txs.json
```

```bash
~/build/sema/src/standalone-ptxsema ~/wasmfuzz/ethfuzz/repo/test/complex-condition/main.bin -o ./bytecode.ll --hex --dump && llvm-link ~/build/rt.o.bc ./bytecode.ll -o ./kernel.bc && llvm-dis kernel.bc -o kernel.ll && ~/wasmfuzz/ethfuzz/repo/scripts/llc-16 -mcpu=sm_86 kernel.bc -o kernel.ptx && /data_HDD/weimin/EXP-Artifact/Smartian-GPU/build/Smartian fuzz -t 300 -p  ~/wasmfuzz/ethfuzz/repo/test/complex-condition/main.bin -a  ~/wasmfuzz/ethfuzz/repo/test/complex-condition/main.abi -k ./kernel.ptx -v 2 -o /tmp/test -g 0
```
