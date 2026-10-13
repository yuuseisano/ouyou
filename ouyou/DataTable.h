#pragma once

#include <string>
#include <unordered_map>

// ステータス構造体
struct Stats
{
    int maxHp;
    int maxMp;
    int attack;
    int defense;
    int speed; // 行動順決定用
};

// データテーブル
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
            // { "Type", { maxHp, maxMp, attack, defense, speed } }
            { "Player",   {100, 30, 20, 5, 10} },
            { "Slime",    {30, 0, 5, 1, 8} },
            { "Goblin",   {50, 0, 12, 3, 12} },
            { "Skeleton", {40, 0, 8, 2, 9} },
            { "Dragon",   {200, 0, 35, 10, 6} },
            { "Default",  {10, 0, 1, 0, 5} }
        };
        return table;
    }
};
