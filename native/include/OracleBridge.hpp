// @author Thomas Cuvillier
// @contract OracleBridge
// @version v0.1.0


#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/crypto.hpp>

using namespace std;
using namespace eosio;

CONTRACT OracleBridge : public contract {

public:
    OracleBridge(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {};
    ~OracleBridge() {};

    //======================== admin actions ========================

    // intialize the contract
    ACTION init(string evm_contract);

    //set the contract version
    ACTION setversion(string new_version);

    //set the bridge evm address
    ACTION setevmcontract(string evm_contract);

    //set new contract admin
    ACTION setadmin(name new_admin);

    //======================== request actions ========================

    // Request a random value w/ native caller
    ACTION requestrand(uint64_t caller_id, uint64_t seed);

    //======================== oracle actions ========================

    // add a new oracle
    ACTION upsertoracle(name oracle_name);

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

        uint64_t primary_key() const { return oracle_name.value; }
        EOSLIB_SERIALIZE(oracle, (oracle_name))
    };
    typedef multi_index<name("oracles"), oracle> oracles_table;
};