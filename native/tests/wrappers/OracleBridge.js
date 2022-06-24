const { EOSJS } = require('telos-native-testing');
const Contract = require('telos-native-testing/wrappers/Contract');
const { EOSRpc, EOSApi } = EOSJS.getInstance();

class OracleBridge extends Contract {
    constructor(...args) {
        super(...args);
    }

    async init(evm_contract, version, admin) {
        return await this.sendAction("init", {
            evm_contract: evm_contract,
            version: version,
            initial_admin: admin,
        })
    }

    // ADMIN =============================================================>
    async getEVMContract() {
        let config = await this.getConfig();
        if(config){
            return config.evm_contract;
        }
        return false;
    }

    async getAdmin() {
        let config = await this.getConfig();
        if(config){
            return config.admin;
        }
        return false;
    }

    async getVersion() {
        let config = await this.getConfig();
        if(config){
            return config.version;
        }
        return false;
    }
    async setVersion(new_version) {
        return await this.sendAction("setversion", {
            new_version: new_version,
        })
    }
    async setAdmin(new_admin) {
        return await this.sendAction("setadmin", {
            new_admin: new_admin,
        })
    }
    async setEVMContract(new_contract) {
        return await this.sendAction("setevmctc", {
            new_contract: new_contract,
        })
    }

    // ORACLE TYPES ==============================================================>
    async addOracleType(oracle_type) {
        return await this.sendAction("upsertorctype", {
            oracle_type: oracle_type,
        })
    }
    async removeOracleType(oracle_type) {
        return await this.sendAction("rmvorctype", {
            oracle_type: oracle_type,
        })
    }

    // ORACLES ==============================================================>
    async addOracle(oracle_type, oracle_name) {
        return await this.sendAction("upsertoracle", {
            oracle_name: oracle_name,
            oracle_type: oracle_type,
        })
    }
    async removeOracle(oracle_name) {
        return await this.sendAction("rmvoracle", {
            oracle_name: oracle_name,
        })
    }

    // CORE ==============================================================>
    async request(callId, data) {

    }

    async reply(callId, data) {

    }

    // UTILS =============================================================>

    async getConfig() {
        var config = await this.getTableRows('configtable', 1);
        if(config.rows){
            return config.rows[0];
        }
        return false;
    }

    async existsOracleType(oracle_type) {
        var types = await this.getTableRows('oracletypes', 100)
        for(var i =0; i < types.rows.length;i++){
            if(types.rows[i].type_name == oracle_type){
                return true;
            }
        }
        return false;
    }

    async existsOracle(oracle){
        var oracles = await this.getTableRows('oracles', 1000)
        for(var i =0; i < oracles.rows.length;i++){
            if(oracles.rows[i].oracle_name == oracle){
                return true;
            }
        }
        return false;
    }
}

module.exports = OracleBridge;