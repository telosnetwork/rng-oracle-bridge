# RNG Oracle Bridge

This repository contains the Telos RNG Oracle EVM <> Antelope Bridge.

The bridge requires the deployment of 3 components to work: the [EVM](https://github.com/telosnetwork/native-oracle-bridge/tree/main/evm) & [Antelope](https://github.com/telosnetwork/native-oracle-bridge/tree/main/antelope) components which are part of this repository and the Listener component that is part of our [Telos Oracle Scripts](https://github.com/telosnetwork/telos-oracle-scripts) repository. Follow the instructions below to deploy it.

## REQUIREMENTS

This repository requires NodeJS 14+, NPM and the cleos command line tool.

## INSTALL

Clone the repo with:

`git clone https://github.com/telosnetwork/rng-oracle-bridge`

## EVM

### RNGOracleBridge.sol

The bridge's EVM contract, it receives requests and send the response back to the consumer's callback, it is currently deployed at

**TESTNET :** TBD

**MAINNET :** TBD

### RNGOracleConsumer.sol

An example implementation of a consumer with request & callback

### GasOracleBridge.sol

The gas bridge to query current gas price. Only for test purposes, this contract will not be deployed.

_To learn how to deploy those contracts, refer to the documentation inside the [`evm`](https://github.com/telosnetwork/rng-oracle-bridge/tree/main/evm) folder_

## ANTELOPE

### rng.bridge.cpp

The bridge's Antelope contract, it gets notified of new requests on the EVM contract by the listeners, checks out the request and imports it, request the random numbers from the RNG Oracle and passes the answer to the EVM contract. It is currently deployed at

**TESTNET :** rng.bridge

**MAINNET :** TBD

_To learn how to deploy it, refer to the documentation inside the [`antelope`](https://github.com/telosnetwork/rng-oracle-bridge/tree/main/antelope) folder_

## LISTENERS

The listener for this bridge is located inside our [Telos Oracle Scripts](https://github.com/telosnetwork/telos-oracle-scripts) repository

Refer to the `config.yml.testnet.sample` file's **listeners > rng > bridge** section for an example.

Optionally, if you have been registered for it, you can look for the **listeners > rng > request** section to enable the RNG Request Listener that will sign incoming RNG Oracle requests on Antelope.

## USING THE BRIDGE

Follow the documentation available in Telos docs [here]() to learn how to request random numbers using our RNG Oracle Bridge.
