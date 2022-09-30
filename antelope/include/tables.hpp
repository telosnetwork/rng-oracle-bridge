#pragma once

using namespace std;
using namespace eosio;
using namespace orc_bridge;

namespace orc_bridge {
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

    struct [[eosio::table, eosio::contract("eosio.evm")]] AccountState {
        uint64_t index;
        eosio::checksum256 key;
        bigint::checksum256 value;

        uint64_t primary_key() const { return index; };
        eosio::checksum256 by_key() const { return key; };

        EOSLIB_SERIALIZE(AccountState, (index)(key)(value));
      };

      typedef eosio::multi_index<"accountstate"_n, AccountState,
        eosio::indexed_by<eosio::name("bykey"), eosio::const_mem_fun<AccountState, eosio::checksum256, &AccountState::by_key>>
      > account_state_table;

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
    struct [[eosio::table, eosio::contract("rng.bridge")]] bridgeconfig {
        eosio::checksum160 evm_contract;
        name admin;
        string version;
        uint64_t evm_contract_scope;

        EOSLIB_SERIALIZE(bridgeconfig, (evm_contract)(admin)(version)(evm_contract_scope));
    } config_row;

    typedef singleton<"bridgeconfig"_n, bridgeconfig> config_singleton_bridge;

    // Request
    struct [[eosio::table, eosio::contract("rng.bridge")]] Request {
        uint64_t request_id;
        eosio::checksum256 call_id;
        bigint::checksum256 gas;
        bigint::checksum256 count;
        std::vector<eosio::checksum256> numbers;

        uint64_t primary_key() const { return request_id; };
        uint256_t getGas() const { return gas; };
        uint256_t getCount() const { return count; };
        eosio::checksum256 by_call_id() const { return call_id; };

        EOSLIB_SERIALIZE(Request, (request_id)(call_id)(gas)(count));
    };
    typedef multi_index<name("requests"), Request,
       eosio::indexed_by<eosio::name("bycallid"), eosio::const_mem_fun<Request, eosio::checksum256, &Request::by_call_id >>
    >  requests_table;

}