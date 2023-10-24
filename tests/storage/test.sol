// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.8.15;

import "../../solidity_utils/lib.sol";

contract main {
    uint256 lock = 123;
    // uint256 public key = 456;

    // function attack (uint256 _a, uint256 _b) public  payable  {
    //     // if (msg.sender != address(0x35c9dfd76bf02107ff4f7128Bd69716612d31dDb)) {
    //     //     return;
    //     // }

    //     if (msg.sender != address(0x68Dd4F5AC792eAaa5e36f4f4e0474E0625dc9024)) {
    //         return;
    //     }
    //     // lock += _a;
    //     // key += _a;
    //     if (lock > 123 && _a < 0x200 && _a > 0x100 && _b < 0x200)
    //     // if (lock > 123 && lock < 200  && _a < 0x200 && _a > 0x100 && _b < 0x200 && _b > 0x100 )
    //         bug();//lock +=1;
    // }
    function unlock (uint256 _a) public payable {
        lock = _a;
    }
    function setUserUseReserveAsCollateral(address adr, bool r, uint256 _a, uint256 _b) public {
        if (adr != address(0x68Dd4F5AC792eAaa5e36f4f4e0474E0625dc9024)) {
            return;
        }
        if (lock > 123 && _a < 0x200 && _a > 0x100 && _b < 0x200)
            bug();
        return;
    }
}

// #gpu
// e1fa7638
// 0000000000000000000000000000000000000000000000000000000000000000
// 000000000000ff00000102000000a2a2a2a2a2a2a2a2a2a2a2a2000000000000

// #corpus
// e1fa7638
// 0000000000000000000000000000000000000000000000000000000000000000
// 000000000000ff00000102000000a2a2a2a2a2a2a2a2a2a2a2a2000000000000