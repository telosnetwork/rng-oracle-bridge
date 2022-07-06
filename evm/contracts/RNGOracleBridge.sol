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
        uint caller_id;
        uint requested_at;
        uint seed;
        uint min;
        uint max;
     }
     mapping (address => Request[]) public requests;

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
     function request(uint callId, uint seed, uint min, uint max) external payable returns (bool) {
        require(msg.value == fee, "Send enough TLOS to pay for the response gas");
        require(requests[msg.sender].length < maxRequests, "Maximum requests reached, wait for replies or delete one");

        // CHECK EXISTS
        require(!this.requestExists(msg.sender, callId), "Call ID already exists");

        // SEND HALF OF FEE TO ORACLE EVM ADDRESS SO IT CAN SEND THE RESPONSE BACK, KEEP THE REST TO SEND THAT RESPONSE BACK TO CALLBACK
        payable(oracle_evm_contract).transfer(fee / 2);

        // BUILD REQUEST
        requests[msg.sender].push(Request (callId, block.timestamp, seed, min, max));

        emit Requested(msg.sender, callId);

        return true;
     }

     function deleteRequest(uint id) external returns (bool) {
        for(uint i; i < requests[msg.sender].length; i++){
            if(requests[msg.sender][i].caller_id == id){
                delete requests[msg.sender][i];
                return true;
            }
        }
        return false;
     }

     // REPLY HANDLING ================================================================ >
     function reply(uint callId, address requestor, uint random) external {
        require(msg.sender == oracle_evm_contract, "Only the oracle bridge EVM address can call this function");
        bool found = false;
        for(uint i; i < requests[requestor].length; i++){
            if(requests[requestor][i].caller_id == callId){
                IRNGOracleConsumer(requestor).receiveRandom(requests[requestor][i].caller_id, random);
                delete requests[requestor][i];
                emit Replied(requestor, callId, random);
                found = true;
            }
        }
        require(found, "Request not found");
     }

     // UTIL ================================================================ >
     function requestExists(address requestor, uint id) external view returns (bool) {
        for(uint i; i < requests[requestor].length; i++){
            if(requests[requestor][i].caller_id == id){
                return true;
            }
        }
        return false;
     }
}