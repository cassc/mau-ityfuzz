// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.8.15;

import "../../solidity_utils/lib.sol";

contract main {
    uint256 public lock = 0;
    uint16 public dkey = 1;
    
    function unlock (uint16 key) public {
        dkey += key;
        if (dkey > 0x1234 && dkey < 0x1245)
            bug();
    }


}
