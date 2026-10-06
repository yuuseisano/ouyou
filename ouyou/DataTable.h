#pragma once

#include <string>
#include <unordered_map>

// シンプルなステータス定義
struct Stats
{
    int maxHp;
    int attack;
    int defense;
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
            { "Player", {100, 20, 5} },
            { "Slime",  {30, 5, 1} },
            { "Goblin", {50, 12, 3} },
            { "Default",{10, 1, 0} }
        };
        return table;
    }
};
