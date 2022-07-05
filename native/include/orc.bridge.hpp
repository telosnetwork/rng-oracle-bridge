// @author Thomas Cuvillier
// @contract OracleBridge
// @version v0.1.0

#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio.evm/eosio.evm.hpp>

#define EVM_CONTRACT name("eosio.evm")

using namespace std;
using namespace eosio;
using namespace eosio_evm;


class [[eosio::contract("orc.bridge")]] bridge : public contract {

public:
    bridge(name self, name code, datastream<const char*> ds) : contract(self, code, ds), config(self, self.value) {};
    ~bridge() {};

    //======================== Admin actions ========================

    // intialize the contract
    ACTION init(eosio::checksum160 evm_contract, string version, name admin, name oracle, string function_signature, bigint::checksum256 gas_limit);

    //set the contract version
    ACTION setversion(string new_version);

    //set the bridge evm address
    ACTION setevmctc(eosio::checksum160 new_contract);

    //set new contract admin
    ACTION setadmin(name new_admin);

    //set new oracle contract
    ACTION setoracle(name new_oracle);

    //set new function signature string
    ACTION setfnsig(string new_function_signature);

    //set new gas limit
    ACTION setgaslimit(bigint::checksum256 new_gas_limit);

    //======================== Request actions ========================

    ACTION rmvrequest(uint64_t request_id);

    //======================== RNG Oracle actions ========================

    ACTION requestrand(bigint::checksum256 call_id, bigint::checksum256 seed, eosio::checksum160 caller, bigint::checksum256 max, bigint::checksum256 min);
    ACTION receiverand(uint64_t caller_id, checksum256 random);

    //======================== eosio.evm tables =======================
    typedef eosio::multi_index<"account"_n, Account,
        eosio::indexed_by<eosio::name("byaddress"), eosio::const_mem_fun<Account, eosio::checksum256, &Account::by_address>>,
        eosio::indexed_by<eosio::name("byaccount"), eosio::const_mem_fun<Account, uint64_t, &Account::get_account_value>>
    > account_table;

    //======================== Tables ========================
    // Config
    TABLE configtable {
        eosio::checksum160 evm_contract;
        string function_signature;
        bigint::checksum256 gas_limit;
        name oracle;
        name admin;
        string version;

        EOSLIB_SERIALIZE(configtable, (evm_contract)(function_signature)(gas_limit)(oracle)(admin)(version));
    } config_row;

    typedef singleton<"configtable"_n, configtable> config_singleton;
    config_singleton config;

    // Request
    TABLE request {
        uint64_t request_id;
        bigint::checksum256 call_id;
        eosio::checksum160 caller;
        bigint::checksum256 max;
        bigint::checksum256 min;

        uint64_t primary_key() const { return request_id; };
        uint256_t getMin() const { return min; };
        uint256_t getMax() const { return max; };
        uint256_t getCallId() const { return call_id; };

        EOSLIB_SERIALIZE(request, (request_id)(call_id)(caller)(max)(min));
    };
    typedef multi_index<name("requests"), request> requests_table;

    //===================== Test actions ===================================
    #if (TESTING == true)
        ACTION clearall()
        {
          require_auth(get_self());
          requests_table requests(get_self(), get_self().value);
          auto itr = requests.end();
          while (requests.begin() != itr)
          {
            itr = requests.erase(--itr);
          }
          if (config.exists())
          {
            config.remove();
          }
        }
    #endif
};