// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

interface INativeOracleBridge {
    function request(string memory callId, function(string[] memory) external callback, address payable oracle, string[] memory data) external payable;
}

contract NativeOracleBridgeTester {

    INativeOracleBridge bridge;

    constructor(address _bridge) {
        bridge = INativeOracleBridge(_bridge);
    }

    function makeRequest(string memory callId, address payable oracle, string[] memory data) external  payable {
        require(msg.value > 0, "Request needs fee passed");
        return bridge.request{value: msg.value }(callId, this.printResponse, oracle, data);
    }

    function printResponse(string[] memory data) external {
    }
}