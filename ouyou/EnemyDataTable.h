#pragma once

#include "EnemyStats.h"
#include <string>
#include <unordered_map>

// 敵専用の初期能力値を管理する。
class EnemyDataTable
{
public:
    static const EnemyStats& Get(const std::string& key)
    {
        const auto& table = GetTable();
        const auto it = table.find(key);
        // 未登録の敵には、従来と同じ既定値を使う。
        return it != table.end() ? it->second : table.at("Default");
    }

private:
    static const std::unordered_map<std::string, EnemyStats>& GetTable()
    {
        static const std::unordered_map<std::string, EnemyStats> table = {
            // 最大ＨＰ・最大ＭＰ・攻撃力・防御力・素早さ・報酬経験値の順。未設定の報酬は０。
            { "Slime",    {30, 0, 5, 1, 8, 5} },
            { "Goblin",   {50, 0, 12, 3, 12, 10} },
            { "Skeleton", {40, 0, 8, 2, 9, 8} },
            { "Dragon",   {200, 0, 35, 10, 30, 50} },
            { "Default",  {10, 0, 1, 0, 5, 1} }
        };
        return table;
    }
};
