contract hashmapping {

    mapping(address => uint256) public balances;

    function depositFunds() public payable {
        balances[msg.sender] = msg.value;
    }
 }  
 
