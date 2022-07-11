require("@nomiclabs/hardhat-waffle");
require("solidity-coverage");
require('hardhat-deploy');

/**
 * @type import('hardhat/config').HardhatUserConfig
 */
module.exports = {
    solidity: {
        compilers: [
            {
                version: "0.8.4",
            },
            {
                version: "0.4.18",
            },
        ],
    },
    namedAccounts: {
        deployer: 'privatekey://0x8dd3ec4846cecac347a830b758bf7e438c4d9b36a396b189610c90b57a70163d'
    },
    networks: {
        local: {
            chainId: 41,
            url: "http://localhost:7000/evm",
            accounts: ['0x8dd3ec4846cecac347a830b758bf7e438c4d9b36a396b189610c90b57a70163d'],
        }
    },
};