// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

interface IRNGOracleBridge {
    function request(uint callId, uint64 seed, uint callback_gas, address callback_address, uint number_count) external payable;
}

contract RNGOracleConsumer {
    IRNGOracleBridge bridge;
     struct Request {
        uint id;
        uint seed;
        uint count;
     }
     Request[] public requests;

    constructor(address _bridge) {
        bridge = IRNGOracleBridge(_bridge);
    }

    function makeRequest(uint64 seed, uint callback_gas, uint count) external  payable {
        require(msg.value > 0, "Request needs fee passed");
        uint callId = 0;
        if(requests.length > 0){
            callId = requests[requests.length - 1].id + 1;
        }
        requests.push(Request(callId, seed, count));
        bridge.request{value: msg.value }(callId, seed, callback_gas, address(this), count);
    }

    function receiveRandom(uint callId, uint[] calldata numbers) external {
        require(msg.sender == address(bridge), "Only the bridge contract can call this function");
         for(uint i = 0; i < requests.length;i++){
            if(requests[i].id == callId){
                requests[i] = requests[requests.length - 1];
                requests.pop();
                return;
            }
         }
        revert("Request not found");
    }
}