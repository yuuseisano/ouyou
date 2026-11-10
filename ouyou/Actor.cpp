#include "Actor.h"
#include "Item.h"

#include <algorithm>

// 簡易状態：Idle / Attack / Dead
class IdleState : public State<Actor>
{
public:
    void Enter(Actor& owner) override { }
    void Execute(Actor& owner) override { }
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
    }
    void Execute(Actor& owner) override {}
    void Exit(Actor& owner) override {}
};

Actor::Actor(const std::string& type, const std::string& name)
    : mType(type)
    , mName(name)
    , mStats(DataTable::Get(type))
    , mHp(mStats.maxHp)
    , mMp(mStats.maxMp)
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

int Actor::Attack(Actor& target)
{
    if (IsDead())
    {
        return 0;
    }

    int damage = std::max(0, GetAttack() - target.GetDefense());
    target.TakeDamage(damage);

    // 攻撃時に自分を AttackState にしておく（デモ用）
    mStateMachine.ChangeState(std::make_shared<AttackState>());
    return damage;
}

void Actor::TakeDamage(int amount)
{
    mHp -= amount;
    if (mHp < 0) mHp = 0;
    if (IsDead())
    {
        mStateMachine.ChangeState(std::make_shared<DeadState>());
    }
}

bool Actor::CastSpell(size_t spellIndex, Actor& target)
{
    const auto& spells = GetSpellBook();
    if (spellIndex >= spells.size())
    {
        return false;
    }

    const Spell& sp = spells[spellIndex];

    if (IsDead())
    {
        return false;
    }

    if (mMp < sp.cost)
    {
        return false;
    }

    mMp -= sp.cost;

    // 魔法ダメージは装甲を無視する（設計上の選択）。必要なら target.GetDefense() を引く。
    int damage = sp.power;
    target.TakeDamage(damage);

    // 魔法使用後は AttackState に（簡易）
    mStateMachine.ChangeState(std::make_shared<AttackState>());
    return true;
}

void Actor::AddItem(std::shared_ptr<Item> item)
{
    if (!item) return;
    mInventory.push_back(item);
}

bool Actor::UseItem(size_t index)
{
    if (index >= mInventory.size()) return false;
    auto item = mInventory[index];
    if (!item) return false;

    // アイテム効果を適用
    item->Use(*this);

    // 使用後はインベントリから削除
    mInventory.erase(mInventory.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}
