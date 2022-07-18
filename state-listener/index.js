require('dotenv').config()
const OracleBridgeListener = require('./src/OracleBridgeListener')
const eosjs = require('eosjs')
const JsSignatureProvider = require('eosjs/dist/eosjs-jssig').JsSignatureProvider
const JsonRpc = eosjs.JsonRpc
const Api = eosjs.Api
const fetch = require('node-fetch')
const util = require('util')
const { BigNumber, ethers, utils } = require("ethers");

const signatureProvider = new JsSignatureProvider([process.env.BRIDGE_KEY]);
const rpc = new JsonRpc(process.env.RPC_ENDPOINT, { fetch })

const api = new Api({
    rpc,
    signatureProvider,
    textDecoder: new util.TextDecoder(),
    textEncoder: new util.TextEncoder()
});

const rpcEVM = ethers.getDefaultProvider(process.env.RPC_EVM_ENDPOINT);

const listener = new OracleBridgeListener(process.env.BRIDGE_CONTRACT, process.env.BRIDGE_NAME, process.env.BRIDGE_PERMISSION, process.env.BRIDGE_EVM_CONTRACT, process.env.STORAGE_LENGTH_KEY, rpc, api, rpcEVM,  process.env.BRIDGE_SIGNER_KEY)
listener.start()