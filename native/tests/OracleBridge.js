const { loadConfig, Blockchain } = require("hydra");

const config = loadConfig("hydra.yml");

describe("orc.bridge", () => {
    let blockchain = new Blockchain(config);
    let tester = blockchain.createAccount(`orc.bridge`);
    beforeAll(async () => {
        tester.setContract(blockchain.contractTemplates[`orc.bridge`]);
        tester.updateAuth(`active`, `owner`, {
            accounts: [
                {
                    permission: {
                        actor: tester.accountName,
                        permission: `eosio.code`
                    },
                    weight: 1
                }
            ]
        });
    });
    beforeEach(async () => {
        tester.resetTables();
    });

    it("", async () => {

    });
}