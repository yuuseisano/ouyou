#pragma once
#include "Actor.h"
#include <iosfwd>
#include <string>

// コンソール表示、入力の受け取り、入力形式と範囲の検証を担当する。
class GameView
{
public:
    GameView(std::istream& input, std::ostream& output);
    void Message(const std::string& message);
    // 指定範囲の整数を受け付け、入力が終了した場合は 0 を返す。
    int ReadChoice(const std::string& prompt, int minimum, int maximum);
    bool ReadNextWave();
    void ShowActors(const std::vector<std::shared_ptr<Actor>>& actors);
    void ShowTurn(int turn);
    int ReadAction(const Actor& actor);
    int ReadTarget(const std::vector<std::shared_ptr<Actor>>& enemies);
    int ReadSpell();
    int ReadItem(const Actor& actor);
    void ShowAttack(const Actor& attacker, const Actor& target, int damage, const std::string& spell = "");
    void ShowRecovery(const Actor& actor, int hp, int mp);
private:
    std::istream& mInput;
    std::ostream& mOutput;
};
