#pragma once

#include "HeroStats.h"
#include <string>
#include <unordered_map>

// 主人公専用の初期能力値を管理する。
class HeroDataTable
{
public:
    static const HeroStats& Get(const std::string& key)
    {
        const auto& table = GetTable();
        const auto it = table.find(key);
        // 未登録の主人公には、主人公用の既定値を使う。
        return it != table.end() ? it->second : table.at("Default");
    }

private:
    static const std::unordered_map<std::string, HeroStats>& GetTable()
    {
        static const std::unordered_map<std::string, HeroStats> table = {
            // 最大ＨＰ・最大ＭＰ・攻撃力・防御力・素早さ・初期レベルの順。
            { "Player",  {100, 30, 20, 5, 10, 1} },
            { "Default", {100, 30, 20, 5, 10, 1} }
        };
        return table;
    }
};
