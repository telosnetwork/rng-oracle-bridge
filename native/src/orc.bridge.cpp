#include "../include/orc.bridge.hpp";

//======================== admin actions ==========================
// initialize the contract
ACTION bridge::init(string evm_contract, string version, name initial_admin){
    // authenticate
    require_auth(get_self());

    // validate
    check(is_account(initial_admin), "initial admin account doesn't exist");

    // initialize
    auto stored = config.get_or_create(get_self(), config_row);
    stored.version = version;
    stored.admin = initial_admin;
    stored.evm_contract = evm_contract;
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

//======================== RNG Oracle actions ========================

// request
ACTION bridge::requestrand(name call_id, string seed, name caller)
{
    // GET ORACLE TYPE & FIND AN ORACLE
    oracles_table oracles(get_self(), get_self().value);
    name oracle_type = "rng"_n;
    auto idx = oracles.get_index<"bytype"_n>();
    auto oraclesRNG = idx.find(oracle_type.value);
    check(oraclesRNG != idx.end() , "No oracles found for type `rng`");

    // CHECK REQUEST HAS UNIQUE CALL ID
    requests_table requests(get_self(), get_self().value);
    auto itr = requests.find(call_id.value);
    check(itr != requests.end(), "Call ID already exists");

    // ADD TO REQUESTS
    requests.emplace(get_self(), [&](auto& r) {
        r.call_id = call_id;
        r.caller = caller;
        r.oracle_type = "rng"_n;
    });

    // SEND ACTION TO FIRST ORACLE THAT ACCEPTS IT
    for ( auto itr = oraclesRNG.begin(); itr != oraclesRNG.end(); itr++ ) {
          // TODO: add a bool return on rng.oracle action so we can iterate over those that don't work / or check the rng.oracle request table ??
          action(
            permission_level{get_self(),"active"_n},
            itr->oracle_name,
            "requestrand"_n,
            std::make_tuple(call_id, seed, caller)
          ).send();

          // if(result){
              itr = idx.end();
          // }
    }
};

// receive callback
ACTION bridge::receiverand(name call_id, uint64_t number)
{
    // open config singleton
    auto conf = config.get();

    // find request
    requests_table requests(get_self(), get_self().value);
    auto &request = requests.get(call_id.value, "Request could not be found");

    // find the serialized tx for rng type
    oracles_types_table oracles_types(get_self(), get_self().value);
    name orc_type = "rng"_n;
    auto oracle_type = oracles_types.find(orc_type.value);
    check(oracle_type != oracles_types.end(), "Oracle type could not be found");
    string raw_evm_tx = oracle_type->serialized_tx;

    // replace placeholders with correct tx parameters (regex too heavy)
    raw_evm_tx.replace(31, 40, request.caller.to_string()); // Place caller
    raw_evm_tx.replace(287, 40, request.call_id.to_string()); // Place call_id
    raw_evm_tx.replace(543, 40, "333"); // Place number

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

//======================== oracle type actions ========================

// remove an oracle type
ACTION bridge::rmvorctype(name oracle_type)
{
    // open config singleton, get config
    auto conf = config.get();

    // authenticate
    require_auth(conf.admin);

    // find type
    oracles_types_table oracles_types(get_self(), get_self().value);
    auto &orc_type = oracles_types.get(oracle_type.value, "oracle type not found");

    // find oracles of that type
    oracles_table oracles(get_self(), get_self().value);
    auto oracles_idx = oracles.get_index<"bytype"_n>();
    auto oracles_found = oracles_idx.lower_bound(oracle_type.value);
    for ( auto itr = oracles_found.begin(); itr != oracles_found.end(); itr++ ) {
        oracles.erase(itr);
    }

    // erase oracle
    oracles_types.erase(orc_type);

};

// add a new oracle type
ACTION bridge::upsertorctype(name oracle_type, string serialized_tx)
{
    // open config singleton, get config
    auto conf = config.get();

    // authenticate admin
    require_auth(conf.admin);

    // open oracles types table, find oracle type
    oracles_types_table oracles_types(get_self(), get_self().value);
    auto itr = oracles_types.find(oracle_type.value);

    if (itr == oracles_types.end())
    {
        // emplace new oracle type
        oracles_types.emplace(conf.admin, [&](auto &row) {
            row.type_name = oracle_type;
            row.serialized_tx = serialized_tx;
        });
    } else {
        oracles_types.modify(itr, conf.admin, [&]( auto& row ) {
            row.serialized_tx = serialized_tx;
        });
    }
};

//======================== oracle actions ========================

// remove an oracle
ACTION bridge::rmvoracle(name oracle_name)
{
    // open config singleton, get config
    auto conf = config.get();

    // authenticate
    require_auth(conf.admin);

    // open oracles table, find oracle
    oracles_table oracles(get_self(), get_self().value);
    auto &oracle = oracles.get(oracle_name.value, "oracle not found");

    // erase oracle
    oracles.erase(oracle);
};

// add a new oracle
ACTION bridge::upsertoracle(name oracle_name, name oracle_type)
{
    // open config singleton, get config
    auto conf = config.get();

    // authenticate admin
    require_auth(conf.admin);

    // check account exists
    check(is_account(oracle_name), "Oracle does not exist, please verify contract name");

    // open oracles table, find oracle
    oracles_table oracles(get_self(), get_self().value);
    auto itr = oracles.find(oracle_name.value);

    if (itr == oracles.end())
    {
        // emplace new oracle
        oracles.emplace(conf.admin, [&](auto &col) {
            col.oracle_name = oracle_name;
            col.oracle_type = oracle_type;
        });
    }
    else
    {
        // update oracle
        oracles.modify(*itr, conf.admin, [&](auto &col) {
            col.oracle_type = oracle_type;
        });
    }
};