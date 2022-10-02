# RNG Oracle Bridge :: Antelope

### INSTALL

Navigate to the `antelope` directory and run the following command:

`npm install`

### BUILD

Run the following command:

`bash build.sh`

### TEST

Run the following command to start testing with the Hydra testing suite:

`npm test`

You can find the tests in the `tests` directory.

### DEPLOY

You can deploy the contract using cleos with the following command:

`bash deploy.sh`

It will default to testnet, pass mainnet as argument to deploy to mainnet

`bash deploy.sh mainnet`

### CONFIGURE

After deploying the EVM contract, configure its address like so:

`cleos --url http://testnet.telos.net push action YOUR_CONTRACT_ACCOUNT setevmctc '{"new_contract": "f7d3A11...."}' -p YOUR_CONTRACT_ACCOUNT`

Note that the address is the RNGOracleBridge's EVM address without the 0x prefix.
