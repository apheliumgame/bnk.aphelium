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
            uint8_t precision;
        };
        
        auto incrementcounter(name counter_name);
        void savetokentx(name player, asset quantity, string operation);
        
        /**
         * Set configuration action
         * 
         * @details Allows contract's account to insert/update a config parameter
         * @param key - the name of the config
         * @param uint_value - if the config value has to be uint, here it goes the value
         * @param int_value - if the config value has to be int32, here it goes the value
         * @param string_value - if the config value has to be a string, here it goes the value
         * @param arr_value - if the config value has to be an array of strings, here it goes the value
         * @param tkn_value - if the config is a vactor of TOKEN, here it goes the value
         * 
         * If validation is successful, config is set
         */
        [[eosio::action]] void setconfig(string key, uint64_t uint_value, int32_t int_value, string string_value, vector<string> arr_value, vector<TOKEN> tkn_value);
        
        /**
         * Delete config action
         * 
         * @details Allows contract's account to delete e config parameter
         * @param key - the name of the config
         * 
         * @pre Config name must exists
         * 
         * If validation is successful, config is deleted
         */
        [[eosio::action]] void delconfig(string key);
        
        /**
         * Create bank action
         * 
         * @details Allows `player` to create a bank account if it does not exists yet
         * @param player - the wallet which wants to create the bank account
         * 
         * @pre A bank must not exists already
         * 
         * If validation is successful, a new bank account is created
         */
        [[eosio::action]] void createbank(name player);
        
        /**
         * Use bank tokens action
         * 
         * @details Allows contract's account to use `quantity` and sends it to another account specified in config `bankacc`
         * @param player - the wallet owner of the bank
         * @param quantity - the asset to use
         * 
         * @pre Bank account must exists
         * @pre `allowedtkns` config must be set
         * @pre `quantity` must be <= the balance of the account in the bank, for that token
         * @pre Token must be present in `allowedtkns` config
         * @pre `bankacc` config must be set
         */
        [[eosio::action]] void usebanktkn(name player, asset quantity);
        
        /**
         * Use bank NFTs action
         * 
         * @details Allows contract's account to use `quantity` NFTs of the specified `template_id` and burns them
         * @param player - the wallet owner of the bank
         * @param template_id - the template id of the NFTs to use
         * @param quantity - the amount of the NFTs to use
         * 
         * @pre Bank account must exists
         * @pre `player` must have enough NFTs in the bank to fullfil `quantity`
         */
        [[eosio::action]] void usebanknft(name player, int32_t template_id, uint64_t quantity);
        
        /**
         * Withdraw token action
         * 
         * @details Allows `player` to withdraw `quantity` from their bank
         * @param player - the wallet owner of the bank
         * @param quantity - the asset to withdraw
         * 
         * @pre Bank account must exists
         * @pre `quantity` must be <= the balance of the account in the bank, for that token
         */
        [[eosio::action]] void withdrawtkn(name player, asset quantity);
        
        /**
         * Withdraw NFTs action
         * 
         * @details Allows `player` to withdraw `quantity` of `template_id` NFTs from their bank
         * @param player - the wallet owner of the bank
         * @param template_id - the template id of the NFTs to withdraw
         * @param quantity - the amount of the NFTs to withdraw
         * 
         * @pre Bank account must exists
         * @pre `player` must have enough NFTs in the bank to fullfil `quantity`
         */
        [[eosio::action]] void withdrawnft(name player, int32_t template_id, uint64_t quantity);
        
        /**
         * Delete bank action
         * 
         * @details Allows `player` to delete their bank account
         * @param player - the wallet owner of the bank
         * 
         * @pre Bank account must exists
         */
        [[eosio::action]] void delbank(name player);
        
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
        
        struct [[eosio::table]] tokentx_s {
            uint64_t id;
            uint32_t created_at;
            asset quantity;
            string operation;
            
            uint64_t primary_key() const { return id; }
        };
        typedef multi_index<name("tokentx"), tokentx_s> tokentx_t;
        tokentx_t get_tokentx(name player) {
            return tokentx_t(get_self(), player.value);
        }
};