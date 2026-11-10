#include "GameController.h"
#include "Item.h"
#include <cassert>
#include <sstream>

int main()
{
    GameModel model;
    model.Initialize();
    assert(model.GetActors().size() == 3);
    auto player = model.GetPlayer();
    auto enemy = std::make_shared<Actor>("Slime", "test");
    player->Attack(*enemy);
    assert(enemy->GetHp() == 11);
    assert(player->CastSpell(1, *enemy));
    assert(enemy->IsDead() && player->GetMp() == 20);
    assert(!player->CastSpell(99, *enemy));
    player->TakeDamage(40);
    player->AddItem(std::make_shared<Item>(ItemType::Potion, "Potion", 30));
    assert(player->UseItem(0) && player->GetHp() == 90);
    assert(player->GetInventory().empty());
    model.AddActor(enemy);
    model.RemoveDeadActors();
    assert(model.GetActors().size() == 3);
    const auto order = model.GetTurnOrder();
    for (size_t i = 1; i < order.size(); ++i)
        assert(order[i - 1]->GetSpeed() >= order[i]->GetSpeed());
    model.Initialize();
    assert(model.GetActors().size() == 3);

    std::istringstream invalid("x\n2abc\n0\n2\n");
    std::ostringstream output;
    GameView view(invalid, output);
    assert(view.ReadChoice("", 1, 6) == 2);
    assert(view.ReadChoice("", 1, 6) == 0);
    std::istringstream quit("6\n");
    GameView quitView(quit, output);
    GameController controller(model, quitView);
    controller.Run();
    std::istringstream eof;
    GameView eofView(eof, output);
    GameController eofController(model, eofView);
    eofController.Run();
}
