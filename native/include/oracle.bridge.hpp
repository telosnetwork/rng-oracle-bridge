// @author Thomas Cuvillier
// @contract OracleBridge
// @version v0.1.0


#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/crypto.hpp>

using namespace std;
using namespace eosio;

class [[eosio::contract("oracle.bridge")]] bridge : public contract {

public:
    bridge(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {};
    ~bridge() {};

    //======================== admin actions ========================

    // intialize the contract
    ACTION init(string evm_contract, string version, name initial_admin);

    //set the contract version
    ACTION setversion(string new_version);

    //set the bridge evm address
    ACTION setevmcontract(string evm_contract);

    //set new contract admin
    ACTION setadmin(name new_admin);

    //======================== request actions ========================

    // Request a random value w/ native caller
    ACTION requestrand(uint64_t caller_id, uint64_t seed);

    //======================== oracle type actions ========================

    // add a new oracle type
    ACTION upsertoracletype(string oracle_type);

    // remove an oracle type
    ACTION rmvoracletype(string oracle_type);

    //======================== oracle actions ========================

    // add a new oracle
    ACTION upsertoracle(name oracle_name, string oracle_type);

    // remove an oracle
    ACTION rmvoracle(name oracle_name);


    //======================== contract tables ========================
    //config singleton
    //scope: self
    TABLE config {
        string evm_contract;
        string version;
        name admin;

        EOSLIB_SERIALIZE(config, (evm_contract))
    };
    typedef singleton<name("config"), config> config_singleton;

    //oracles
    //scope: self
    TABLE oracle {
        name oracle_name;
        string oracle_type;
        uint64_t primary_key() const { return oracle_name.value; }
        EOSLIB_SERIALIZE(oracle, (oracle_name))
    };
    typedef multi_index<name("oracles"), oracle> oracles_table;

    //oracles types
    //scope: self
    TABLE oracle_type {
        string type_id;
        uint64_t primary_key() const { return type_id.value; }
        EOSLIB_SERIALIZE(oracle, (type_id))
    };
    typedef multi_index<name("oracles_types"), oracle> oracles_types_table;
};