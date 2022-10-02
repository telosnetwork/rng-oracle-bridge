#! /bin/bash
echo ">>> Building contract..."
if [ ! -d "$PWD/build" ]
then
  mkdir build
fi
eosio-cpp -I="./include/"  -I="./external/"  -o="./build/rng.bridge.wasm" -contract="rng.bridge" -abigen ./src/rng.bridge.cpp