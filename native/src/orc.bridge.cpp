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
ACTION bridge::requestrand(uint64_t call_id, string seed, name caller)
{
    // GET ORACLE TYPE & FIND AN ORACLE
    oracles_table oracles(get_self(), get_self().value);
    auto oraclesRNG = oracles.get_index<"typeid"_n>("rng");
    check(oraclesRNG != oracles.end() , "No oracles found for type `rng`");

    // CHECK REQUEST HAS UNIQUE CALL ID
    requests_table requests(get_self(), get_self().value);
    auto itr = requests.get(call_id.value);
    check(itr == requests.end(), "Call ID already exists");

    // ADD TO REQUESTS
    requests.emplace(get_self(), [&](auto& r) {
        r.call_id = call_id;
        r.caller = caller;
        r.oracle_type = "rng";
    });

    // SEND ACTION TO FIRST ORACLE THAT ACCEPTS IT
    for ( auto itr = oraclesRNG.begin(); itr != oraclesRNG.end(); itr++ ) {
          // TODO: add a bool return on rng.oracle action so we can iterate over those that don't work / or check the rng.oracle request table ??
          bool result = action(
            permission_level{get_self(),"active"_n},
            itr.oracle_name,
            "requestrand"_n,
            std::make_tuple(call_id, seed, caller)
          ).send();

          if(result){
              itr = oraclesRNG.end();
          }
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
    auto oracle_type = oracles_types.find("rng");
    check(oracle_type != oracles_types.end(), "Oracle type could not be found");
    string raw_evm_tx = oracle_type.serialized_tx;

    // replace placeholders with correct tx parameters (regex too heavy)
    raw_evm_tx.replace(31, 40, request.caller); // Place caller
    raw_evm_tx.replace(287, 40, request.call_id); // Place call_id
    raw_evm_tx.replace(543, 40, number); // Place number

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
    // TODO: open oracles table, find oracle by type and delete too

    oracles_types_table oracles_types(get_self(), get_self().value);
    auto &orc_type = oracles_types.get(oracle_type.value, "oracle type not found");

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
        oracles_types.emplace(conf.admin, [&](auto &col) {
            col.type_name = oracle_type;
            col.serialized_tx = serialized_tx;
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
ACTION bridge::upsertoracle(name oracle_name, string oracle_type)
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
        oracles.modify(*itr, same_payer, [&](auto &col) {
            col.oracle_type = oracle_type;
        });
    }
};