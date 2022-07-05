// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

interface INativeOracleBridge {
    function request(uint callId, function(uint callId, string[] memory) external callback, address payable oracle, string[] memory data) external payable;
}

contract NativeOracleConsumer {

    INativeOracleBridge bridge;
     struct Request {
        uint id;
        string[] data;
     }
     Request[] public requests;

    constructor(address _bridge) {
        bridge = INativeOracleBridge(_bridge);
    }

    function makeRequest(address payable oracle, string[] memory data) external  payable {
        require(msg.value > 0, "Request needs fee passed");
        uint callId = 0;
        if(requests.length > 0){
            callId = requests[requests.length - 1].id + 1;
        }
        requests.push(Request(callId, data));
        return bridge.request{value: msg.value }(callId, this.printResponse, oracle, data);
    }

    function printResponse(uint callId, string[] memory data) external {
    }
}