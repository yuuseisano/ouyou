#include "GameController.h"
#include "Item.h"
#include <cassert>
#include <sstream>

int main()
{
    // 初期配置が主人公１人と敵２体になることを確認する。
    GameModel model;
    model.Initialize();
    assert(model.GetActors().size() == 3);
    auto player = model.GetPlayer();
    auto enemy = std::make_shared<Actor>("Slime", "test");
    // 通常攻撃の防御力計算、魔法の威力とＭＰ消費、不正な魔法番号を確認する。
    player->Attack(*enemy);
    assert(enemy->GetHp() == 11);
    assert(player->CastSpell(1, *enemy));
    assert(enemy->IsDead() && player->GetMp() == 20);
    assert(!player->CastSpell(99, *enemy));
    // 回復効果と、使用したアイテムが所持品から削除されることを確認する。
    player->TakeDamage(40);
    player->AddItem(std::make_shared<Item>(ItemType::Potion, "Potion", 30));
    assert(player->UseItem(0) && player->GetHp() == 90);
    assert(player->GetInventory().empty());
    // 死亡した敵の削除と、素早さの降順での行動を確認する。
    model.AddActor(enemy);
    model.RemoveDeadActors();
    assert(model.GetActors().size() == 3);
    const auto order = model.GetTurnOrder();
    for (size_t i = 1; i < order.size(); ++i)
        assert(order[i - 1]->GetSpeed() >= order[i]->GetSpeed());
    model.Initialize();
    assert(model.GetActors().size() == 3);

    // 文字列、末尾の余分な文字、範囲外の値を拒否し、入力終了を検出する。
    std::istringstream invalid("x\n2abc\n0\n2\n");
    std::ostringstream output;
    GameView view(invalid, output);
    assert(view.ReadChoice("", 1, 6) == 2);
    assert(view.ReadChoice("", 1, 6) == 0);
    // 終了コマンドと入力終了のどちらでもゲームが終了することを確認する。
    std::istringstream quit("6\n");
    GameView quitView(quit, output);
    GameController controller(model, quitView);
    controller.Run();
    std::istringstream eof;
    GameView eofView(eof, output);
    GameController eofController(model, eofView);
    eofController.Run();
}
