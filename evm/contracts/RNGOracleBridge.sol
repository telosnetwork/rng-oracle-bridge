// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

import "@openzeppelin/contracts/access/Ownable.sol";

interface IRNGOracleConsumer {
    function receiveRandom(uint, uint) external;
}

contract RNGOracleBridge is Ownable {
    event  Requested(address indexed requestor, uint callId);
    event  Replied(address indexed requestor, uint callId, uint random);

     struct Request {
        uint id;
        address caller_address;
        uint caller_id;
        uint requested_at;
        uint64 seed;
        uint min;
        uint max;
     }
     mapping (address => uint) public request_count;
     Request[] public requests; // Not using a mapping to be able to read from accountstate in native (else we need to know the mapping key we want to lookup)
     uint public fee;
     uint public maxRequests;
     address public oracle_evm_contract;

      constructor(uint _fee, uint _maxRequests, address _oracle_evm_contract) {
        fee = _fee;
        maxRequests = _maxRequests;
        oracle_evm_contract = _oracle_evm_contract;
      }

     // SETTERS  ================================================================ >
     function setFee(uint _fee) external onlyOwner returns(bool) {
        fee = _fee;
        return true;
     }

     function setMaxRequests(uint _maxRequests) external onlyOwner returns(bool) {
        maxRequests = _maxRequests;
        return true;
     }

     function setOracleEVMContract(address _oracle_evm_contract) external onlyOwner returns(bool) {
        oracle_evm_contract = _oracle_evm_contract;
        return true;
     }

     // REQUEST HANDLING ================================================================ >
     function request(uint callId, uint64 seed, uint min, uint max) external payable returns (bool) {
        require(msg.value == fee, "Send enough TLOS to pay for the response gas");
        require(request_count[msg.sender] < maxRequests, "Maximum requests reached, wait for replies or delete one");

        // CHECK EXISTS
        require(!this.requestExists(msg.sender, callId), "Call ID already exists");

        // SEND HALF OF FEE TO ORACLE EVM ADDRESS SO IT CAN SEND THE RESPONSE BACK, KEEP THE REST TO SEND THAT RESPONSE BACK TO CALLBACK
        payable(oracle_evm_contract).transfer(fee / 2);

        request_count[msg.sender]++;

        // BUILD REQUEST
        uint id = 0;
        if(requests.length > 0){
            id = requests[requests.length - 1].id + 1;
        }
        requests.push(Request (id, msg.sender , callId, block.timestamp, seed, min, max));

        emit Requested(msg.sender, callId);

        return true;
     }

     function deleteRequest(uint id) external returns (bool) {
        for(uint i = 0; i < requests.length; i++){
            if(requests[i].id == id){
                require(msg.sender == requests[i].caller_address || msg.sender == owner(), "Only the requestor or owner can delete a request");
                address caller = requests[i].caller_address;
                requests[i] = requests[requests.length - 1];
                requests.pop();
                request_count[caller]--;
                return true;
            }
        }
        revert("Request not found");
     }

     // REPLY HANDLING ================================================================ >
     function reply(uint callId, uint random) external {
        require(msg.sender == oracle_evm_contract, "Only the native oracle bridge EVM address can call this function");
        for(uint i = 0; i < requests.length; i++){
            if(requests[i].id == callId){
                uint caller_id = requests[i].caller_id;
                address caller = requests[i].caller_address;
                IRNGOracleConsumer(caller).receiveRandom(caller_id, random);
                requests[i] = requests[requests.length - 1];
                requests.pop();
                request_count[caller]--;
                emit Replied(caller, caller_id, random);
                return;
            }
        }
        revert("Request not found");
     }

     // UTIL ================================================================ >
     function requestExists(address requestor, uint id) external view returns (bool) {
        for(uint i = 0; i < requests.length; i++){
            if(requests[i].caller_id == id && requests[i].caller_address == requestor ){
                return true;
            }
        }
        return false;
     }
}