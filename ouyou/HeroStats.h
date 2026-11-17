#pragma once

// 主人公の能力値と成長情報。敵の報酬データとは別の型で管理する。
struct HeroStats
{
    int maxHp;
    int maxMp;
    int attack;
    int defense;
    int speed;
    int level; // 現在のレベル（最大99）
    int experience; // 次のレベルに向けて蓄積した経験値
};
