pragma solidity ^0.8.0;
    
contract Example {
    struct Store {
        string id;         
        uint time;         
    }
 
    mapping (address => Store) public purchases;

    function set(string memory _id, uint _time) public returns(bool) {
        purchases[msg.sender].id = _id;
        purchases[msg.sender].time = _time;
        return true;
    }

    function get() public view returns(Store memory) {
        return purchases[tx.origin];
    }
}
