# Aphelium Bank Contract
This contract handles the bank accounts used in Aphelium

### Token
The contract allows users to send balance of any of the allowed tokens (set via the *allowedtkns* config). The user can withdraw them anytime with no fees involved.

### NFTs
The contract allows users to send NFTs and retrieve them anytime.

NB. For this feature to work, the collection of the NFTs used must put this contract into the *Notified accounts* settings of the collection.

## Building
To compile the contract use `eosio-cpp -abigen bank.cpp -o bank.wasm`
