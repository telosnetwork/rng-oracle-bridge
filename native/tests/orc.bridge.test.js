const { loadConfig, Blockchain } = require("@klevoya/hydra");

const config = loadConfig("hydra.yml");

describe("orc.bridge", () => {
    let blockchain = new Blockchain(config);
    let bridge = blockchain.createAccount(`orc.bridge`);
    let oracle = blockchain.createAccount(`rng.oracle`);
    beforeAll(async () => {
        oracle.setContract(blockchain.contractTemplates[`rng.oracle`]);
        oracle.updateAuth(`active`, `owner`, {
            accounts: [
                {
                    permission: {
                        actor: oracle.accountName,
                        permission: `eosio.code`
                    },
                    weight: 1
                }
            ]
        });
        bridge.setContract(blockchain.contractTemplates[`orc.bridge`]);
        bridge.updateAuth(`active`, `owner`, {
            accounts: [
                {
                    permission: {
                        actor: bridge.accountName,
                        permission: `eosio.code`
                    },
                    weight: 1
                }
            ]
        });
    });
    beforeEach(async () => {
        bridge.resetTables();
        await bridge.contract.init({evm_contract: `0x9a469d1e668425907548228EA525A661FF3BFa2B`, version: `1`, admin: `orc.bridge`, oracle: 'rng.oracle', serialized_tx: '02156161fDaeae'});
    });
    describe(":: Settings", () => {
        it("can set a new serialized tx", async () => {
            const txTrace = await bridge.contract.setserialtx({new_serialized_tx: `0dF8170000000000002a189898E`});
        });

        it("can set a new oracle", async () => {
            const txTrace = await bridge.contract.setoracle({new_oracle: `rng.oracle`});
        });

        it("can set a new admin", async () => {
            const txTrace = await bridge.contract.setadmin({new_admin: `orc.bridge`});
        });

        it("can set a new version", async () => {
            const txTrace = await bridge.contract.setversion({ new_version: `2`});
        });

        it("can set a new evm contract", async () => {
            const txTrace = await bridge.contract.setevmctc({new_contract: `0x9a469d1e668425907548228EA525A661FF3BFa2B`});
        });
    });
    describe(":: Request", () => {
        it("can create a new request", async () => {
            const txTrace = await bridge.contract.requestrand({call_id: 1, seed: 199, caller: "0x9a469d1e668425907548228EA525A661FF3BFa2B", max: 64, min: 2});
            // get all print output of the transfer action and its created notifications
            const consoleOutput = txTrace.action_traces;

            // print it to test runner's stdout
            console.log(consoleOutput);
        });
        it("can remove that request", async () => {
            const txTrace = await bridge.contract.rmvrequest({call_id: 1});
        });
        it("can reply to that request", async () => {
            const txTrace = await bridge.contract.receiverand({call_id: 1, number: `01`});
        });
    });
});