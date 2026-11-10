#include "GameView.h"
#include "Item.h"
#include <istream>
#include <ostream>
#include <sstream>

GameView::GameView(std::istream& input, std::ostream& output) : mInput(input), mOutput(output) {}
void GameView::Message(const std::string& message) { mOutput << message << '\n'; }

int GameView::ReadChoice(const std::string& prompt, int minimum, int maximum)
{
    for (std::string line;;)
    {
        mOutput << prompt;
        if (!std::getline(mInput, line)) return 0;
        std::istringstream stream(line);
        int value;
        if ((stream >> value) && (stream >> std::ws).eof() && value >= minimum && value <= maximum)
            return value;
        mOutput << minimum << "～" << maximum << " の整数を入力してください。\n";
    }
}

bool GameView::ReadNextWave()
{
    Message("敵を全て倒しました！ スペースを入力して Enter で次の敵へ。その他は終了。 ");
    std::string line;
    return std::getline(mInput, line) && !line.empty() && line[0] == ' ';
}

void GameView::ShowActors(const std::vector<std::shared_ptr<Actor>>& actors)
{
    for (const auto& actor : actors)
        mOutput << actor->GetName() << " [" << actor->GetType() << "] HP:" << actor->GetHp()
                << " MP:" << actor->GetMp() << " ATK:" << actor->GetAttack()
                << " DEF:" << actor->GetDefense() << " SPD:" << actor->GetSpeed()
                << (actor->IsDead() ? " (倒)" : "") << '\n';
}

void GameView::ShowTurn(int turn) { mOutput << "\n--- ターン " << turn << " ---\n"; }
int GameView::ReadAction(const Actor& actor)
{
    mOutput << "\nあなたの番: " << actor.GetName() << " HP:" << actor.GetHp()
            << " MP:" << actor.GetMp() << " SPD:" << actor.GetSpeed() << '\n';
    Message("1) 攻撃  2) 魔法  3) アイテム  4) ステータス  5) 待機  6) 終了");
    return ReadChoice("選択> ", 1, 6);
}

int GameView::ReadTarget(const std::vector<std::shared_ptr<Actor>>& enemies)
{
    for (size_t i = 0; i < enemies.size(); ++i)
        mOutput << i + 1 << ") " << enemies[i]->GetName() << " HP:" << enemies[i]->GetHp()
                << " SPD:" << enemies[i]->GetSpeed() << '\n';
    return ReadChoice("対象番号> ", 1, static_cast<int>(enemies.size()));
}

int GameView::ReadSpell()
{
    const auto& spells = GetSpellBook();
    for (size_t i = 0; i < spells.size(); ++i)
        mOutput << i + 1 << ") " << spells[i].name << " MP:" << spells[i].cost << " 威力:" << spells[i].power << '\n';
    return ReadChoice("魔法番号> ", 1, static_cast<int>(spells.size()));
}

int GameView::ReadItem(const Actor& actor)
{
    const auto& items = actor.GetInventory();
    for (size_t i = 0; i < items.size(); ++i)
        mOutput << i + 1 << ") " << items[i]->GetName() << " 回復量:" << items[i]->GetValue() << '\n';
    return ReadChoice("アイテム番号> ", 1, static_cast<int>(items.size()));
}

void GameView::ShowAttack(const Actor& attacker, const Actor& target, int damage, const std::string& spell)
{
    mOutput << attacker.GetName() << " → " << target.GetName() << " "
            << (spell.empty() ? "攻撃" : spell) << " ダメージ:" << damage
            << " 残りHP:" << target.GetHp() << '\n';
    if (target.IsDead()) Message(target.GetName() + " は倒れた。");
}

void GameView::ShowRecovery(const Actor& actor, int hp, int mp)
{
    mOutput << actor.GetName() << " HP回復:" << hp << " MP回復:" << mp
            << " 現在HP:" << actor.GetHp() << " 現在MP:" << actor.GetMp() << '\n';
}
