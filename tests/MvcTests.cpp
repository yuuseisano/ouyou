#include "GameController.h"
#include "Item.h"
#include <cassert>
#include <sstream>

int main()
{
    // 分割後も各アクターの能力値が維持され、別のテーブルの種類は参照しない。
    const char* types[] = { "Player", "Slime", "Goblin", "Skeleton", "Dragon", "Unknown" };
    const EnemyStats expected[] = {
        {100, 30, 20, 5, 10}, {30, 0, 5, 1, 8}, {50, 0, 12, 3, 12},
        {40, 0, 8, 2, 9}, {200, 0, 35, 10, 30}, {10, 0, 1, 0, 5}
    };
    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i)
    {
        Actor actor(types[i], "test");
        assert(actor.GetHp() == expected[i].maxHp);
        assert(actor.GetMp() == expected[i].maxMp);
        assert(actor.GetAttack() == expected[i].attack);
        assert(actor.GetDefense() == expected[i].defense);
        assert(actor.GetSpeed() == expected[i].speed);
    }
    assert(HeroDataTable::Get("Slime").maxHp == 100);
    assert(EnemyDataTable::Get("Player").maxHp == 10);

    // 専用の型とデータがアクターまで維持され、他方のステータスは持たない。
    Actor hero("Player", "hero");
    Actor slime("Slime", "slime");
    assert(hero.GetHeroStats() && !hero.GetEnemyStats());
    assert(hero.GetHeroStats()->level == 1);
    assert(slime.GetEnemyStats() && !slime.GetHeroStats());
    assert(slime.GetEnemyStats()->experienceReward == 0);
    assert(DataTable::GetHero("Player").level == 1);
    assert(DataTable::GetEnemy("Unknown").experienceReward == 0);

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
    player->AddItem(std::make_shared<Item>(ItemType::HP_Potion, "Potion", 30));
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
