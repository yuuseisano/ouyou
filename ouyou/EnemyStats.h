#pragma once

// 敵の能力値と討伐報酬。主人公のレベル情報は持たない。
struct EnemyStats
{
    int maxHp;
    int maxMp;
    int attack;
    int defense;
    int speed;
    int exp; // 倒したときに得られる経験値
};
