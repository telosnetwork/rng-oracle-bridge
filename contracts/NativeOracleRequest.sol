// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

import "@openzeppelin/contracts/access/Ownable.sol";

contract NativeOracleRequest is Ownable {
    event  Requested(address indexed requestor, address indexed oracle, string callId);
    event  Answered(address indexed requestor, address indexed oracle, string callId);

     struct Request {
        uint requested_at;
        string[] data;
        address oracle;
        string callId;
        function(string[] memory) external callback;
     }
     mapping (address => Request[]) public requests;

     struct Oracle {
        address evm_address;
        string native_address;
     }

     Oracle[] public oracles;

     uint public fee;

      constructor(uint _fee) {
        fee = _fee;
      }

     // FEE  ================================================================ >
     function setFee(uint _fee) external onlyOwner returns(bool) {
        fee = _fee;
        return true;
     }

     // ORACLES REGISTRY ================================================================ >
     function registerOracle(address _evm_address, string memory _native_address) external onlyOwner returns(bool) {
        for(uint i; i < oracles.length;i++){
            if(oracles[i].evm_address == _evm_address){
                require(false, "Oracle already added, delete it first");
            }
        }
        oracles.push(Oracle(_evm_address, _native_address));
        return true;
     }

     function removeOracle(address _evm_address) external onlyOwner returns (bool) {
        for(uint i; i < oracles.length; i++){
            if(oracles[i].evm_address == _evm_address){
                delete oracles[i];
                return true;
            }
        }
        return false;
     }

     // REQUEST HANDLING ================================================================ >
     function request(string memory callId, function(string[] memory) external callback, address payable oracle, string[] memory data) external payable {
        require(msg.value == fee, "Needs TLOS to pay for response gas");
        // CHECK EXISTS
        for(uint i; i < requests[msg.sender].length; i++){
            if(keccak256(bytes(requests[msg.sender][i].callId)) == keccak256(bytes(callId))){
                require(false, "Call ID already exists");
            }
        }
        // CHECK ORACLE IS CORRECT
        bool found = false;
        for(uint i; i < oracles.length;i++){
            if(oracles[i].evm_address == oracle){
                found = true;
            }
        }
        require(found, "Oracle was not found, make sure the address is correct");

        // TODO: SEND HALF OF FEE TO ORACLE EVM ADDRESS SO IT CAN SEND THE RESPONSE BACK, KEEP THE REST TO SEND THAT RESPONSE BACK TO CALLBACK
        oracle.send(fee / 2);

        // BUILD REQUEST
        requests[msg.sender].push(Request (block.timestamp, data, oracle, callId, callback));

        emit Requested(msg.sender, oracle, callId);
     }

     function deleteRequest(string memory callId) external returns (bool) {
        for(uint i; i < requests[msg.sender].length; i++){
            if(keccak256(bytes(requests[msg.sender][i].callId)) == keccak256(bytes(callId))){
                delete requests[msg.sender][i];
                return true;
            }
        }
        return false;
     }

     // RESPONSE HANDLING ================================================================ >
     function respond(string memory callId, address requestor, string[] memory args) external {

        // TODO: MAKE SURE AN ORACLE IS CALLING

        for(uint i; i < requests[requestor].length; i++){
            if(keccak256(bytes(requests[requestor][i].callId)) == keccak256(bytes(callId))){
                requests[requestor][i].callback(args);
                delete requests[requestor][i];
                emit Answered(requestor, requests[requestor][i].oracle, callId);
            }
        }
        require(false, "Request not found");
     }
}