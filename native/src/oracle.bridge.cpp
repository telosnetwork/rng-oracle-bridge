#include "../include/oracle.bridge.hpp";

//======================== admin actions ==========================

// intialize the contract
ACTION bridge::init(string evm_contract, string app_version, name initial_admin){
    //authenticate
    require_auth(get_self());

    //open config singleton
    config_singleton configs(get_self(), get_self().value);

    //validate
    check(!configs.exists(), "contract already initialized");
    check(is_account(initial_admin), "initial admin account doesn't exist");

    //initialize
    config initial_conf = {
        evm_contract,
        app_version,   //app_version
        initial_admin, //admin
    };

    //set initial config
    configs.set(initial_conf, get_self());
};

// set the contract version
ACTION bridge::setversion(string new_version){
    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //authenticate
    require_auth(conf.admin);

    //change version
    conf.app_version = new_version;

    //set new config
    configs.set(conf, get_self());

};

// set the bridge evm address
ACTION bridge::setevmcontract(string new_contract){
    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //authenticate
    require_auth(conf.admin);

    //change version
    conf.evm_contract = new_contract;

    //set new config
    configs.set(conf, get_self());

};

// set new contract admin
ACTION bridge::setadmin(name new_admin){
    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //authenticate
    require_auth(conf.admin);

    //change version
    conf.admin = new_admin;

    //set new config
    configs.set(conf, get_self());
};

//======================== request actions ========================

// request a random value w/ native caller
ACTION bridge::requestrand(uint64_t caller_id, uint64_t seed)
{

};

//======================== oracle type actions ========================

// remove an oracle type
ACTION bridge::rmvoracletype(string oracle_type)
{
    require_auth(conf.admin);
    // TODO: open oracles table, find oracle by type and delete too

    oracles_types_table oracles_types(get_self(), get_self().value);
    auto &orc_type = oracles_types.get(oracle_type.value, "oracle not found");
    // config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    // erase oracle
    oracles.erase(orc_type);

};

// add a new oracle type
ACTION bridge::upsertoracletype(string oracle_type)
{

    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //authenticate admin
    require_auth(conf.admin);

    //open oracles types table, find oracle type
    oracles_types_table oracles_types(get_self(), get_self().value);
    auto itr = oracles_types.find(oracle_type.value);

    if (itr == oracles_types.end())
    {
        //emplace new oracle type
        oracles_types.emplace(conf.admin, [&](auto &col) {
            col.type_id = oracle_type;
        });
    }
};

//======================== oracle actions ========================

// remove an oracle
ACTION bridge::rmvoracle(name oracle_name)
{
    //open oracles table, find oracle
    oracles_table oracles(get_self(), get_self().value);
    auto &oracle = oracles.get(oracle_name.value, "oracle not found");

    //config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //authenticate
    if (!has_auth(conf.admin))
        require_auth(oracle.oracle_name);

    //erase oracle
    oracles.erase(oracle);
};

// add a new oracle
ACTION bridge::upsertoracle(name oracle_name, string oracle_type)
{
    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //authenticate admin
    require_auth(conf.admin);

    //open oracles table, find oracle
    oracles_table oracles(get_self(), get_self().value);
    auto itr = oracles.find(oracle_name.value);

    if (itr == oracles.end())
    {
        //emplace new oracle
        oracles.emplace(conf.admin, [&](auto &col) {
            col.oracle_name = oracle_name;
            col.oracle_type = oracle_type;
        });
    }
    else
    {
        //update oracle
        oracles.modify(*itr, same_payer, [&](auto &col) {
            col.oracle_type = oracle_type;
        });
    }
};