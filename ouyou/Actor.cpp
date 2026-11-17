#include "Actor.h"
#include "Item.h"

#include <algorithm>

// 待機・攻撃・死亡を表す簡易状態。各状態の開始・実行・終了処理は現在空。
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
    , mHeroStats(type == "Player" ? std::make_unique<HeroStats>(DataTable::GetHero(type)) : nullptr)
    , mEnemyStats(type != "Player" ? std::make_unique<EnemyStats>(DataTable::GetEnemy(type)) : nullptr)
    , mHp(GetMaxHp())
    , mMp(GetMaxMp())
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
    if (!KnowsSpell(spellIndex))
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
    int damage = sp.GetDamage(target.GetType());
    target.TakeDamage(damage);

    // 魔法使用後は AttackState に（簡易）
    mStateMachine.ChangeState(std::make_shared<AttackState>());
    return true;
}

void Actor::AddItem(std::shared_ptr<Item> item)
{
    if (!item) return;
    for (auto& stack : mInventory)
    {
        // 表示名が違っても、種類と回復量が同じなら同じ枠へ加算する。
        if (stack.item->GetType() == item->GetType() && stack.item->GetValue() == item->GetValue())
        {
            ++stack.count;
            return;
        }
    }
    mInventory.push_back({ item, 1 });
}

bool Actor::UseItem(size_t index)
{
    if (index >= mInventory.size()) return false;
    auto& stack = mInventory[index];
    auto item = stack.item;
    if (!item) return false;

    // アイテム効果を適用
    item->Use(*this);

    // １個だけ消費し、残数が０になった枠を削除する。
    if (--stack.count == 0)
        mInventory.erase(mInventory.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

int Actor::GetExperienceToNextLevel() const
{
    if (!mHeroStats || mHeroStats->level >= 99) return 0;
    return mHeroStats->level * 20 - mHeroStats->experience;
}

int Actor::AddExperience(int amount)
{
    if (!mHeroStats || IsDead() || amount <= 0 || mHeroStats->level >= 99) return 0;
    auto& stats = *mHeroStats;
    const int previousLevel = stats.level;
    // 大量の経験値でも加算があふれないよう、計算には広い整数型を使う。
    long long experience = static_cast<long long>(stats.experience) + amount;
    while (stats.level < 99 && experience >= stats.level * 20)
    {
        experience -= stats.level * 20;
        ++stats.level;
        stats.maxHp += 10;
        stats.maxMp += 5;
        stats.attack += 3;
        stats.defense += 2;
        stats.speed += 1;
    }
    stats.experience = stats.level == 99 ? 0 : static_cast<int>(experience);
    if (stats.level != previousLevel)
    {
        // レベルアップ時だけ全回復する。経験値の獲得だけでは回復しない。
        mHp = stats.maxHp;
        mMp = stats.maxMp;
    }
    return stats.level - previousLevel;
}

int Actor::GainExperienceFrom(Actor& enemy)
{
    if (!mHeroStats || IsDead() || !enemy.mEnemyStats || !enemy.IsDead()
        || enemy.mHasGrantedExperience) return 0;
    enemy.mHasGrantedExperience = true;
    const int reward = std::max(0, enemy.mEnemyStats->exp);

    AddExperience(reward);
    return reward;
}

bool Actor::KnowsSpell(size_t spellIndex) const
{
    const auto& spells = GetSpellBook();
    const int level = mHeroStats ? mHeroStats->level : 1;
    return spellIndex < spells.size() && level >= spells[spellIndex].requiredLevel;
}

void Actor::ScaleEnemyStats(int heroLevel)
{
    if (!mEnemyStats || IsDead()) return;
    const int stage = std::max(1, std::min(99, heroLevel)) / 5;
    const auto& base = DataTable::GetEnemy(mType);
    const int oldMaxHp = mEnemyStats->maxHp;
    // 毎回基礎値から計算し、同じ段階を繰り返し適用しても強くなり続けない。
    mEnemyStats->maxHp = base.maxHp + std::max(1, base.maxHp / 10) * stage;
    mEnemyStats->maxMp = base.maxMp + (base.maxMp / 10) * stage;
    mEnemyStats->attack = base.attack + stage;
    mEnemyStats->defense = base.defense + stage;
    mEnemyStats->speed = base.speed + stage;
    // 最大ＨＰの増加分だけ現在ＨＰも増やし、受けたダメージは維持する。
    mHp = std::max(1, std::min(mEnemyStats->maxHp, mHp + mEnemyStats->maxHp - oldMaxHp));
}
