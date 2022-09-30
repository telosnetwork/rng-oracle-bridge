#include "../include/rng.bridge.hpp";

namespace orc_bridge
{
    //======================== Admin actions ==========================
    // Initialize the contract
    ACTION rngbridge::init(eosio::checksum160 evm_contract, string version, name admin){
        // Authenticate
        require_auth(get_self());

        // Validate
        check(!config_bridge.exists(), "contract already initialized");
        check(is_account(admin), "initial admin account doesn't exist");

        // Initialize
        auto stored = config_bridge.get_or_create(get_self(), config_row);

        stored.version            = version;
        stored.admin              = admin;
        stored.evm_contract       = evm_contract;

        // Get the scope
        account_table accounts(EVM_SYSTEM_CONTRACT, EVM_SYSTEM_CONTRACT.value);
        auto accounts_byaddress = accounts.get_index<"byaddress"_n>();
        auto account = accounts_byaddress.require_find(pad160(evm_contract), "EVM bridge contract not found in eosio.evm accounts");

        stored.evm_contract_scope       = account->index;

        config_bridge.set(stored, get_self());
    };

    // Set the contract version
    ACTION rngbridge::setversion(string new_version){
        // Authenticate
        require_auth(config_bridge.get().admin);

        auto stored = config_bridge.get();
        stored.version = new_version;

        // Modify
        config_bridge.set(stored, get_self());
    };

    // Set the bridge evm address
    ACTION rngbridge::setevmctc(eosio::checksum160 new_contract){
        // Authenticate
        require_auth(config_bridge.get().admin);

        // Get the scope for accountstates
        account_table accounts(EVM_SYSTEM_CONTRACT, EVM_SYSTEM_CONTRACT.value);
        auto accounts_byaddress = accounts.get_index<"byaddress"_n>();
        auto account = accounts_byaddress.require_find(pad160(new_contract), "EVM bridge contract not found in eosio.evm accounts");

        // Save
        auto stored = config_bridge.get();
        stored.evm_contract = new_contract;
        stored.evm_contract_scope = account->index;

        config_bridge.set(stored, get_self());
    };

    // Set new contract admin
    ACTION rngbridge::setadmin(name new_admin){
        // Authenticate
        require_auth(config_bridge.get().admin);

        // Check account exists
        check(is_account(new_admin), "New admin account does not exist, please verify the account name provided");

        auto stored = config_bridge.get();
        stored.admin = new_admin;
        // Modify
        config_bridge.set(stored, get_self());
    };

    //======================== Request actions ========================

    // Remove a request
    ACTION rngbridge::rmvrequest(uint64_t request_id)
    {
        // Authenticate
        require_auth(config_bridge.get().admin);

        // Find request
        requests_table requests(get_self(), get_self().value);
        auto itr = requests.find(request_id);
        check(itr != requests.end(), "Request not found");

        // Delete it
        requests.erase(itr);
    };

    //======================== RNG Oracle actions ========================
    // Request notification, checks for values in eosio.evm accountstate and adds RNG requests accordingly
    ACTION rngbridge::reqnotify()
    {
        // Open config_bridge singleton
        auto conf = config_bridge.get();

        // Define rest of tables
        account_state_table account_states(EVM_SYSTEM_CONTRACT, conf.evm_contract_scope);
        requests_table requests(get_self(), get_self().value);

        // Get array slot to find array length
        auto account_states_bykey = account_states.get_index<"bykey"_n>();
        auto storage_key = toChecksum256(uint256_t(STORAGE_INDEX));
        auto array_length = account_states_bykey.require_find(storage_key, "No requests");
        auto array_slot = checksum256ToValue(keccak_256(storage_key.extract_as_byte_array()));

        check(array_length->value > 0, "No requests found");

        // Loop to make sure we do not miss requests (concurrency)
        for(uint256_t i = 0; i < array_length->value;i=i+1){
            auto position = array_length->value - i;
            auto id_slot = getArrayMemberSlot(array_slot, 0, 9, position);

            // Get call ID
            auto call_id_checksum = account_states_bykey.find(id_slot);
            auto call_id = (call_id_checksum == account_states_bykey.end()) ? uint256_t(0) : call_id_checksum->value;

            // Check request not already processing
            auto requests_by_call_id = requests.get_index<"bycallid"_n>();
            auto request = requests_by_call_id.find(toChecksum256(call_id));
            if(request != requests_by_call_id.end()){
                continue;
            }

            // Get data stored in account state
            const auto seed = account_states_bykey.require_find(getArrayMemberSlot(array_slot, 4, 9, position), "Seed not found");

            const auto count_checksum = account_states_bykey.find(getArrayMemberSlot(array_slot, 5, 9, i));
            const uint256_t count = (count_checksum == account_states_bykey.end()) ? uint256_t(1) : count_checksum->value;

            const auto gas_checksum = account_states_bykey.find(getArrayMemberSlot(array_slot, 6, 9, i));
            const uint256_t gas = (gas_checksum == account_states_bykey.end()) ? uint256_t(0) : gas_checksum->value;

            // Add request
            uint64_t request_id = requests.available_primary_key();
            requests.emplace(get_self(), [&](auto& r) {
                r.request_id = request_id;
                r.call_id = toChecksum256(call_id);
                r.gas = gas;
                r.count = count;
                r.numbers = new uint256_t[count];
            });

            uint64_t seed_64 = intx::lo_half(intx::lo_half(seed->value)); // Seed is on first 64 bits of stored value (64b stored as 256b in accountstates table)

            // Send request to oracle * count of numbers requested (remember that seed is modified for each request)
            for(uint256_t i = 0; i < count; i++){
                action(
                    permission_level{get_self(),"active"_n},
                    ORACLE,
                    "requestrand"_n,
                    std::make_tuple(request_id, seed_64, get_self())
                ).send();
            }
        }

    };

