#include "../include/rngorc.brdg.hpp";

//======================== admin actions ==========================
// initialize the contract
ACTION OracleBridge::init(eosio::checksum160 evm_contract, string version, name admin, name oracle, string function_signature, bigint::checksum256 gas_limit){
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
ACTION OracleBridge::setfnsig(string new_function_signature){
    // authenticate
    require_auth(config.get().admin);

    auto stored = config.get();
    stored.function_signature = new_function_signature;

    // modify
    config.set(stored, get_self());
};

// set new gas limit
ACTION OracleBridge::setgaslimit(bigint::checksum256 new_gas_limit){
    // authenticate
    require_auth(config.get().admin);

    auto stored = config.get();
    stored.gas_limit = new_gas_limit;

    // modify
    config.set(stored, get_self());
};

// set the oracle
ACTION OracleBridge::setoracle(name new_oracle){
    // authenticate
    require_auth(config.get().admin);

    auto stored = config.get();
    stored.oracle = new_oracle;

    // modify
    config.set(stored, get_self());
};

// set the contract version
ACTION OracleBridge::setversion(string new_version){
    // authenticate
    require_auth(config.get().admin);

    auto stored = config.get();
    stored.version = new_version;

    // modify
    config.set(stored, get_self());
};

// set the bridge evm address
ACTION OracleBridge::setevmctc(eosio::checksum160 new_contract){
    // authenticate
    require_auth(config.get().admin);
    auto stored = config.get();
    stored.evm_contract = new_contract;
    // modify
    config.set(stored, get_self());
};

// set new contract admin
ACTION OracleBridge::setadmin(name new_admin){
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
ACTION OracleBridge::rmvrequest(uint64_t request_id)
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
ACTION OracleBridge::requestrand(bigint::checksum256 call_id, bigint::checksum256 seed, eosio::checksum160 caller, bigint::checksum256 max, bigint::checksum256 min)
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
ACTION OracleBridge::receiverand(uint64_t assoc_id, checksum256 random)
{
    // open config singleton
    auto conf = config.get();
    config_table evm_conf_table(EVM_SYSTEM_CONTRACT, EVM_SYSTEM_CONTRACT.value);
    auto evm_conf = evm_conf_table.get();
    // authenticate
    //require_auth(conf.admin);

    // find request
    requests_table requests(get_self(), get_self().value);
    auto &request = requests.get(assoc_id, "Request could not be found");

    // find account
    account_table _accounts(EVM_SYSTEM_CONTRACT, EVM_SYSTEM_CONTRACT.value);
    auto accounts_byaccount = _accounts.get_index<"byaccount"_n>();
    auto account = accounts_byaccount.require_find(get_self().value, "Account not found");

    // Get number from min/max
    auto byte_array = random.extract_as_byte_array();
    uint256_t random_int = 0;
    for (int i = 0; i < 32; i++) {
        random_int <<= 32;
        random_int |= (uint256_t)byte_array[i];
    }
    uint256_t number = request.getMin() + ( random_int % ( request.getMax() - request.getMin() + 1 ) );
    auto number_str = intx::to_byte_string(number);
    number_str.insert(number_str.begin(),(32 - number_str.size()), 0);

    // Prepare address
    auto evm_contract = conf.evm_contract.extract_as_byte_array();
    std::vector<uint8_t> to;
    to.insert(to.end(),  evm_contract.begin(), evm_contract.end());

    // Prepare solidity function parameters (function signature + arguments)
    // TODO: make that a function std::vector<uint8_t> prepareData(Args ...args) in utils (?)
    std::vector<uint8_t> call_id = intx::to_byte_string(request.call_id);
    call_id.insert(call_id.begin(),(16 - call_id.size()), 0);
    auto caller = pad160(request.caller).extract_as_byte_array();
    auto fnsig = toBin(conf.function_signature);

    std::vector<uint8_t> data;
    data.insert(data.end(), fnsig.begin(), fnsig.end());
    data.insert(data.end(), call_id.begin(), call_id.end());
    data.insert(data.end(), caller.begin(), caller.end());
    data.insert(data.end(), number_str.begin(), number_str.end());

    // Instantiate a transaction
    EthereumTransaction tx (
        account->nonce,
        evm_conf.gas_price,
        conf.gas_limit,
        to,
        uint256_t(0), // NO VALUE NEEDED
        data
    );

    // Send it using eosio.evm
    action(
        permission_level{get_self(),"active"_n},
        EVM_SYSTEM_CONTRACT,
        "raw"_n,
        std::make_tuple(get_self(), tx.encode_as_vector(), false, account->address)
    ).send();

    // DELETE REQUEST
   requests.erase(request);
};