// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

import "@openzeppelin/contracts/access/Ownable.sol";

interface IRNGOracleConsumer {
    function receiveRandom(uint, uint) external;
}

interface IGasOracleBridge {
    function getPrice() external view returns(uint);
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
        uint callback_gas;
        address callback_address;
     }
     mapping (address => uint) public request_count;
     Request[] public requests; // Not using a mapping to be able to read from accountstate in native (else we need to know the mapping key we want to lookup)
     uint public fee;
     uint count;
     uint public maxRequests;
     address public oracle_evm_contract;
     IGasOracleBridge public gasOracle;

      constructor(uint _fee, uint _maxRequests, address _oracle_evm_contract, IGasOracleBridge _gas_oracle) {
        gasOracle = _gas_oracle;
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

     function _getCost(uint callback_gas) internal view returns(uint) {
        uint gasPrice =  gasOracle.getPrice();
        require(gasPrice > 0, "Could not retrieve gas price");
        return (fee + (callback_gas * gasPrice));
     }

     function getCost(uint callback_gas) external view returns(uint) {
        return _getCost(callback_gas);
     }

     // REQUEST HANDLING ================================================================ >
     function request(uint callId, uint64 seed, uint min, uint max, uint callback_gas, address callback_address) external payable returns (bool) {
        require(msg.value == _getCost(callback_gas), "Send enough TLOS to cover fee and callback gas, use getCost(callback_gas)");
        require(max >= min, "Max cannot be less than min");
        require(request_count[msg.sender] < maxRequests, "Maximum requests reached, wait for replies or delete one using deleteRequestorRequest(address requestor, uint callId)");

        // CHECK EXISTS
        require(!this.requestExists(msg.sender, callId), "Call ID already exists");

        // SEND HALF OF FEE TO ORACLE EVM ADDRESS SO IT CAN SEND THE RESPONSE BACK, KEEP THE REST TO SEND THAT RESPONSE BACK TO CALLBACK
        payable(oracle_evm_contract).transfer(fee / 2);

        request_count[msg.sender]++;

        // BUILD REQUEST
        requests.push(Request (count, msg.sender , callId, block.timestamp, seed, min, max, callback_gas, callback_address));

        count++;

        emit Requested(msg.sender, callId);

        return true;
     }

     function deleteRequestorRequest(address requestor, uint callId) external returns (bool) {
        for(uint i = 0; i < requests.length; i++){
            if(requests[i].caller_address == requestor && requests[i].caller_id == callId){
                require(msg.sender == requests[i].caller_address, "Only the requestor can delete a request by requestor and callId");
                address caller = requests[i].caller_address;
                requests[i] = requests[requests.length - 1];
                requests.pop();
                request_count[caller]--;
                return true;
            }
        }
        revert("Request not found");
     }

     function deleteRequest(uint id) external returns (bool) {
        for(uint i = 0; i < requests.length; i++){
            if(requests[i].id == id){
                require(msg.sender == oracle_evm_contract || msg.sender == owner(), "Only the bridge or owner can delete a request by ID");
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
                uint gas = requests[i].callback_gas;
                address callback_address = requests[i].callback_address;
                requests[i] = requests[requests.length - 1];
                requests.pop();
                request_count[caller]--;
                if(gas > 0){
                    try IRNGOracleConsumer(callback_address).receiveRandom{gas: gas}(caller_id, random){}catch{}
                }
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