#include "include/bank.hpp"

void bank::setconfig(string key, uint64_t uint_value, int32_t int_value, string string_value, vector<string> arr_value, vector<TOKEN> tkn_value)
{
    require_auth(get_self());
    
    auto itr = configs.find(name(key).value);
    if (itr != configs.end()) {
        configs.modify(itr, get_self(), [&](auto &_config) {
            _config.uint_value = uint_value;
            _config.int_value = int_value;
            _config.string_value = string_value;
            _config.arr_value = arr_value;
            _config.tkn_value = tkn_value;
        });
    }
    else {
        configs.emplace(get_self(), [&](auto &_config) {
            _config.key = name(key);
            _config.uint_value = uint_value;
            _config.int_value = int_value;
            _config.string_value = string_value;
            _config.arr_value = arr_value;
            _config.tkn_value = tkn_value;
        });
    }
}

void bank::delconfig(string key)
{
    require_auth(get_self());
    
    auto itr = configs.require_find(name(key).value, "Config not found");
    
    configs.erase(itr);
}

void bank::handletransfer (name from, name to, asset quantity, string memo)
{
    if (to != get_self() || from == get_self()) {
        print("These are not the droids you are looking for");
        return;
    }
    
    check(quantity.amount > 0, "Amount has to be > 0");
    
    string symbol = quantity.symbol.code().to_string();
    auto allowedtkns = configs.require_find(name("allowedtkns").value, "Allowed tokens not set, contact an administrator");
    bool config_found = false;
    for (TOKEN tkn : allowedtkns->tkn_value) {
        if (symbol == tkn.symbol) {
            config_found = true;
        }
    }
    check(config_found, "Symbol not allowed");
    
    check(memo.substr(0, 7) == "deposit", "Token transfer not allowed");
    
    auto itr = balances.find(from.value);
    
    check(itr != balances.end(), "You need to create a bank first");
    
    vector <asset> quantities = itr->quantities;
    bool found_token = false;
    for (asset &token : quantities) {
        if (quantity.symbol == token.symbol) {
            found_token = true;
            token.amount += quantity.amount;
            break;
        }
    }
    if (!found_token) {
        quantities.push_back(quantity);
    }
    
    balances.modify(itr, get_self(), [&](auto &_balance) {
        _balance.quantities = quantities;
    });
}

void bank::handlenfttransfer (name collection_name, name from, name to, vector<uint64_t> asset_ids, string memo)
{
    if (to != get_self() || from == get_self()) {
        print("These are not the droids you are looking for");
        return;
    }
    
    check(memo.substr(0, 7) == "deposit", "NFT transfer not allowed");
    
    atomicassets::assets_t owned_assets = atomicassets::get_assets(to);
    auto itr = balances.require_find(from.value, "You need to create a bank first");
    
    vector<NFT> nfts = itr->nfts;
    for (uint64_t asset_id: asset_ids) {
        auto asset_itr = owned_assets.find(asset_id);
        
        NFT nft;
        nft.template_id = asset_itr->template_id;
        nft.asset_id = asset_id;
        
        nfts.push_back(nft);
    }
    
    balances.modify(itr, get_self(), [&](auto &_balance) {
        _balance.nfts = nfts;
    });
}

void bank::createbank(name player)
{
    require_auth(player);
    
    auto itr = balances.find(player.value);
    check(itr == balances.end(), "Bank account already created");
    
    balances.emplace(player, [&](auto &_bank) {
        _bank.owner = player;
    });
}

void bank::usebanktkn(name player, asset quantity)
{
    require_auth(get_self());
    
    auto itr = balances.require_find(player.value, "Bank not found");
    
    vector <asset> quantities = itr->quantities;
    bool found = false;
    for (asset &token : quantities) {
        if (quantity.symbol == token.symbol) {
            found = true;
            check(token.amount > quantity.amount, "Insufficent balance");
            token.amount -= quantity.amount;
            auto config_itr = configs.find(name("bankacc").value);
            if (config_itr != configs.end()) {
                action(
                    permission_level{get_self(), name("active")},
                    name("eosio.token"),
                    name("transfer"),
                    make_tuple(
                        get_self(),
                        name(config_itr->string_value),
                        quantity,
                        string("bank use")
                    )
                ).send();
            }
            break;
        }
    }
    
    check(found, "Asset not found");
    
    balances.modify(itr, get_self(), [&](auto &_balance) {
        _balance.quantities = quantities;
    });
}

void bank::usebanknft(name player, int32_t template_id, uint64_t quantity)
{
    require_auth(get_self());
    
    auto itr = balances.require_find(player.value, "Bank not found");
    
    uint64_t used = 0;
    vector<NFT> left_nfts = itr->nfts;
    for (NFT nft : itr->nfts) {
        if (nft.template_id == template_id) {
            used++;
            // Burn the NFT
            action(
                permission_level{get_self(), name("active")},
                name("atomicassets"),
                name("burnasset"),
                make_tuple(
                    get_self(),
                    nft.asset_id
                )
            ).send();
            left_nfts.erase(left_nfts.begin());
        }
    }
    
    check(used == quantity, "Not enough assets in the bank");
    
    balances.modify(itr, get_self(), [&](auto &_balance) {
        _balance.nfts = left_nfts;
    });
}

void bank::withdrawtkn(name player, asset quantity)
{
    require_auth(player);
    
    auto itr = balances.require_find(player.value, "Bank not found");
    auto allowedtkns = configs.require_find(name("allowedtkns").value, "Allowed tokens not set, contact an administrator");
    string symbol = quantity.symbol.code().to_string();
    
    bool asset_found = false;
    vector <asset> quantities = itr->quantities;
    for (asset &tkn : quantities) {
        if (quantity.symbol == tkn.symbol) {
            asset_found = true;
            check(tkn.amount >= quantity.amount, "Not enough balance");
            bool config_found = false;
            for (TOKEN tknconf : allowedtkns->tkn_value) {
                if (symbol == tknconf.symbol) {
                    config_found = true;
                        action(
                        permission_level{get_self(), name("active")},
                        name(tknconf.contract),
                        name("transfer"),
                        make_tuple(
                            get_self(),
                            player,
                            quantity,
                            string("withdraw")
                        )
                    ).send();
                    tkn.amount -= quantity.amount;
                }
            }
            check(config_found, "Symbol not allowed");
        }
    }
    
    check(asset_found, "No balance available for this token");
    
    balances.modify(itr, player, [&](auto &_balance) {
        _balance.quantities = quantities;
    });
}