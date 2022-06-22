// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

import "@openzeppelin/contracts/access/Ownable.sol";

contract NativeOracleBridge is Ownable {
    event  Requested(address indexed requestor, address indexed oracle, string callId);
    event  Replied(address indexed requestor, address indexed oracle, string callId);

     struct Request {
        uint requested_at;
        string[] data;
        address oracle;
        string callId;
        function(string[] memory) external callback;
     }
     mapping (address => Request[]) private requests;

     struct Oracle {
        address evm_address;
        string native_address;
     }

     Oracle[] public oracles;

     uint public fee;
     uint public maxRequests;

      constructor(uint _fee, uint _maxRequests) {
        fee = _fee;
        maxRequests = _maxRequests;
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

     // ORACLES REGISTRY ================================================================ >
     function registerOracle(address _evm_address, string memory _native_address) external onlyOwner returns(bool) {
        require(!this.oracleExists(_evm_address), "Oracle already registered");
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
        require(false, "No oracle found");
     }

     // REQUEST HANDLING ================================================================ >
     function request(string memory callId, function(string[] memory) external callback, address payable oracle, string[] memory data) external payable {
        require(msg.value == fee, "Send enough TLOS to pay for the response gas");
        require(requests[msg.sender].length < maxRequests, "Maximum requests reached, wait for replies or delete one");

        // CHECK EXISTS
        require(this.requestExists(msg.sender, callId), "Call ID already exists");
        require(this.oracleExists(oracle), "Oracle was not found, make sure the address is correct");

        // TODO: SEND HALF OF FEE TO ORACLE EVM ADDRESS SO IT CAN SEND THE RESPONSE BACK, KEEP THE REST TO SEND THAT RESPONSE BACK TO CALLBACK
        oracle.transfer(fee / 2);

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

     // REPLY HANDLING ================================================================ >
     function reply(string memory callId, address requestor, string[] memory args) external {
        require(this.oracleExists(msg.sender), "Only a registered oracle can call this function");
        for(uint i; i < requests[requestor].length; i++){
            if(keccak256(bytes(requests[requestor][i].callId)) == keccak256(bytes(callId))){
                requests[requestor][i].callback(args);
                // TODO: MAKE RESULTS PUBLICLY VIEWABLE (this won't be user friendly on explorer ^) ?
                delete requests[requestor][i];
                emit Replied(requestor, requests[requestor][i].oracle, callId);
                return;
            }
        }
        require(false, "Request not found");
     }

     // UTIL ================================================================ >
     function oracleExists(address oracle) external view returns (bool) {
         for(uint i; i < oracles.length;i++){
             if(oracles[i].evm_address == oracle){
                 return true;
             }
         }
         return false;
     }
     function requestExists(address requestor, string memory callId) external view returns (bool) {
        for(uint i; i < requests[requestor].length){
            if(keccak256(bytes(requests[requestor][i].callId)) == keccak256(bytes(callId))){
                return true;
            }
        }
        return false;
     }
}