# RNG Oracle Bridge

This repository includes three folders that need to be setup for the bridge to work: [EVM](https://github.com/telosnetwork/native-oracle-bridge/tree/main/evm), [Antelope](https://github.com/telosnetwork/native-oracle-bridge/tree/main/antelope), [listeners](https://github.com/telosnetwork/native-oracle-bridge/tree/main/listeners).

Follow the instructions below, in the right order, to deploy it

## REQUIREMENTS

This repository requires NodeJS 14+, NPM and pm2 installed as well as the cleos command line tool.

## INSTALL

Clone the repo with:

`git clone https://github.com/telosnetwork/rng-oracle-bridge`

## EVM

## ANTELOPE

## LISTENERS

## MAKE A REQUEST !

Deploy a contract that calls the newly deployed `RNGOracleBridge` contract's `request(uint callId, uint64 seed, uint min, uint max, uint callback_gas, address callback_address)` function, passing a value to cover fee and callback gas cost (see below). On the same contract, or in a new one, implement a `receiveRandom(uint callId, uint random)` callback function in order to receive the oracle's answer. 

You can refer to the [`RNGOracleConsumer`](https://github.com/telosnetwork/rng-oracle-bridge/blob/main/evm/contracts/RNGOracleConsumer.sol) EVM contract for an example.

### What is callback gas ? How do I know what value to pass ?

The `callback_gas` variable contains the gas you estimate will be needed to call your `receiveRandom()` callback function in your own smart contract (ie: 50000). This is the maximum amount of gas that will be spent by the bridge when calling your contract, if your callback implementation asks for more gas, the transaction will fail and the request will be deleted from storage.

You can query the TLOS value to pass in your `request()` function call by calling the `getCost(uint callback_gas)` function. 


