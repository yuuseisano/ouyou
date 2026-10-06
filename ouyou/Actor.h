#pragma once

#include <string>
#include <memory>
#include "State.h"
#include "DataTable.h"

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

    // 速さを返す
    int GetSpeed() const { return mStats.speed; }

    // FSM へアクセス（テストや外部操作用）
    StateMachine<Actor>& GetStateMachine() { return mStateMachine; }

    int GetHp() const { return mHp; }
    int GetAttack() const { return mStats.attack; }
    int GetDefense() const { return mStats.defense; }

private:
    std::string mType;
    std::string mName;
    Stats mStats;
    int mHp;

    // Actor に紐づく状態機械
    StateMachine<Actor> mStateMachine;
};

