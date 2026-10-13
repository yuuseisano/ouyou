#pragma once

#include "Actor.h"
#include <string>
#include <memory>
#include <random>
#include <iostream>

// シンプルなアイテム定義（ヘッダ一本で完結）
class Actor; // 前方宣言（Actor.h をインクルードしていますが念のため）

enum class ItemType
{
    Potion, // HP回復
    Ether   // MP回復
};

class Item
{
public:
    Item(ItemType type, const std::string& name, int value)
        : mType(type), mName(name), mValue(value) {}

    const std::string& GetName() const { return mName; }
    ItemType GetType() const { return mType; }
    int GetValue() const { return mValue; }

    // アイテムを使用（Actor 側が効果を受け取る）
    void Use(Actor& user) const;

    // ランダムドロップ生成ユーティリティ
    static std::shared_ptr<Item> CreateRandomDrop()
    {
        // Drop率: 60%で何かドロップ（Potion:40%, Ether:20%），40%は何も落とさない（呼び出し側で nullptr をチェック）
        static std::mt19937& rng = *([]()->std::mt19937* { static std::mt19937 r(static_cast<unsigned int>(std::random_device{}())); return &r; })();
        std::uniform_int_distribution<int> d(1, 100);
        int v = d(rng);
        if (v <= 40) return std::make_shared<Item>(ItemType::Potion, "Potion", 30); // HP 30 回復
        if (v <= 60) return std::make_shared<Item>(ItemType::Ether, "Ether", 20);   // MP 20 回復
        return nullptr;
    }

private:
    ItemType mType;
    std::string mName;
    int mValue;
};

// Item::Use の実装
inline void Item::Use(Actor& user) const
{
    if (mType == ItemType::Potion)
    {
        int before = user.mHp;
        user.mHp += mValue;
        if (user.mHp > user.mStats.maxHp) user.mHp = user.mStats.maxHp;
        int healed = user.mHp - before;
        std::cout << user.GetName() << " は " << healed << " HP 回復した。現在HP: " << user.mHp << "\n";
    }
    else if (mType == ItemType::Ether)
    {
        int before = user.mMp;
        user.mMp += mValue;
        if (user.mMp > user.mStats.maxMp) user.mMp = user.mStats.maxMp;
        int recovered = user.mMp - before;
        std::cout << user.GetName() << " は " << recovered << " MP 回復した。現在MP: " << user.mMp << "\n";
    }
}
