#include "../include/OracleBridge.hpp";

using namespace OracleBridge;


//======================== admin actions ==========================

// intialize the contract
ACTION OracleBridge::init(string evm_contract){

};

// set the contract version
ACTION OracleBridge::setversion(string new_version){

};

// set the bridge evm address
ACTION OracleBridge::setevmcontract(string evm_contract){

};

// set new contract admin
ACTION OracleBridge::setadmin(name new_admin){

};

//======================== request actions ========================

// request a random value w/ native caller
ACTION OracleBridge::requestrand(uint64_t caller_id, uint64_t seed)
{

};

//======================== oracle actions ========================

// remove an oracle
ACTION OracleBridge::rmvoracle(name oracle_name)
{

};

// add a new oracle
ACTION OracleBridge::upsertoracle(name oracle_name)
{

};