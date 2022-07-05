// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

import "@openzeppelin/contracts/access/Ownable.sol";

contract NativeOracleBridge is Ownable {
    event  Requested(address indexed requestor, address indexed oracle, uint callId);
    event  Replied(address indexed requestor, address indexed oracle, uint callId);

     struct Request {
        uint id;
        uint requested_at;
        string[] data;
        address bridge;
        function(uint callId, string[] memory) external callback;
     }
     mapping (address => Request[]) public requests;

     struct OracleBridge {
        address evm_contract;
        string native_contract;
        string oracle_native_contract;
     }

     OracleBridge[] public bridges;

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
     function registerOracleBridge(address _evm_contract, string memory _native_contract, string memory _oracle_native_contract) external onlyOwner returns(bool) {
        require(!this.oracleBridgeExists(_evm_contract), "Oracle Bridge already registered");
        bridges.push(OracleBridge(_evm_contract, _native_contract, _oracle_native_contract));
        return true;
     }

     function removeOracleBridge(address _evm_address) external onlyOwner returns (bool) {
        for(uint i; i < bridges.length; i++){
            if(bridges[i].evm_contract == _evm_address){
                delete bridges[i];
                return true;
            }
        }
        require(false, "No oracle bridge found");
     }

     // REQUEST HANDLING ================================================================ >
     function request(uint callId, function(uint callId, string[] memory) external callback, address payable bridge, string[] memory data) external payable returns (bool) {
        require(msg.value == fee, "Send enough TLOS to pay for the response gas");
        require(requests[msg.sender].length < maxRequests, "Maximum requests reached, wait for replies or delete one");

        // CHECK EXISTS
        require(!this.requestExists(msg.sender, callId), "Call ID already exists");
        require(this.oracleBridgeExists(bridge), "Oracle Bridge was not found, make sure the address is correct");

        // SEND HALF OF FEE TO ORACLE EVM ADDRESS SO IT CAN SEND THE RESPONSE BACK, KEEP THE REST TO SEND THAT RESPONSE BACK TO CALLBACK
        bridge.transfer(fee / 2);

        // BUILD REQUEST
        requests[msg.sender].push(Request (callId, block.timestamp, data, bridge, callback));

        emit Requested(msg.sender, bridge, callId);

        return true;
     }

     function deleteRequest(uint id) external returns (bool) {
        for(uint i; i < requests[msg.sender].length; i++){
            if(requests[msg.sender][i].id == id){
                delete requests[msg.sender][i];
                return true;
            }
        }
        return false;
     }

     // REPLY HANDLING ================================================================ >
     function reply(uint callId, address requestor, string[] memory args) external {
        require(this.oracleBridgeExists(msg.sender), "Only a registered oracle bridge EVM address can call this function");
        for(uint i; i < requests[requestor].length; i++){
            if(requests[requestor][i].id == callId){
                requests[requestor][i].callback(callId, args);
                // TODO: MAKE RESULTS PUBLICLY VIEWABLE (this won't be user friendly on explorer ^) ?
                delete requests[requestor][i];
                emit Replied(requestor, requests[requestor][i].bridge, callId);
                return;
            }
        }
        require(false, "Request not found");
     }

     // UTIL ================================================================ >
     function oracleBridgeExists(address bridge) external view returns (bool) {
         for(uint i; i < bridges.length;i++){
             if(bridges[i].evm_contract == bridge){
                 return true;
             }
         }
         return false;
     }
     function requestExists(address requestor, uint id) external view returns (bool) {
        for(uint i; i < requests[requestor].length; i++){
            if(requests[requestor][i].id == id){
                return true;
            }
        }
        return false;
     }
}