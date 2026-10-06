#pragma once

#include <string>
#include <unordered_map>

// シンプルなステータス定義
struct Stats
{
    int maxHp;
    int attack;
    int defense;
    int speed; // 追加：速さ（ターン順に影響）
};

// データテーブル（ヘッダオンリー）
class DataTable
{
public:
    static const Stats& Get(const std::string& key)
    {
        auto it = GetTable().find(key);
        if (it != GetTable().end()) return it->second;
        return GetTable().at("Default");
    }

private:
    static const std::unordered_map<std::string, Stats>& GetTable()
    {
        static std::unordered_map<std::string, Stats> table = {
            { "Player",   {100, 20, 5, 10} },
            { "Slime",    {30, 5, 1, 8} },


            { "Goblin",   {50, 12, 3, 12} },   // 速い敵（プレイヤーより先に動く可能性あり）
            { "Skeleton", {40, 8, 2, 9} },
            { "Dragon",   {200, 35, 10, 6} },
            { "Default",  {10, 1, 0, 5} }
        };
        return table;
    }
};
