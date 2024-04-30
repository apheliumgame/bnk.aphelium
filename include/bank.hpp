#include <eosio/eosio.hpp>
#include "atomicassets.hpp"
#include "atomicdata.hpp"
#include <eosio/system.hpp>

using namespace std;
using namespace eosio;

CONTRACT bank : public contract {
    public:
        using contract::contract;
        
        struct NFT {
            int32_t template_id;
            uint64_t asset_id;
        };
        
        struct TOKEN {
            string symbol;
            string contract;
        };
        
        [[eosio::action]] void setconfig(string key, uint64_t uint_value, int32_t int_value, string string_value, vector<string> arr_value, vector<TOKEN> tkn_value);
        [[eosio::action]] void delconfig(string key);
        [[eosio::action]] void createbank(name player);
        [[eosio::action]] void usebanktkn(name player, asset quantity);
        [[eosio::action]] void usebanknft(name player, int32_t template_id, uint64_t quantity);
        [[eosio::action]] void withdrawtkn(name player, asset quantity);
        
        [[eosio::on_notify("atomicassets::logtransfer")]] void handlenfttransfer(name collection_name, name from, name to, vector<uint64_t> asset_ids, string memo);
        [[eosio::on_notify("*::transfer")]] void handletransfer (name from, name to, asset quantity, string memo);
    
    private:
        struct [[eosio::table]] config_s {
            name key;
            uint64_t uint_value;
            int32_t int_value;
            string string_value;
            vector<string> arr_value;
            vector<TOKEN> tkn_value;
            
            uint64_t primary_key() const { return key.value; }
        };
        typedef multi_index<name("config"), config_s> config_t;
        config_t configs = config_t(get_self(), get_self().value);
    
        struct [[eosio::table]] counters_s {
            name counter_name;
            uint64_t counter_value;
            
            uint64_t primary_key() const { return counter_name.value; }
        };
        typedef multi_index<name("counters"), counters_s> counters_t;
        counters_t counters = counters_t(get_self(), get_self().value);
        
        struct [[eosio::table]] balances_s {
            name owner;
            vector <asset> quantities;
            vector <NFT> nfts;
            
            uint64_t primary_key() const { return owner.value; }
        };
        typedef multi_index<name("balances"), balances_s> balances_t;
        balances_t balances = balances_t(get_self(), get_self().value);
};