#include "../include/orc.bridge.hpp";

//======================== admin actions ==========================
// initialize the contract
ACTION bridge::init(string evm_contract, string version, name admin, name oracle, string serialized_tx){
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
ACTION bridge::setserialtx(string new_serialized_tx){
    // authenticate
    require_auth(config.get().admin);

    auto stored = config.get();
    stored.serialized_tx = new_serialized_tx;

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
ACTION bridge::setevmctc(string new_contract){
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
ACTION bridge::rmvrequest(name call_id)
{
    requests_table requests(get_self(), get_self().value);
    auto itr = requests.find(call_id.value);
    check(itr != requests.end(), "Call ID not found");
    requests.erase(itr);
};

//======================== RNG Oracle actions ========================

// request
ACTION bridge::requestrand(name call_id, uint64_t seed, string caller)
{
    // CHECK REQUEST HAS UNIQUE CALL ID
    requests_table requests(get_self(), get_self().value);
    auto itr = requests.find(call_id.value);
    check(itr == requests.end(), "Call ID already exists");

    // ADD TO REQUESTS
    requests.emplace(get_self(), [&](auto& r) {
        r.call_id = call_id;
        r.caller = caller;
    });

    // open config singleton
    auto conf = config.get();

    action(
        permission_level{get_self(),"active"_n},
        conf.oracle,
        "requestrand"_n,
        std::make_tuple(call_id, seed, get_self())
    ).send();
};

// receive callback
ACTION bridge::receiverand(name call_id, checksum256 number)
{
    // open config singleton
    auto conf = config.get();

    // find request
    requests_table requests(get_self(), get_self().value);
    auto &request = requests.get(call_id.value, "Request could not be found");

    // convert checksum256 to string
    auto num = number.extract_as_byte_array();
    string num_str( reinterpret_cast<char *>( num.data() ), num.size() );

    string raw_evm_tx = conf.serialized_tx;
    // replace placeholders with correct tx parameters (regex too heavy)
    raw_evm_tx.replace(31, 40, request.caller); // Place caller
    raw_evm_tx.replace(287, 40, request.call_id.to_string()); // Place call_id
    raw_evm_tx.replace(543, 40, num_str); // Place number

    // SEND RESULT TO EVM
      action(
        permission_level{get_self(),"active"_n},
        "eosio.evm"_n,
        "raw"_n,
        std::make_tuple(get_self(), raw_evm_tx, false, conf.evm_contract)
      ).send();

    // DELETE REQUEST
    requests.erase(request);

};
