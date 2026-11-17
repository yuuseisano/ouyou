#pragma once

#include <string>
#include <memory>
#include <vector>
#include "State.h"
#include "DataTable.h"
#include "Spell.h"

class Item; // 前方宣言

// 能力値、所持品、戦闘処理、現在の状態を保持するキャラクター。
class Actor
{
public:
    Actor(const std::string& type, const std::string& name);
    virtual ~Actor() = default;

    void Update();
    int Attack(Actor& target);
    void TakeDamage(int amount);

    bool IsDead() const { return mHp <= 0; }
    const std::string& GetType() const { return mType; }
    const std::string& GetName() const { return mName; }

    // ステータス取得
    int GetSpeed() const { return (mHeroStats ? mHeroStats->speed : mEnemyStats->speed); }
    int GetHp() const { return mHp; }
    int GetAttack() const { return (mHeroStats ? mHeroStats->attack : mEnemyStats->attack); }
    int GetDefense() const { return (mHeroStats ? mHeroStats->defense : mEnemyStats->defense); }

    // 自分の種類に対応するステータスだけを返す。異なる種類なら nullptr。
    const HeroStats* GetHeroStats() const { return mHeroStats.get(); }
    const EnemyStats* GetEnemyStats() const { return mEnemyStats.get(); }
    int GetMaxHp() const { return mHeroStats ? mHeroStats->maxHp : mEnemyStats->maxHp; }

    // MP 関連
    int GetMp() const { return mMp; }
    int GetMaxMp() const { return (mHeroStats ? mHeroStats->maxMp : mEnemyStats->maxMp); }

    // 魔法を撃つ（スペル番号を指定）
    bool CastSpell(size_t spellIndex, Actor& target);

    // アイテム管理
    void AddItem(std::shared_ptr<Item> item);
    const std::vector<std::shared_ptr<Item>>& GetInventory() const { return mInventory; }
    bool UseItem(size_t index); // 使用に成功したら true を返す

    // 状態機械（待機・攻撃・死亡の状態を管理）
    StateMachine<Actor>& GetStateMachine() { return mStateMachine; }

private:
    std::string mType;
    std::string mName;
    // コンストラクターで種類に合う片方だけを生成する。
    std::unique_ptr<HeroStats> mHeroStats;
    std::unique_ptr<EnemyStats> mEnemyStats;
    int mHp;
    int mMp;

    // アイテム所持
    std::vector<std::shared_ptr<Item>> mInventory;

    // ドロップ処理済みの記録用（現在の処理では未使用）
    bool mHasDropped = false;

    // このアクターの状態機械
    StateMachine<Actor> mStateMachine;

    friend class Item; // Item::Use が内部へアクセスするため（簡潔化のため）
};
