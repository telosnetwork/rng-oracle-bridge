#include "../include/orc.bridge.hpp";

//======================== admin actions ==========================
// initialize the contract
ACTION bridge::init(eosio::checksum160 evm_contract, string version, name admin, name oracle, string function_signature, bigint::checksum256 gas_limit){
    // authenticate
    require_auth(get_self());

    // validate
    check(!config.exists(), "contract already initialized");
    check(is_account(admin), "initial admin account doesn't exist");

    // initialize
    auto stored = config.get_or_create(get_self(), config_row);
    stored.version = version;
    stored.admin = admin;
    stored.oracle = oracle;
    stored.gas_limit = gas_limit;
    stored.function_signature = function_signature;
    stored.evm_contract = evm_contract;
    config.set(stored, get_self());
};

// set a new function signature
ACTION bridge::setfnsig(string new_function_signature){
    // authenticate
    require_auth(config.get().admin);

    auto stored = config.get();
    stored.function_signature = new_function_signature;

    // modify
    config.set(stored, get_self());
};

// set new gas limit
ACTION bridge::setgaslimit(bigint::checksum256 new_gas_limit){
    // authenticate
    require_auth(config.get().admin);

    auto stored = config.get();
    stored.gas_limit = new_gas_limit;

    // modify
    config.set(stored, get_self());
};

// set the oracle
ACTION bridge::setoracle(name new_oracle){
    // authenticate
    require_auth(config.get().admin);

    auto stored = config.get();
    stored.oracle = new_oracle;

    // modify
    config.set(stored, get_self());
};

// set the contract version
ACTION bridge::setversion(string new_version){
    // authenticate
    require_auth(config.get().admin);

    auto stored = config.get();
    stored.version = new_version;

    // modify
    config.set(stored, get_self());
};

// set the bridge evm address
ACTION bridge::setevmctc(eosio::checksum160 new_contract){
    // authenticate
    require_auth(config.get().admin);
    auto stored = config.get();
    stored.evm_contract = new_contract;
    // modify
    config.set(stored, get_self());
};

// set new contract admin
ACTION bridge::setadmin(name new_admin){
    // authenticate
    require_auth(config.get().admin);

    // check account exists
    check(is_account(new_admin), "New admin account does not exist, please verify the account name provided");

    auto stored = config.get();
    stored.admin = new_admin;
    // modify
    config.set(stored, get_self());
};

//======================== Request actions ========================

// remove a request
ACTION bridge::rmvrequest(uint64_t request_id)
{
    // open config singleton
    auto conf = config.get();

    // authenticate
    require_auth(config.get().admin);

    // find request
    requests_table requests(get_self(), get_self().value);
    auto itr = requests.find(request_id);
    check(itr != requests.end(), "Request not found");

    //delete it
    requests.erase(itr);
};

//======================== RNG Oracle actions ========================

// request
ACTION bridge::requestrand(bigint::checksum256 call_id, bigint::checksum256 seed, eosio::checksum160 caller, bigint::checksum256 max, bigint::checksum256 min)
{
    requests_table requests(get_self(), get_self().value);

    // add request
    uint64_t request_id = requests.available_primary_key();
    requests.emplace(get_self(), [&](auto& r) {
        r.request_id = request_id;
        r.call_id = call_id;
        r.caller = caller;
        r.min = min;
        r.max = max;
    });

    // open config singleton
    auto conf = config.get();

    uint64_t seed_64 = lo_half(lo_half(seed)); // Seed should be on first 64 bits of stored value (64b stored as 256b in accountstates table)
    // Possibly use other method to get seed (doesn't matter much ??)

    // send to oracle
    action(
        permission_level{get_self(),"active"_n},
        conf.oracle,
        "requestrand"_n,
        std::make_tuple(request_id, seed_64, get_self())
    ).send();
};

// receive callback
ACTION bridge::receiverand(uint64_t caller_id, checksum256 random)
{
    // open config singleton
    auto conf = config.get();
    config_table evm_conf_table(EVM_CONTRACT, EVM_CONTRACT.value);
    auto evm_conf = evm_conf_table.get();
    // authenticate
    require_auth(conf.admin);

    // find request
    requests_table requests(get_self(), get_self().value);
    auto &request = requests.get(caller_id, "Request could not be found");

    // find account for nonce
    account_table _accounts(EVM_CONTRACT, EVM_CONTRACT.value);
    auto accounts_byaccount = _accounts.get_index<"byaccount"_n>();
    auto account = accounts_byaccount.require_find(get_self().value, "Account not found");

    // Handle min / max
    auto byte_array = random.extract_as_byte_array();
    uint256_t random_int = 0;
    for (int i = 0; i < 32; i++) {
        random_int <<= 32;
        random_int |= (uint256_t)byte_array[i];
    }
    uint256_t number = request.getMin() + ( random_int % ( request.getMax() - request.getMin() + 1 ) );
    auto number_str = intx::to_byte_string(number);

    // Prepare data
    auto fn = toBin(conf.function_signature);
    uint256_t gas_limit = conf.gas_limit;
    uint256_t gas_price = evm_conf.gas_price;
    std::vector<uint8_t> call_id = intx::to_byte_string(request.call_id);
    auto caller = pad160(request.caller).extract_as_byte_array();

    auto evm_contract = conf.evm_contract.extract_as_byte_array();
    std::vector<uint8_t> to;
    to.insert(to.end(),  evm_contract.begin(), evm_contract.end());

    // Prepare solidity function parameters (function signature + arguments)
    std::vector<uint8_t> data;
    data.insert(data.end(),  fn.begin(), fn.end());
    data.insert(data.end(), call_id.begin(), call_id.end());
    data.insert(data.end(), caller.begin(), caller.end());
    data.insert(data.end(), number_str.begin(), number_str.end());

    // Instantiate a transaction
    EthereumTransaction tx (
        account->nonce,
        gas_price,
        gas_limit,
        to,
        uint256_t(0), // NO VALUE
        data
    );

    // Encode it
    string rlp_encoded = tx.encodeUnsigned();

    // Print it
    std::vector<uint8_t> raw;
    raw.insert(raw.end(), std::begin(rlp_encoded), std::end(rlp_encoded));
    print(bin2hex(raw));

    // Send it using eosio.evm
    /* action(
        permission_level{get_self(),"active"_n},
        EVM_CONTRACT,
        "raw"_n,
        std::make_tuple(get_self(), bin2hex(raw), false, conf.evm_contract)
    ).send();

    // DELETE REQUEST
   requests.erase(request); */
};