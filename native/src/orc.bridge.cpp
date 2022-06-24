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

    auto stored = config.get();
    stored.admin = new_admin;
    // modify
    config.set(stored, get_self());
};

//======================== request actions ========================

// request a random value w/ native caller
ACTION bridge::requestrand(uint64_t caller_id, uint64_t seed)
{

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
    auto &orc_type = oracles_types.get(oracle_type.value, "oracle not found");

    // erase oracle
    oracles_types.erase(orc_type);

};

// add a new oracle type
ACTION bridge::upsertorctype(name oracle_type)
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
        });
    }
};

//======================== oracle actions ========================

// remove an oracle
ACTION bridge::rmvoracle(name oracle_name)
{
    // open oracles table, find oracle
    oracles_table oracles(get_self(), get_self().value);
    auto &oracle = oracles.get(oracle_name.value, "oracle not found");

    // open config singleton, get config
    auto conf = config.get();

    // authenticate
    if (!has_auth(conf.admin))
        require_auth(oracle.oracle_name);

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