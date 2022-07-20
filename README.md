# RNG Native Oracle <> EVM Bridge

This repository includes two folders, the [EVM side](https://github.com/telosnetwork/native-oracle-bridge/tree/main/evm) of the rng oracle bridge and its [Native side](https://github.com/telosnetwork/native-oracle-bridge/tree/main/native).

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

## 2. DEPLOY THE NATIVE CONTRACT

### INSTALL

Navigate to the `native` directory and run the following command:

`npm install`

### CONFIGURE

Edit the following values in the `env` file:

### BUILD

Run the following command:

`mkdir build & cd build`
`cmake.. & make -j4`

### TEST

Run the following command to start testing with the Hydra testing suite:

`npm test`

You can find the tests in the `tests` directory.

### DEPLOY

You can deploy the contract using cleos with the following command:

`cleos --url http://testnet.telos.net set contract [CONTRACT ACCOUNT] ./build ./rng.bridge.cpp ./rng.bridge.hpp`

## 3. START THE NATIVE LISTENER

From the `state-listener` directory, use `pm2` to start the listener script:

`pm2 index.js`

## 4. MAKE A REQUEST !

Deploy a contract that calls the newly deployed `RNGOracleBridge` EVM contract's `function request(uint callId, uint64 seed, uint min, uint max)` function and implements a `receiveRandom(uint callId, uint random)` callback function in order to receive the oracle's answer. Refer to the `RNGOracleConsumer` EVM contract for an example.

