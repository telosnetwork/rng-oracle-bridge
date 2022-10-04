# RNG Oracle Bridge

This repository contains the Telos RNG Oracle EVM <> Antelope Bridge.

The bridge requires the deployment of 3 components to work: the [EVM](https://github.com/telosnetwork/native-oracle-bridge/tree/main/evm) & [Antelope](https://github.com/telosnetwork/native-oracle-bridge/tree/main/antelope) components which are part of this repository and the Listener component that is part of our [Telos Oracle Scripts](https://github.com/telosnetwork/telos-oracle-scripts) repository. Follow the instructions below to deploy it.

## REQUIREMENTS

This repository requires NodeJS 14+, NPM and the cleos command line tool.

## INSTALL

Clone the repo with:

`git clone https://github.com/telosnetwork/rng-oracle-bridge`

## EVM

### RNGOracleBridge

The bridge contract, currently deployed at

**TESTNET :** TBD

**MAINNET :** TBD

### RNGOracleConsumer

An example implementation of a consumer with request & callback

### GasOracleBridge

The gas bridge to query current gas price. Only for test purposes, this contract will not be deployed.

For more, refer to the documentation inside the [`evm`](https://github.com/telosnetwork/rng-oracle-bridge/tree/main/evm) folder

## ANTELOPE

### rng.bridge.cpp

For more, refer to the documentation inside the [`antelope`](https://github.com/telosnetwork/rng-oracle-bridge/tree/main/antelope) folder

## LISTENERS

The listener for this bridge is located inside our [Telos Oracle Scripts](https://github.com/telosnetwork/telos-oracle-scripts) repository

Refer to the `config.yml.testnet.sample` file's **listeners > rng > bridge** section for an example.

Optionally, if you have been registered for it, you can look for the **listeners > rng > request** section to enable the RNG Request Listener that will sign incoming RNG Oracle requests on Antelope.

## HOW IT WORKS

The method we use is similar to Chainlink's Direct funding method. You must directly fund consuming contracts with TLOS tokens before they request randomness.

The `callback_gas` variable contains the gas units you estimate will be needed to call your `receiveRandom()` callback function in your own smart contract (ie: 50000). This is the maximum amount of gas that will be spent by the bridge when calling your contract.

You can query the TLOS value to pass in your `request()` function call by calling the `calculateRequestPrice(uint callback_gas)` public function. 

You can alternatively calculate that price by taking the gas price from the `GasOracleBridge` with `getPrice()`, multiply that price with your estimate gas units (ie: 50000) and add the fee from the `RNGOracleBridge` that you can query with `fee()`:

`Price = Gas Units * Gas Price + Bridge Fee`

### Note on transaction costs.

Because the consuming contract directly pays the TLOS for the request, the cost is calculated during the request and not during the callback when the randomness is fulfilled. Test your callback function to learn how to correctly estimate the callback gas limit.

If the gas limit is underestimated, the callback fails and the consuming contract is still charged for the work done to generate the requested random values.
If the gas limit is overestimated, the callback function will be executed but your contract is not refunded for the excess gas amount that you paid.
Make sure that your consuming contracts are funded with enough TLOS tokens to cover the transaction costs. If the consuming contract doesn't have enough TLOS tokens, your request will revert.

## REQUEST & RECEIVE DATA
 
![RNGOracleBridge](https://user-images.githubusercontent.com/5913758/193873078-b8e2e7ab-1f33-41d8-ac47-32ec81548c64.jpg)

## MAKE A REQUEST !

Deploy a contract that calls the `RNGOracleBridge` contract's `request()` function, passing a value to cover fee and callback gas cost (refer to the **callback gas** section further ahead).

```
interface IRNGOracleBridge {
    function request(uint callId, uint64 seed, uint callback_gas, address callback_address, uint number_count) external payable;
}

contract MyContract {
    IRNGOracleBridge bridge;

    constructor(address _bridge) {
        bridge = IRNGOracleBridge(_bridge);
    }
    
    function makeRequest(uint64 seed, uint callback_gas, uint count) external  payable {
        ... YOUR LOGIC TO SETUP THE CALLID, ETC...
        bridge.request{value: msg.value }(callId, seed, callback_gas, address(this), count);
    }
}
```

The request function takes in a **callId**, for you to keep track of the requests & handle answers later, a **seed** for the random generation that you can generate however you like as long as it fits into 64 bytes, the **callback_gas** unit (see below), the **callback_address** where to call your implementation of the callback function  and the **number_count** of requested numbers. 

On the same contract, or in a new one, implement a `receiveRandom(uint callId, uint[] numbers) external` callback function in order to receive the oracle's answer. 

You can refer to the [`RNGOracleConsumer`](https://github.com/telosnetwork/rng-oracle-bridge/blob/main/evm/contracts/RNGOracleConsumer.sol) EVM contract for an example.

### What is callback gas ? How do I know what value to pass ?


