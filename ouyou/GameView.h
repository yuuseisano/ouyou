#pragma once
#include "Actor.h"
#include <iosfwd>
#include <string>

// View: all console input, validation, and presentation.
class GameView
{
public:
    GameView(std::istream& input, std::ostream& output);
    void Message(const std::string& message);
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
