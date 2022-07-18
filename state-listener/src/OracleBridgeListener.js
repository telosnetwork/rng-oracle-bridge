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
    }
    async start() {
        await this.startStream();
        //await this.doTableCheck();
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
                let length =  parseInt(ethers.utils.formatEther(await this.rpcEVM.getStorageAt(this.bridgeEVMAddress, this.storageLengthKey)));
                // check if new requests (not delete)
                // if new request send notification to bridge contract to read accountstate (pass a param ?? how to keep track of what was sent or not ??)
            }
            ack();
        };

        this.streamClient.connect(() => {
            console.log("Connected to Hyperion Stream !");
        });
    }
}

module.exports = OracleBridge;