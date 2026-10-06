#include "Actor.h"
#include <iostream>

// 簡易状態：Idle / Attack / Dead
class IdleState : public State<Actor>
{
public:
    void Enter(Actor& owner) override
    {
        // nothing
    }
    void Execute(Actor& owner) override
    {
        // 待機中のログ（省略可）
    }
    void Exit(Actor& owner) override {}
};

class AttackState : public State<Actor>
{
public:
    void Enter(Actor& owner) override {}
    void Execute(Actor& owner) override {}
    void Exit(Actor& owner) override {}
};

class DeadState : public State<Actor>
{
public:
    void Enter(Actor& owner) override
    {
        std::cout << owner.GetName() << " は倒れた。\n";
    }
    void Execute(Actor& owner) override {}
    void Exit(Actor& owner) override {}
};

Actor::Actor(const std::string& type, const std::string& name)
    : mType(type)
    , mName(name)
    , mStats(DataTable::Get(type))
    , mHp(mStats.maxHp)
    , mStateMachine(*this)
{
    // 初期状態は Idle
    mStateMachine.ChangeState(std::make_shared<IdleState>());
}

void Actor::Update()
{
    if (IsDead())
    {
        mStateMachine.ChangeState(std::make_shared<DeadState>());
    }
    mStateMachine.Update();
}

void Actor::Attack(Actor& target)
{
    if (IsDead())
    {
        std::cout << mName << " は行動できない。\n";
        return;
    }

    int damage = std::max(0, GetAttack() - target.GetDefense());
    std::cout << mName << " が " << target.GetName() << " に攻撃！ ダメージ " << damage << "\n";
    target.TakeDamage(damage);

    // 攻撃時に自分を AttackState にしておく（デモ用）
    mStateMachine.ChangeState(std::make_shared<AttackState>());
}

void Actor::TakeDamage(int amount)
{
    mHp -= amount;
    if (mHp < 0) mHp = 0;
    std::cout << mName << " の残りHP: " << mHp << "\n";
    if (IsDead())
    {
        mStateMachine.ChangeState(std::make_shared<DeadState>());
    }
}
