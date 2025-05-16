contract xcall {
    uint public a = 0;

    function depositFunds() public payable {
        a = 2;
    }

    function withdrawFunds () payable public  {
        msg.sender.call.value(0)();
        a -= 1;
    }
 }  
