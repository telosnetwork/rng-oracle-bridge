// @author Thomas Cuvillier
// @contract OracleBridge
// @version v0.1.0

#include <vector>

// EOSIO
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>

// EXTERNAL
#include <intx/base.hpp>
#include <rlp/rlp.hpp>
#include <ecc/uECC.c>
#include <keccak256/k.c>
#include <boost/multiprecision/cpp_int.hpp>

// TELOS EVM
#include <util.hpp>
#include <datastream.hpp>

#define EVM_SYSTEM_CONTRACT name("eosio.evm")

using namespace std;
using namespace eosio;
using namespace orc_bridge;

namespace orc_bridge
{
    class [[eosio::contract("rngorc.brdg")]] OracleBridge : public contract {
        public:
            using contract::contract;
            OracleBridge(name self, name code, datastream<const char*> ds) : contract(self, code, ds), config_bridge(self, self.value) { };
            ~OracleBridge() {};

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
              struct [[eosio::table, eosio::contract("eosio.evm")]] Account {
                uint64_t index;
                eosio::checksum160 address;
                eosio::name account;
                uint64_t nonce;
                std::vector<uint8_t> code;
                bigint::checksum256 balance;

                Account () = default;
                Account (uint256_t _address): address(addressToChecksum160(_address)) {}
                uint64_t primary_key() const { return index; };

                uint64_t get_account_value() const { return account.value; };
                uint256_t get_address() const { return checksum160ToAddress(address); };
                uint256_t get_balance() const { return balance; };
                uint64_t get_nonce() const { return nonce; };
                std::vector<uint8_t> get_code() const { return code; };
                bool is_empty() const { return nonce == 0 && balance == 0 && code.size() == 0; };

                eosio::checksum256 by_address() const { return pad160(address); };

                EOSLIB_SERIALIZE(Account, (index)(address)(account)(nonce)(code)(balance));
              };
            typedef eosio::multi_index<"account"_n, Account,
                eosio::indexed_by<eosio::name("byaddress"), eosio::const_mem_fun<Account, eosio::checksum256, &Account::by_address>>,
                eosio::indexed_by<eosio::name("byaccount"), eosio::const_mem_fun<Account, uint64_t, &Account::get_account_value>>
            > account_table;

              struct [[eosio::table, eosio::contract("eosio.evm")]] config {
                uint32_t trx_index = 0;
                uint32_t last_block = 0;
                bigint::checksum256 gas_used_block = 0;
                bigint::checksum256 gas_price = 1;

                EOSLIB_SERIALIZE(config, (trx_index)(last_block)(gas_used_block)(gas_price));
              };
              typedef eosio::singleton<"config"_n, config> config_singleton_evm;

            //======================== Tables ========================
            // Config
            TABLE bridgeconfig {
                eosio::checksum160 evm_contract;
                string function_signature;
                bigint::checksum256 gas_limit;
                name oracle;
                name admin;
                string version;

                EOSLIB_SERIALIZE(bridgeconfig, (evm_contract)(function_signature)(gas_limit)(oracle)(admin)(version));
            } config_row;

            typedef singleton<"bridgeconfig"_n, bridgeconfig> config_singleton_bridge;
            config_singleton_bridge config_bridge;

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

    };
}