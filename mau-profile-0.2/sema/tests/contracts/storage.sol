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
