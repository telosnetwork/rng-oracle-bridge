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
    ACTION init(string evm_contract, string version, name admin, name oracle, string serialized_tx);

    //set the contract version
    ACTION setversion(string new_version);

    //set the bridge evm address
    ACTION setevmctc(string new_contract);

    //set new contract admin
    ACTION setadmin(name new_admin);

    //set new oracle contract
    ACTION setoracle(name new_oracle);

    //set new serialized transaction string
    ACTION setserialtx(string new_serialized_tx);

    //======================== Request actions ========================

    ACTION rmvrequest(name call_id);

    //======================== RNG Oracle actions ========================

    ACTION requestrand(name call_id, uint64_t seed, string caller);
    ACTION receiverand(name call_id, checksum256 number);

    //======================== contract tables ========================
    // Config
    TABLE configtable {
        string evm_contract;
        string serialized_tx;
        name oracle;
        name admin;
        string version;

        EOSLIB_SERIALIZE(configtable, (evm_contract)(serialized_tx)(oracle)(admin)(version))
    } config_row;
    typedef singleton<name("configtable"), configtable> config_singleton;
    config_singleton config;

    // Request
    TABLE request {
        name call_id;
        string caller;

        uint64_t primary_key() const { return call_id.value; }
        EOSLIB_SERIALIZE(request, (call_id)(caller))
    };
    typedef multi_index<name("requests"), request> requests_table;

};