#pragma once

#include "HeroDataTable.h"
#include "EnemyDataTable.h"
#include <string>

// 用途ごとに異なる型を返し、主人公と敵の専用項目を失わずに取得する。
class DataTable
{
public:
    static const HeroStats& GetHero(const std::string& key)
    {
        return HeroDataTable::Get(key);
    }
    static const EnemyStats& GetEnemy(const std::string& key)
    {
        return EnemyDataTable::Get(key);
    }
};
