#pragma once

#include <string>
#include <vector>

// 単純なスペル定義
struct Spell
{
    std::string name;
    int cost;   // MP 消費
    int power;  // 威力（魔法ダメージ）
};

// 弱い魔法から強い魔法までを返すユーティリティ
inline const std::vector<Spell>& GetSpellBook()
{
    static std::vector<Spell> spells = {
        { "Spark",   5,  10 },  // 弱い魔法
        { "Fire",   10,  25 },
        { "Thunder",15,  45 },
        { "Meteor", 25,  90 }   // 強力な魔法
    };
    return spells;
}
