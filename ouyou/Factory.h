#pragma once

#include <memory>
#include <string>
#include "Actor.h"

// シンプルなファクトリ
class ActorFactory
{
public:
    // インスタンス化は禁止し、静的な生成関数だけを使う。
    ActorFactory() = delete;

    static std::shared_ptr<Actor> Create(const std::string& type)
    {
        // 種類に応じた名前を付けて生成し、敵には種類ごとの連番を付ける。
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
