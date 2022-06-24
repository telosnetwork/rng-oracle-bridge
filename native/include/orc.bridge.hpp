// @author Thomas Cuvillier
// @contract OracleBridge
// @version v0.1.0


#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>

using namespace std;
using namespace eosio;

class [[eosio::contract("orc.bridge")]] bridge : public contract {

public:
    bridge(name self, name code, datastream<const char*> ds) : contract(self, code, ds), config(self, self.value) {};
    ~bridge() {};

    //======================== admin actions ========================

    // intialize the contract
    ACTION init(string evm_contract, string version, name initial_admin);

    //set the contract version
    ACTION setversion(string new_version);

    //set the bridge evm address
    ACTION setevmctc(string new_contract);

    //set new contract admin
    ACTION setadmin(name new_admin);

    //======================== core actions ========================

    ACTION request(uint64_t caller_id);
    ACTION receive(uint64_t caller_id);

    //======================== oracle type actions ========================

    // add a new oracle type
    ACTION upsertorctype(name oracle_type);

    // remove an oracle type
    ACTION rmvorctype(name oracle_type);

    //======================== oracle actions ========================

    // add a new oracle
    ACTION upsertoracle(name oracle_name, string oracle_type);

    // remove an oracle
    ACTION rmvoracle(name oracle_name);


    //======================== contract tables ========================
    //config singleton
    //scope: self
    TABLE configtable {
        string evm_contract;
        string version;
        name admin;
        EOSLIB_SERIALIZE(configtable, (evm_contract)(version)(admin))
    } config_row;
    typedef singleton<name("configtable"), configtable> config_singleton;
    config_singleton config;

    //oracles
    //scope: self
    TABLE oracle {
        name oracle_name;
        string oracle_type;
        uint64_t primary_key() const { return oracle_name.value; }
        EOSLIB_SERIALIZE(oracle, (oracle_name)(oracle_type))
    };
    typedef multi_index<name("oracles"), oracle> oracles_table;

    //oracles types
    //scope: self
    TABLE oracle_type {
        name type_name;
        uint64_t primary_key() const { return type_name.value; }
        EOSLIB_SERIALIZE(oracle_type, (type_name))
    };
    typedef multi_index<name("oracletypes"), oracle_type> oracles_types_table;
};