#! /bin/bash
echo ">>> Building contract..."
eosio-cpp -I="./include/"  -I="./external/"  -o="./build/rng.bridge.wasm" -contract="rng.bridge" -abigen ./src/rng.bridge.cpp