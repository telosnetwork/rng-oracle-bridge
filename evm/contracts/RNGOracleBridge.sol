// SPDX-License-Identifier: MIT

pragma solidity ^0.8.0;

import "@openzeppelin/contracts/access/Ownable.sol";

interface IRNGOracleConsumer {
    function receiveRandom(uint, uint[] calldata) external;
}

interface IGasOracleBridge {
    function getPrice() external view returns(uint);
}

contract RNGOracleBridge is Ownable {
    event  Requested(address indexed requestor, uint call_id, uint number_count);
    event  Replied(address indexed requestor, uint call_id, uint[] random_numbers);

     struct Request {
        uint id;
        address caller_address;
        uint caller_id;
        uint requested_at;
        uint64 seed;
        uint8 number_count;
        uint callback_gas;
        address callback_address;
     }

     mapping (address => uint) public request_count;
     Request[] public requests; // Not using a mapping to be able to read from accountstate in Antelope (else we'd need to know the mapping key we want to lookup)
     uint count;

     uint public fee;
     uint public max_requests;
     uint public max_callback_gas;
     uint8 public max_number_count;
     address public oracle_evm_address;
     IGasOracleBridge public gas_oracle;

      constructor(uint _fee, uint _max_requests, uint8 _max_number_count, uint max_callback_gas, address _oracle_evm_address, IGasOracleBridge _gas_oracle) {
        gas_oracle = _gas_oracle;
        fee = _fee;
        max_requests = _max_requests;
        oracle_evm_address = _oracle_evm_address;
        max_number_count = _max_number_count;
        max_callback_gas = _max_callback_gas;
      }

     // SETTERS  ================================================================ >
     function setFee(uint _fee) external onlyOwner returns(bool) {
        fee = _fee;
        return true;
     }

     function setMaxNumberCount(uint _max_number_count) external onlyOwner returns(bool) {
        max_number_count = _max_number_count;
        return true;
     }

     function setMaxRequests(uint _max_requests) external onlyOwner returns(bool) {
        max_requests = _max_requests;
        return true;
     }

     function setOracleEVMAddress(address _oracle_evm_address) external onlyOwner returns(bool) {
        oracle_evm_address = _oracle_evm_address;
        return true;
     }

     function _calculateRequestPrice(uint callback_gas) internal view returns(uint) {
        uint gas_price =  gas_oracle.getPrice();
        require(gas_price > 0, "Could not retrieve gas price from Gas Oracle");
        return (fee + (callback_gas * gas_price));
     }

     function calculateRequestPrice(uint callback_gas) external view returns(uint) {
        return _calculateRequestPrice(callback_gas);
     }

     // REQUEST HANDLING ================================================================ >
     function request(uint callId, uint64 seed, uint callback_gas, address callback_address, uint8 number_count) external payable returns (bool) {
        // CHECK ARGUMENTS
        require(request_count[msg.sender] < max_requests, "Maximum requests reached, wait for replies or delete one using deleteRequestorRequest(address requestor, uint callId)");
        require(max_number_count >= number_count, "Requested number count is above max");
        require(max_callback_gas >= callback_gas, "Callback gas is above maximum");
        require(msg.value == _calculateRequestPrice(callback_gas), "Send enough TLOS to cover fee and callback gas, use getCost(callback_gas)");

        // CHECK EXISTS
        require(!this.requestExists(msg.sender, callId), "Call ID already exists");

        // SEND FEE TO ORACLE EVM ADDRESS SO IT CAN SEND THE RESPONSE BACK
        payable(oracle_evm_address).transfer(fee);

        request_count[msg.sender]++;

        // BUILD REQUEST
        requests.push(Request (count, msg.sender , callId, block.timestamp, seed, number_count, callback_gas, callback_address));

        count++;

        emit Requested(msg.sender, callId, number_count);

        return true;
     }

     function deleteRequestorRequest(address requestor, uint callId) external returns (bool) {
        for(uint i = 0; i < requests.length; i++){
            if(requests[i].caller_address == requestor && requests[i].caller_id == callId){
                require(msg.sender == requests[i].caller_address || msg.sender == owner(), "Only the requestor or owner can delete a request by requestor and callId");
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
                require(msg.sender == oracle_evm_address || msg.sender == owner(), "Only the bridge or owner can delete a request by ID");
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
     function reply(uint call_id, uint[] calldata numbers) external {
        require(msg.sender == oracle_evm_address, "Only the native oracle bridge EVM address can call this function");
        for(uint i = 0; i < requests.length; i++){
            if(requests[i].id == call_id){
                uint caller_id = requests[i].caller_id;
                address caller = requests[i].caller_address;
                uint gas = requests[i].callback_gas;
                address callback_address = requests[i].callback_address;
                requests[i] = requests[requests.length - 1];
                requests.pop();
                request_count[caller]--;
                if(gas > 0){
                    try IRNGOracleConsumer(callback_address).receiveRandom{gas: gas}(caller_id, numbers){} catch {}
                }
                emit Replied(caller, caller_id, numbers);
                return;
            }
        }
        revert("Request not found");
     }

     // UTIL ================================================================ >
     function requestExists(address requestor, uint call_id) external view returns (bool) {
        for(uint i = 0; i < requests.length; i++){
            if(requests[i].caller_id == call_id && requests[i].caller_address == requestor ){
                return true;
            }
        }
        return false;
     }
}