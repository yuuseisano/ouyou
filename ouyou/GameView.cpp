#include "GameView.h"
#include "Item.h"
#include <istream>
#include <ostream>
#include <sstream>
#include <iostream>
#include <cstdio>
#ifdef _WIN32
#include <conio.h>
#include <io.h>
#endif

GameView::GameView(std::istream& input, std::ostream& output) : mInput(input), mOutput(output) {}
void GameView::Message(const std::string& message) { mOutput << message << '\n'; }

int GameView::ReadLine(std::string& line)
{
    line.clear();
#ifdef _WIN32
    // 実コンソールだけキー単位で読み、EscをEnterなしで受け付ける。
    // テスト用ストリームやリダイレクト入力は通常の行読み取りを使う。
    if (mInput.rdbuf() == std::cin.rdbuf() && _isatty(_fileno(stdin)))
    {
        mOutput.flush();
        for (;;)
        {
            const int key = _getch();
            if (key == 0 || key == 224) { _getch(); continue; } // 矢印などの拡張キー
            if (key == 27) { mOutput << "\n"; return Cancelled; }
            if (key == 26 || key == 3) { mOutput << "\n"; return InputEnded; }
            if (key == '\r' || key == '\n') { mOutput << "\n"; return 1; }
            if (key == '\b')
            {
                if (!line.empty()) { line.pop_back(); mOutput << "\b \b" << std::flush; }
            }
            else if (key >= 32 && key < 127)
            {
                line += static_cast<char>(key);
                mOutput << static_cast<char>(key) << std::flush;
            }
        }
    }
#endif
    if (!std::getline(mInput, line)) return InputEnded;
    return line.find('\x1b') != std::string::npos ? Cancelled : 1;
}

int GameView::ReadChoice(const std::string& prompt, int minimum, int maximum)
{
    for (std::string line;;)
    {
        mOutput << prompt;
        // 入力終了を通知し、再入力を無限に繰り返さない。
        const int result = ReadLine(line);
        if (result <= 0) return result;
        std::istringstream stream(line);
        int value;
        // 整数の後ろは空白だけを許し、数字に文字が続く入力は拒否する。
        if ((stream >> value) && (stream >> std::ws).eof() && value >= minimum && value <= maximum)
            return value;
        mOutput << minimum << "～" << maximum << " の整数を入力してください。\n";
    }
}

bool GameView::ReadNextWave()
{
    Message("敵を全て倒しました！ スペースを入力して Enter で次の敵へ。その他は終了。 ");
    std::string line;
    // 行頭が半角スペースの場合だけ、次の戦闘へ進む。
    return ReadLine(line) > 0 && !line.empty() && line[0] == ' ';
}

void GameView::ShowActors(const std::vector<std::shared_ptr<Actor>>& actors)
{
    for (const auto& actor : actors)
    {
        mOutput << actor->GetName() << " [" << actor->GetType() << "] HP:" << actor->GetHp()
                << " MP:" << actor->GetMp() << " ATK:" << actor->GetAttack()
                << " DEF:" << actor->GetDefense() << " SPD:" << actor->GetSpeed()
                << (actor->IsDead() ? " (倒)" : "");
        if (const auto* hero = actor->GetHeroStats())
            mOutput << " Lv:" << hero->level << " EXP:" << hero->experience
                    << " 次のレベルまで:" << actor->GetExperienceToNextLevel();
        mOutput << '\n';
    }
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
    return ReadChoice("対象番号（Escでコマンドに戻る）> ", 1, static_cast<int>(enemies.size()));
}

int GameView::ReadSpell(const Actor& actor)
{
    const auto& spells = GetSpellBook();
    std::vector<size_t> knownSpells;
    for (size_t i = 0; i < spells.size(); ++i)
    {
        if (!actor.KnowsSpell(i)) continue;
        knownSpells.push_back(i);
        mOutput << knownSpells.size() << ") " << spells[i].name << " MP:" << spells[i].cost
                << " 威力:" << spells[i].power;
        if (!spells[i].bonusTarget.empty()) mOutput << " 対ドラゴン威力２倍";
        mOutput << '\n';
    }
    if (knownSpells.empty()) return 0;
    const int choice = ReadChoice("魔法番号（Escでコマンドに戻る）> ", 1, static_cast<int>(knownSpells.size()));
    // 入力終了・キャンセルは添字に変換せず、そのまま返す。
    return choice > 0 ? static_cast<int>(knownSpells[choice - 1]) + 1 : choice;
}

int GameView::ReadItem(const Actor& actor)
{
    const auto& items = actor.GetInventory();
    for (size_t i = 0; i < items.size(); ++i)
        mOutput << i + 1 << ") " << items[i].item->GetName() << " 所持数:" << items[i].count << " 回復量:" << items[i].item->GetValue() << '\n';
    return ReadChoice("アイテム番号（Escでコマンドに戻る）> ", 1, static_cast<int>(items.size()));
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
