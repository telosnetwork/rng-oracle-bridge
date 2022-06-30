// @author Thomas Cuvillier
// @contract OracleBridge
// @version v0.1.0

#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio.evm/eosio.evm.hpp>

using namespace std;
using namespace eosio;
using namespace eosio_evm;

class [[eosio::contract("orc.bridge")]] bridge : public contract {

public:
    bridge(name self, name code, datastream<const char*> ds) : contract(self, code, ds), config(self, self.value) {};
    ~bridge() {};

    //======================== Admin actions ========================

    // intialize the contract
    ACTION init(eosio::checksum160 evm_contract, string version, name admin, name oracle, string function_signature);

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

    //======================== Request actions ========================

    ACTION rmvrequest(uint64_t request_id);

    //======================== RNG Oracle actions ========================

    ACTION requestrand(bigint::checksum256 call_id, bigint::checksum256 seed, eosio::checksum160 caller, bigint::checksum256 max, bigint::checksum256 min);
    ACTION receiverand(uint64_t caller_id, checksum256 random);

    //======================== eosio.evm tables =======================
    struct [[eosio::table, eosio::contract("eosio.evm")]] Account {
        uint64_t index;
        eosio::checksum160 address;
        eosio::name account;
        uint64_t nonce;
        std::vector<uint8_t> code;
        eosio::checksum256 balance;

        Account () = default;
        Account (uint256_t _address): address(eosio_evm::addressToChecksum160(_address)) {}
        uint64_t primary_key() const { return index; };

        uint64_t get_account_value() const { return account.value; };
        uint256_t get_address() const { return eosio_evm::checksum160ToAddress(address); };

        // TODO: make this work if we need to lookup EVM balances, which we don't for this contract
        //uint256_t get_balance() const { return balance; };
        //bool is_empty() const { return nonce == 0 && balance == 0 && code.size() == 0; };
        uint64_t get_nonce() const { return nonce; };
        std::vector<uint8_t> get_code() const { return code; };

        eosio::checksum256 by_address() const { return eosio_evm::pad160(address); };

        EOSLIB_SERIALIZE(Account, (index)(address)(account)(nonce)(code)(balance));
    };

    typedef eosio::multi_index<"account"_n, Account,
            eosio::indexed_by<eosio::name("byaddress"), eosio::const_mem_fun<Account, eosio::checksum256, &Account::by_address>>,
    eosio::indexed_by<eosio::name("byaccount"), eosio::const_mem_fun<Account, uint64_t, &Account::get_account_value>>
    > account_table;

    //======================== Contract tables ========================
    // Config
    TABLE configtable {
        eosio::checksum160 evm_contract;
        string function_signature;
        name oracle;
        name admin;
        string version;
        EOSLIB_SERIALIZE(configtable, (evm_contract)(function_signature)(oracle)(admin)(version));
    } config_row;
    typedef singleton<name("configtable"), configtable> config_singleton;
    config_singleton config;

    // Request
    TABLE request {
        uint64_t request_id;
        bigint::checksum256 call_id;
        eosio::checksum160 caller;
        bigint::checksum256 max;
        bigint::checksum256 min;

        uint64_t primary_key() const { return request_id; };
        uint256_t get_min() const { return min; };
        uint256_t get_max() const { return max; };
        uint256_t get_call_id() const { return call_id; };

        EOSLIB_SERIALIZE(request, (request_id)(call_id)(caller)(max)(min));
    };
    typedef multi_index<name("requests"), request> requests_table;

};