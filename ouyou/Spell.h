#pragma once

#include <string>
#include <vector>

// 単純なスペル定義
struct Spell
{
    std::string name;
    int cost;   // MP 消費
    int power;  // 威力（魔法ダメージ）
    int requiredLevel = 1; // 習得レベル
    std::string bonusTarget; // 特効対象の種類。空なら特効なし。

    int GetDamage(const std::string& targetType) const
    {
        return !bonusTarget.empty() && targetType == bonusTarget ? power * 2 : power;
    }
};

// 選択画面で使う順序で、魔法名・消費ＭＰ・威力の一覧を返す。
inline const std::vector<Spell>& GetSpellBook()
{
    static std::vector<Spell> spells = {
        { "Water",   5,  10 },  // 弱い魔法
        { "Fire",   10,  25 },
        { "Thunder",15,  45 },
        { "Meteor", 25,  90 },   // 強力な魔法
        { "ドラゴンブレイカー", 30, 120, 10, "Dragon" } // ドラゴンに２倍の威力
    };
    return spells;
}