    // Receive random number
    ACTION rngbridge::receiverand(uint64_t caller_id, checksum256 random)
    {
        // Open config singletons
        auto conf = config_bridge.get();
        config_singleton_evm config_evm(EVM_SYSTEM_CONTRACT, EVM_SYSTEM_CONTRACT.value);
        auto evm_conf = config_evm.get();

        // Authenticate
        require_auth(ORACLE);

        // Find request
        requests_table requests(get_self(), get_self().value);
        auto &request = requests.get(caller_id, "Request could not be found");

        // Find account
        account_table _accounts(EVM_SYSTEM_CONTRACT, EVM_SYSTEM_CONTRACT.value);
        auto accounts_byaccount = _accounts.get_index<"byaccount"_n>();
        auto account = accounts_byaccount.require_find(get_self().value, "Account not found");

        // Get random number from checksum256 (casted as uint256_t for Solidity)
        auto byte_array = random.extract_as_byte_array();
        uint256_t random_int = 0;
        for (int i = 0; i < 32; i++) {
            random_int <<= 32;
            random_int |= (uint256_t)byte_array[i];
        }

        // Modify request to save the number
        request.modify(iter,get_self(), [&]( auto& row ) {
           row.numbers[row.numbers.length] = random_int;
        });

        if(request.numbers.length != request.count){
            return; // Don't send response as long as we don't have the requested count of numbers
        }

        // Prepare address
        auto evm_contract = conf.evm_contract.extract_as_byte_array();
        std::vector<uint8_t> to;
        to.insert(to.end(),  evm_contract.begin(), evm_contract.end());

        // Prepare solidity function parameters (function signature + arguments)
        std::vector<uint8_t> data;
        std::vector<uint8_t> call_id = intx::to_byte_string(checksum256ToValue(request.call_id));
        call_id.insert(call_id.begin(),(16 - call_id.size()), 0);
        auto fnsig = toBin(FUNCTION_SIGNATURE);
        data.insert(data.end(), fnsig.begin(), fnsig.end());
        data.insert(data.end(), call_id.begin(), call_id.end());

        // Prepare the uint256[] tuple that holds the numbers
        prefixTupleArray(data, request.numbers.length);
        // Insert each member's position
        for(int k = 0; k < request.numbers.length; k++){
           std::vector<uint8_t> element_position = pad(intx::to_byte_string(uint256_t(32 * request.numbers.length + (32 * 9 * k))), 32, true);  // position of each member
           data.insert(data.end(), element_position.begin(), element_position.end());
        }
        // Insert each member's data
        for(int k = 0; k < request.numbers.length; k++){
           const auto number_bs = intx::to_byte_string(request.numbers[k]);
           data.insert(data.end(), number_bs.begin(), number_bs.end());
        }

        // Send it back to EVM using eosio.evm
        action(
            permission_level {get_self(), "active"_n},
            EVM_SYSTEM_CONTRACT,
            "raw"_n,
            std::make_tuple(get_self(), rlp::encode(account->nonce, evm_conf.gas_price, request.gas + BASE_GAS, to, uint256_t(0), data, 41, 0, 0),  false, std::optional<eosio::checksum160>(account->address))
        ).send();

        // Delete request
        requests.erase(request);
    };
}