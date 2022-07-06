// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

interface IRNGOracleBridge {
    function request(uint callId, uint seed, uint min, uint max) external payable;
}

contract RNGOracleConsumer {
    IRNGOracleBridge bridge;
     struct Request {
        uint id;
        uint seed;
        uint min;
        uint max;
     }
     Request[] public requests;

    constructor(address _bridge) {
        bridge = IRNGOracleBridge(_bridge);
    }

    function makeRequest(uint seed, uint min, uint max) external  payable {
        require(msg.value > 0, "Request needs fee passed");
        uint callId = 0;
        if(requests.length > 0){
            callId = requests[requests.length - 1].id + 1;
        }
        requests.push(Request(callId, seed, min, max));
        return bridge.request{value: msg.value }(callId, seed, min, max);
    }

    function receiveRandom(uint callId, uint random) external {
         for(uint i = 0; i < requests.length;i++){
            if(requests[i].id == callId){
                requests[i] = requests[requests.length - 1];
                requests.pop();
            }
         }
    }
}