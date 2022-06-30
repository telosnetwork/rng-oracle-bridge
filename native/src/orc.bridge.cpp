#include "../include/orc.bridge.hpp";

//======================== admin actions ==========================
// initialize the contract
ACTION bridge::init(eosio::checksum160 evm_contract, string version, name admin, name oracle, string serialized_tx){
    // authenticate
    require_auth(get_self());

    // validate
    check(is_account(admin), "initial admin account doesn't exist");

    // initialize
    auto stored = config.get_or_create(get_self(), config_row);
    stored.version = version;
    stored.admin = admin;
    stored.oracle = oracle;
    stored.serialized_tx = serialized_tx;
    stored.evm_contract = evm_contract;
    config.set(stored, get_self());
};
// set the oracle
ACTION bridge::setfnsig(string new_function_signature){
    // authenticate
    require_auth(config.get().admin);

    auto stored = config.get();
    stored.function_signature = new_function_signature;

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
    auto seed_64 = intx::as_words(seed);

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
    // authenticate
    // require_auth(conf.oracle);

    // find request
    requests_table requests(get_self(), get_self().value);
    auto &request = requests.get(caller_id, "Request could not be found");

    // Generate EVM serialized transaction
    string function_signature = conf.function_signature;
    string gas_limit = conf.gas_limit;
    string gas_price = conf.gas_price;
    auto nonce = account.nonce;
    auto caller = rlp::encode(eosio_evm::checksum160ToAddress(request.caller));
    string data[1];
    data[0] = to_string(eosio_evm::checksum256ToValue(random));
    rlp::encode(data[0]);
    auto raw = rlp::encode(request.call_id);
    /* raw_evm_tx.replace(597, ceil(number_str_length / 10), to_string(number_str_length)); // Place number length
    raw_evm_tx.replace(598, 32, number); // Place number
    raw_evm_tx.replace(30, caller.length(), caller); // Place caller
    raw_evm_tx.replace(341, ceil(call_str_length / 10), to_string(call_str_length)); // Place call_id length
    raw_evm_tx.replace(342, 32, call_str); // Place call_id

    action(
        permission_level{get_self(),"active"_n},
        "eosio.evm"_n,
        "raw"_n,
        std::make_tuple(get_self(), std::vector<int8_t>(raw_evm_tx.c_str(), raw_evm_tx.c_str() + raw_evm_tx.size()), false, conf.evm_contract)
    ).send();

    // DELETE REQUEST
    requests.erase(request); */

};