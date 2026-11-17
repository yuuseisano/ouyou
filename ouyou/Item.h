#pragma once

#include "Actor.h"
#include <string>
#include <memory>
#include <random>


// シンプルなアイテム定義（ヘッダ一本で完結）
class Actor; // 前方宣言（Actor.h をインクルードしていますが念のため）

enum class ItemType
{
    HP_Potion, // HP回復
    MP_Potion   // MP回復
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
        // ポーション40％、エーテル20％、ドロップなし40％。呼び出し側で空のポインターを確認する。
        static std::mt19937& rng = *([]()->std::mt19937* { static std::mt19937 r(static_cast<unsigned int>(std::random_device{}())); return &r; })();
        std::uniform_int_distribution<int> d(1, 100);
        int v = d(rng);
        if (v <= 40) return std::make_shared<Item>(ItemType::HP_Potion, "Potion", 30); // HP 30 回復
        if (v <= 60) return std::make_shared<Item>(ItemType::MP_Potion, "Ether", 20);   // MP 20 回復
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
    if (mType == ItemType::HP_Potion)
    {
        user.mHp += mValue;
        if (user.mHp > user.GetMaxHp()) user.mHp = user.GetMaxHp();
    }
    else if (mType == ItemType::MP_Potion)
    {
        user.mMp += mValue;
        if (user.mMp > user.GetMaxMp()) user.mMp = user.GetMaxMp();
    }
}
