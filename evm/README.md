# RNG Oracle Bridge :: EVM

_This part of the repository assumes that a `GasOracleBridge` instance is deployed on the network already. Its address is stored inside the deploy script._

This will deploy the `RNGOracleBridge` contract as well as a `RNGOracleConsumer` contract so that you can interact directly with the bridge. Users should write their own implementation of the consumer, using the same `receiveRandom(uint, uint) external` callback function. The `GasOracleBridge` contract is just included for test purposes and will not be deployed.

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

### CONFIGURE

Start the console with:

`npx hardhat console --network=testnet`

Use ethers to get an instance of the bridge and configure the EVM address of the anteloppe contract, like so

`await bridge.setOracleEVMContract('0xf8Da1....')`

_If your anteloppe contract doesn't have an EVM address yet use the `eosio.evm` contract `create(account, data)` action to create one._
