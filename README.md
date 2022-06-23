# Native Oracle <> EVM Bridge

This repository includes two folders, the [EVM side](https://github.com/telosnetwork/native-oracle-bridge/tree/main/evm) of the oracle bridge and its [Native side](https://github.com/telosnetwork/native-oracle-bridge/tree/main/native).

Follow the instructions below, in the right order, to deploy it

## REQUIREMENTS

This repository requires NodeJS 14+

## 1. DEPLOY EVM

### INSTALL

Install the EVM repository by navigating to the evm directory with

`cd evm`

Then run the following command:

`npm install`

### TEST
### DEPLOY

⚠️ **Save that address to configure the native contract next**

## 2. DEPLOY NATIVE

### INSTALL
### TEST
### DEPLOY

## 3. MAKE A REQUEST !

Deploy a contract that calls the newly deployed `NativeOracleBridge` EVM contract's `request()` function and implements a callback function in order to receive the oracle's answer (confer to the specific oracle documentation for arguments returned). Refer to the `NativeOracleBridgeTester` EVM contract for an example.

