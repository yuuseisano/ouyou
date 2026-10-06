#pragma once

#include <memory>
#include <string>
#include "Actor.h"

// シンプルなファクトリ
class ActorFactory
{
public:
    // コンストラクタ非公開（static メソッドのみ使用）
    ActorFactory() = delete;

    static std::shared_ptr<Actor> Create(const std::string& type)
    {
        // コンストラクタを使って生成（要求のコンストラクタ使用）
        if (type == "Player")
        {
            return std::make_shared<Actor>("Player", "主人公");
        }
        else if (type == "Slime")
        {
            static int slimeId = 1;
            return std::make_shared<Actor>("Slime", "スライム#" + std::to_string(slimeId++));
        }
        else if (type == "Goblin")
        {
            static int goblinId = 1;
            return std::make_shared<Actor>("Goblin", "ゴブリン#" + std::to_string(goblinId++));
        }
        else if (type == "Skeleton")
        {
            static int skeletonId = 1;
            return std::make_shared<Actor>("Skeleton", "スケルトン#" + std::to_string(skeletonId++));
        }
        else if (type == "Dragon")
        {
            static int dragonId = 1;
            return std::make_shared<Actor>("Dragon", "ドラゴン#" + std::to_string(dragonId++));
        }
        // デフォルト
        return std::make_shared<Actor>("Default", "無名");
    }
};
