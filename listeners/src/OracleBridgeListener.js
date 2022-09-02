const ecc = require("eosjs-ecc");
const HyperionStreamClient = require("@eosrio/hyperion-stream-client").default;
const fetch = require("node-fetch");
const { BigNumber, ethers, utils } = require("ethers");

class OracleBridge {
    constructor(
        bridgeNativeContract,
        bridgeNativeName,
        bridgeNativePermission,
        bridgeEVMAddress,
        storageLengthKey,
        rpc,
        api,
        rpcEVM,
        signerKey
    ) {
        this.bridgeNativeContract = bridgeNativeContract;
        this.bridgeNativeName = bridgeNativeName;
        this.bridgeNativePermission = bridgeNativePermission;
        this.bridgeEVMAddress = bridgeEVMAddress;
        this.rpc = rpc;
        this.api = api;
        this.rpcEVM = rpcEVM;
        this.storageLengthKey = storageLengthKey;
        this.counter = 0;
    }
    async start() {
        try {
            // Get the scope for our EVM contrat in Mandel eosio.evm contract
            await this.startStream();
        } catch (e) {

        }

        // await this.doTableCheck();
        // TODO: maybe a 15 second setInterval against doTableCheck?  In case stream takes a crap?
        // Or: find a way to check stream health at set interval (better), if failed then doTableCheck & launch stream back
        // Or: disconnect and connect back every X minutes
    }

    async doTableCheck() {
        console.log(`Doing table check...`);
        const results = await this.rpc.get_table_rows({
            code: this.oracleContract,
            table: ACCOUNT_STATE_TABLE,
            scope: process.env.ACCOUNT_STATE_SCOPE,
            limit: 1000,
        });

        results.rows.forEach((row) => this.signRow(row));
        console.log(`Done doing table check!`);
    }
    async startStream() {
        this.streamClient = new HyperionStreamClient(
            process.env.HYPERION_ENDPOINT,
            {
                async: true,
                fetch: fetch,
            }
        );
        let getInfo = await this.rpc.get_info();
        let headBlock = getInfo.head_block_num;
        this.streamClient.onConnect = () => {
            this.streamClient.streamDeltas({
                code: 'eosio.evm',
                table: "accountstate",
                scope: process.env.ACCOUNT_STATE_SCOPE,
                payer: "",
                start_from: headBlock,
                read_until: 0,
            });
        };

        this.streamClient.onData = async (data, ack) => {
            if (data.content.present){
                let row = data.content.data;
                if(this.counter == 0){
                    this.api.transact({
                        actions: [{
                            account: this.bridgeNativeContract,
                            name: 'reqnotify',
                            authorization: [{ actor: this.bridgeNativeContract, permission: 'active' }],
                            data: {},
                        }]
                    }, {
                        blocksBehind: 3,
                        expireSeconds: 30,
                    }).then(result => {
                        console.log('\nCalled reqnotify()');
                    }).catch(e => {
                        console.log('\nCaught exception: ' + e);
                    });
                }
                this.counter++;
                if(this.counter == 11){
                    this.counter = 0;
                }
            }
            ack();
        };

        this.streamClient.connect(() => {
            console.log("Connected to Hyperion Stream !");
        });
    }
}

module.exports = OracleBridge;