// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.8.15;

import "../../solidity_utils/lib.sol";

contract main {
    // solution: a = 1
    function flashLoan(address a,address b,uint256 c ,bytes memory d) public {
        require(c > 0 , "");
        require(c > 0xffffff , "");
        require(c > 0xffffffffff , "");
        require(c > 0xffffffffffffff , "");
        require(c > 0xffffffffffffffffff , "");
        require(c > 0xfffffffffffffffffffff , "");
        require(c < 0xfffffffffffffffffffff10 , "");
        require(c > 0xfffffffffffffffffffff00 , "");
        bug();
    }

    // function process(uint256[2] calldata a) public payable returns (string memory){
    //     // require(msg.value == 1 ether, "");
    //     // require(a[0] - 0x12d312 > 0 && a[0] - 0x12d312 < 0xf324 && a[1] + 0xffff81 > 0x1000000000000000000 && a[1] + 0xffff81 < 0x100000000000000f000, "");
    //     // require(a[0] - 12  == 4 && a[1] > 0x12200 && a[1] < 0x1222ff, "");
    //     // require(a[0] - 12  == 4 && a[1] + 81 == 100, "");
    //     // require(a[0] - 12  == 999, "");
    //     require(a[0] > 0 , "");
    //     require(a[0] > 0xffffff , "");
    //     require(a[0] > 0xffffffffff , "");
    //     require(a[0] > 0xffffffffffffff , "");
    //     require(a[0] > 0xffffffffffffffffff , "");
    //     require(a[0] > 0xfffffffffffffffffffff , "");
    //     require(a[0] > 0xfffffffffffffffffffffffff , "");
    //     // require(a[0] != 0, "a[0] != 1");
    //     require(a[1] != 0, "a[1] != 2");
    //     // require(a[2] != 0, "a[2] != 3");
    //     // require(a[3] - 12 == 4, "a[3] != 4");
    //     // require(a[4] + 81 == 100, "a[4] != 5");
    //     // require(a[5] < 5, "a[5] != 6");
    //     // require(a[6] == 7, "a[6] != 7");
    //     // require(a[7] == 8, "a[7] != 8");
    //     bug();
    //     return 'Hello Contracts';
    // }
}
