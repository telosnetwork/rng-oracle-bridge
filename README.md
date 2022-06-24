# Native Oracle <> EVM Bridge

This repository includes two folders, the [EVM side](https://github.com/telosnetwork/native-oracle-bridge/tree/main/evm) of the oracle bridge and its [Native side](https://github.com/telosnetwork/native-oracle-bridge/tree/main/native).

Follow the instructions below, in the right order, to deploy it

## REQUIREMENTS

This repository requires NodeJS 14+

## 1. DEPLOY THE EVM CONTRACT

### INSTALL

First clone the repo with:

`git clone https://github.com/telosnetwork/native-oracle-bridge`

Then navigate to the `evm` directory and run the following command:

`npm install`

### TEST

Run the tests with:

`npx hardhat test`

### DEPLOY

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

### BUILD

Run the following command:

### TEST

Run the following command to start testing:

`npm test`

You can find and configure the tests in the `tests` directory. By default, they run on testnet but best practice would be using your local node with system contracts and our native oracles setup.

### DEPLOY

## 3. START THE NATIVE LISTENER

## 4. MAKE A REQUEST !

Deploy a contract that calls the newly deployed `NativeOracleBridge` EVM contract's `request()` function and implements a callback function in order to receive the oracle's answer (confer to the specific oracle documentation for arguments returned). Refer to the `NativeOracleBridgeTester` EVM contract for an example.

