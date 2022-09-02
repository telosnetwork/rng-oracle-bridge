# RNG Oracle Bridge

This repository includes three folders that need to be setup for the bridge to work: [EVM](https://github.com/telosnetwork/native-oracle-bridge/tree/main/evm), [Antelope](https://github.com/telosnetwork/native-oracle-bridge/tree/main/antelope), [listeners](https://github.com/telosnetwork/native-oracle-bridge/tree/main/listeners).

Follow the instructions below, in the right order, to deploy it

## REQUIREMENTS

This repository requires NodeJS 14+ and pm2 installed

## INSTALL

Clone the repo with:

`git clone https://github.com/telosnetwork/rng-oracle-bridge`

## 1. DEPLOY THE EVM CONTRACT

### INSTALL

Navigate to the `evm` directory and run the following command:

`npm install`

### BUILD & TEST

Run the tests with:

`npx hardhat test`

### BUILD & DEPLOY

Deploy the contract with:

`npx hardhat deploy --network=testnet`

⚠️ **Save the address output in console to configure the native contract next**

### VERIFY

Verify the contract  with:

`npx hardhat sourcify --network=testnet`

## 2. DEPLOY THE ANTELOPE CONTRACT

### INSTALL

Navigate to the `antelope` directory and run the following command:

`npm install`

### CONFIGURE

Edit the following values in the `env` file:

### BUILD

Run the following command:

`bash build.sh`

Files will be saved to the `build` directory

### TEST

Run the following command to start testing with the Hydra testing suite:

`npm test`

You can find the tests in the `tests` directory.

### DEPLOY

You can deploy the contract using cleos with the following command:

`cleos --url http://testnet.telos.net set contract [CONTRACT ACCOUNT] ./build ./rng.bridge.cpp ./rng.bridge.hpp`

## 3. START THE LISTENER

### CONFIGURE

Edit the following values in the `env` file:

### RUN

From the `listeners` directory, use `pm2` to start the listener script:

`pm2 index.js`

## 4. MAKE A REQUEST !

Deploy a contract that calls the newly deployed `RNGOracleBridge` contract's `request(uint callId, uint64 seed, uint min, uint max, uint callback_gas)` function, passing a value to cover fee and callback gas cost, and implements a `receiveRandom(uint callId, uint random)` callback function in order to receive the oracle's answer. Refer to the `RNGOracleConsumer` EVM contract for an example.

### What is callback gas ? How do I know what value to pass ?

The `callback_gas` variable contains the gas you estimate will be needed to call your `receiveRandom()` callback function in your own smart contract (ie: 21000). This is the maximum amount of gas that will be spent by the bridge when calling your contract. If your callback implementation asks for more gas, the transaction will fail.

You can query the TLOS value to pass in your `request()` function call by calling the `getCost(uint callback_gas)` function. 


