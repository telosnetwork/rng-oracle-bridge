// @author Thomas Cuvillier
// @contract rngorcbrdg
// @version v0.1.0

#include <vector>

// EOSIO
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/crypto.hpp>

// EXTERNAL
#include <intx/base.hpp>
#include <rlp/rlp.hpp>
#include <ecc/uECC.c>
#include <keccak256/k.c>
#include <boost/multiprecision/cpp_int.hpp>

// TELOS EVM
#include <constants.hpp>
#include <util.hpp>
#include <datastream.hpp>
#include <tables.hpp>

#define EVM_SYSTEM_CONTRACT name("eosio.evm")
#define TESTING true

using namespace std;
using namespace eosio;
using namespace orc_bridge;

namespace orc_bridge
{
    class [[eosio::contract("rng.bridge")]] rngbridge : public contract {
        public:
            using contract::contract;
            rngbridge(name self, name code, datastream<const char*> ds) : contract(self, code, ds), config_bridge(self, self.value) { };
            ~rngbridge() {};

            //======================== Admin actions ========================
            // intialize the contract
            ACTION init(eosio::checksum160 evm_contract, string version, name admin);

            //set the contract version
            ACTION setversion(string new_version);

            //set the bridge evm address
            ACTION setevmctc(eosio::checksum160 new_contract);

            //set new contract admin
            ACTION setadmin(name new_admin);

            //======================== Request actions ========================
            ACTION rmvrequest(uint64_t request_id);

            //======================== RNG Oracle actions ========================

            ACTION reqnotify();
            ACTION receiverand(uint64_t caller_id, checksum256 random);

            //======================= Testing action =============================
            #if (TESTING == true)
                ACTION clearall()
                {
                    config_bridge.remove();
                    requests_table requests(get_self(), get_self().value);
                    auto itr = requests.end();
                    while (requests.begin() != itr)
                    {
                      itr = requests.erase(--itr);
                    }
                }
            #endif
            config_singleton_bridge config_bridge;
    };
}