#pragma once

#include <string>
#include <memory>
#include <vector>
#include "State.h"
#include "DataTable.h"
#include "Spell.h"

class Item; // 前方宣言

class Actor
{
public:
    Actor(const std::string& type, const std::string& name);
    virtual ~Actor() = default;

    void Update();
    void Attack(Actor& target);
    void TakeDamage(int amount);

    bool IsDead() const { return mHp <= 0; }
    const std::string& GetType() const { return mType; }
    const std::string& GetName() const { return mName; }

    // ステータス取得
    int GetSpeed() const { return mStats.speed; }
    int GetHp() const { return mHp; }
    int GetAttack() const { return mStats.attack; }
    int GetDefense() const { return mStats.defense; }

    // MP 関連
    int GetMp() const { return mMp; }
    int GetMaxMp() const { return mStats.maxMp; }

    // 魔法を撃つ（スペル番号を指定）
    bool CastSpell(size_t spellIndex, Actor& target);

    // アイテム管理
    void AddItem(std::shared_ptr<Item> item);
    const std::vector<std::shared_ptr<Item>>& GetInventory() const { return mInventory; }
    bool UseItem(size_t index); // 使用に成功したら true を返す

    // FSM
    StateMachine<Actor>& GetStateMachine() { return mStateMachine; }

private:
    std::string mType;
    std::string mName;
    Stats mStats;
    int mHp;
    int mMp;

    // アイテム所持
    std::vector<std::shared_ptr<Item>> mInventory;

    // ドロップ済みフラグ（倒された敵が複数回処理されないように）
    bool mHasDropped = false;

    // Actor 用 FSM
    StateMachine<Actor> mStateMachine;

    friend class Item; // Item::Use が内部へアクセスするため（簡潔化のため）
};

