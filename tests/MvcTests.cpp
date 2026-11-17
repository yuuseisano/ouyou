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
    assert(slime.GetEnemyStats()->exp == 5);
    assert(DataTable::GetHero("Player").level == 1);
    assert(DataTable::GetEnemy("Unknown").exp == 1);

    // しきい値直前では成長せず、到達時に能力値上昇と全回復を行う。
    hero.TakeDamage(40);
    assert(hero.CastSpell(0, slime));
    assert(hero.AddExperience(19) == 0);
    assert(hero.GetHeroStats()->experience == 19 && hero.GetHp() == 60 && hero.GetMp() == 25);
    assert(hero.GetExperienceToNextLevel() == 1);
    assert(hero.AddExperience(1) == 1);
    assert(hero.GetHeroStats()->level == 2 && hero.GetHeroStats()->experience == 0);
    assert(hero.GetMaxHp() == 110 && hero.GetHp() == 110);
    assert(hero.GetMaxMp() == 35 && hero.GetMp() == 35);
    assert(hero.GetAttack() == 23 && hero.GetDefense() == 7 && hero.GetSpeed() == 11);
    assert(hero.GetExperienceToNextLevel() == 40);
    assert(hero.AddExperience(105) == 2);
    assert(hero.GetHeroStats()->level == 4 && hero.GetHeroStats()->experience == 5);
    assert(hero.AddExperience(0) == 0 && hero.AddExperience(-10) == 0);
    assert(hero.GetHeroStats()->experience == 5);
    assert(slime.AddExperience(100) == 0);
    // 報酬は討伐後に一度だけ与え、通常攻撃と魔法の両方に対応する。
    Actor hunter("Player", "hunter");
    Actor victim("Slime", "victim");
    assert(hunter.GainExperienceFrom(victim) == 0);
    hunter.Attack(victim);
    hunter.Attack(victim);
    assert(hunter.GainExperienceFrom(victim) == 5);
    assert(hunter.GainExperienceFrom(victim) == 0);
    Actor spellVictim("Slime", "spellVictim");
    assert(hunter.CastSpell(2, spellVictim));
    assert(hunter.GainExperienceFrom(spellVictim) == 5);
    assert(hunter.GetHeroStats()->level == 1 && hunter.GetHeroStats()->experience == 10);
    // 死亡した主人公が経験値で復活せず、他の主人公も報酬の対象にならない。
    Actor fallen("Player", "fallen");
    fallen.TakeDamage(fallen.GetHp());
    assert(fallen.AddExperience(100) == 0 && fallen.IsDead());
    Actor unclaimed("Slime", "unclaimed");
    unclaimed.TakeDamage(unclaimed.GetHp());
    assert(fallen.GainExperienceFrom(unclaimed) == 0);
    assert(hunter.GainExperienceFrom(unclaimed) == 5);
    assert(hunter.GainExperienceFrom(fallen) == 0);
    // 大量獲得でもレベル上限を守り、以降は経験値や能力値を変更しない。
    assert(hero.AddExperience(2147483647) == 95);
    assert(hero.GetHeroStats()->level == 99 && hero.GetHeroStats()->experience == 0);
    assert(hero.GetExperienceToNextLevel() == 0);
    const int maximumHp = hero.GetMaxHp();
    assert(hero.AddExperience(10) == 0 && hero.GetMaxHp() == maximumHp);

    // 強化はLv5・10の境界で変わり、繰り返しても重複せず、被ダメージを維持する。
    Actor scaledSlime("Slime", "scaled");
    scaledSlime.ScaleEnemyStats(4);
    assert(scaledSlime.GetHp() == 30 && scaledSlime.GetAttack() == 5);
    scaledSlime.TakeDamage(10);
    scaledSlime.ScaleEnemyStats(5);
    assert(scaledSlime.GetMaxHp() == 33 && scaledSlime.GetHp() == 23);
    assert(scaledSlime.GetAttack() == 6 && scaledSlime.GetDefense() == 2 && scaledSlime.GetSpeed() == 9);
    scaledSlime.ScaleEnemyStats(9);
    assert(scaledSlime.GetHp() == 23 && scaledSlime.GetAttack() == 6);
    scaledSlime.ScaleEnemyStats(10);
    assert(scaledSlime.GetMaxHp() == 36 && scaledSlime.GetHp() == 26 && scaledSlime.GetAttack() == 7);
    assert(scaledSlime.GetEnemyStats()->exp == 5);
    scaledSlime.TakeDamage(scaledSlime.GetHp());
    scaledSlime.ScaleEnemyStats(15);
    assert(scaledSlime.IsDead());

    // Lv9では新魔法が一覧に出ず、直接指定してもＨＰ・ＭＰを変更しない。
    Actor mage("Player", "mage");
    Actor dragon("Dragon", "dragon");
    const size_t dragonSpell = GetSpellBook().size() - 1;
    mage.AddExperience(720);
    assert(mage.GetHeroStats()->level == 9 && !mage.KnowsSpell(dragonSpell));
    const int lockedMp = mage.GetMp();
    assert(!mage.CastSpell(dragonSpell, dragon) && mage.GetMp() == lockedMp && dragon.GetHp() == 200);
    std::istringstream lockedInput("5\n4\n");
    std::ostringstream lockedOutput;
    GameView lockedView(lockedInput, lockedOutput);
    assert(lockedView.ReadSpell(mage) == 4);
    assert(lockedOutput.str().find("ドラゴンブレイカー") == std::string::npos);
    mage.AddExperience(180);
    assert(mage.GetHeroStats()->level == 10 && mage.KnowsSpell(dragonSpell));
    assert(!slime.KnowsSpell(dragonSpell));
    std::istringstream learnedInput("5\n");
    std::ostringstream learnedOutput;
    GameView learnedView(learnedInput, learnedOutput);
    assert(learnedView.ReadSpell(mage) == static_cast<int>(dragonSpell) + 1);
    assert(learnedOutput.str().find("ドラゴンブレイカー") != std::string::npos);
    // Lv10で強化されたドラゴンにも特効が効き、通常の敵には基本威力となる。
    dragon.ScaleEnemyStats(10);
    assert(dragon.GetHp() == 240);
    assert(GetSpellBook()[dragonSpell].GetDamage("Dragon") == 240);
    assert(GetSpellBook()[dragonSpell].GetDamage("Goblin") == 120);
    assert(mage.CastSpell(dragonSpell, dragon) && dragon.IsDead() && mage.GetMp() == 45);
    Actor otherTarget("Dragon", "other");
    assert(mage.CastSpell(dragonSpell, otherTarget) && mage.GetMp() == 15);
    Actor survivingTarget("Dragon", "survivor");
    assert(!mage.CastSpell(dragonSpell, survivingTarget));
    assert(survivingTarget.GetHp() == 200 && mage.GetMp() == 15);
    Actor fastLearner("Player", "fastLearner");
    fastLearner.AddExperience(1100);
    assert(fastLearner.GetHeroStats()->level == 11 && fastLearner.KnowsSpell(dragonSpell));

    // 生存中の敵も更新し、再戦で生成する敵には現在の段階を最初から適用する。
    GameModel scalingModel;
    scalingModel.Initialize();
    scalingModel.GetPlayer()->AddExperience(200);
    scalingModel.UpdateEnemyDifficulty();
    for (const auto& target : scalingModel.GetEnemies())
        assert(target->GetAttack() == EnemyDataTable::Get(target->GetType()).attack + 1);
    scalingModel.GetPlayer()->AddExperience(700);
    const auto scaledWave = scalingModel.SpawnEnemies(2);
    for (const auto& target : scaledWave)
    {
        const auto& base = EnemyDataTable::Get(target->GetType());
        assert(target->GetAttack() == base.attack + 2);
        assert(target->GetMaxHp() == base.maxHp + (base.maxHp / 10) * 2);
        assert(target->GetHp() == target->GetMaxHp());
    }

    // 初期配置が主人公１人と敵２体になることを確認する。
    GameModel model;
    model.Initialize();
    assert(model.GetActors().size() == 3);
    auto player = model.GetPlayer();
    assert(player->GetInventory().size() == 2);
    assert(player->GetInventory()[0].item->GetType() == ItemType::HP_Potion);
    assert(player->GetInventory()[0].item->GetValue() == 30);
    assert(player->GetInventory()[1].item->GetType() == ItemType::MP_Potion);
    assert(player->GetInventory()[1].item->GetValue() == 20);
    assert(player->GetInventory()[0].count == 1 && player->GetInventory()[1].count == 1);
    // ドロップと初期アイテムで名前が異なっても、同じ種類・回復量なら個数をまとめる。
    player->AddItem(std::make_shared<Item>(ItemType::HP_Potion, "Potion", 30));
    assert(player->GetInventory().size() == 2 && player->GetInventory()[0].count == 2);
    player->AddItem(nullptr);
    assert(!player->UseItem(99) && player->GetInventory()[0].count == 2);
    std::istringstream itemInput("1\n");
    std::ostringstream itemOutput;
    GameView itemView(itemInput, itemOutput);
    assert(itemView.ReadItem(*player) == 1);
    assert(itemOutput.str().find("所持数:2") != std::string::npos);
    auto enemy = std::make_shared<Actor>("Slime", "test");
    // 通常攻撃の防御力計算、魔法の威力とＭＰ消費、不正な魔法番号を確認する。
    player->Attack(*enemy);
    assert(enemy->GetHp() == 11);
    assert(player->CastSpell(1, *enemy));
    assert(enemy->IsDead() && player->GetMp() == 20);
    assert(!player->CastSpell(99, *enemy));
    // 回復効果と、使用したアイテムが所持品から削除されることを確認する。
    player->TakeDamage(40);
    assert(player->UseItem(0) && player->GetHp() == 90);
    assert(player->GetInventory().size() == 2 && player->GetInventory()[0].count == 1);
    assert(player->UseItem(0) && player->GetHp() == 100);
    assert(player->GetInventory().size() == 1);
    assert(player->UseItem(0) && player->GetMp() == 30);
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

    assert(model.GetPlayer()->GetInventory().size() == 2);
    // 再戦では主人公と所持品を引き継ぎ、敵だけを２体生成する。
    auto continuingPlayer = model.GetPlayer();
    assert(continuingPlayer->UseItem(0));
    for (const auto& target : model.GetEnemies()) target->TakeDamage(target->GetHp());
    model.RemoveDeadActors();
    const auto nextEnemies = model.SpawnEnemies(2);
    assert(nextEnemies.size() == 2 && model.GetEnemies().size() == 2);
    assert(model.GetActors().size() == 3 && model.GetPlayer() == continuingPlayer);
    assert(continuingPlayer->GetInventory().size() == 1);

    // 同じ種類でも回復量が異なるアイテムは別の枠として保持する。
    continuingPlayer->AddItem(std::make_shared<Item>(ItemType::MP_Potion, "強いMPポーション", 50));
    assert(continuingPlayer->GetInventory().size() == 2);
    assert(continuingPlayer->GetInventory()[0].count == 1);
    assert(continuingPlayer->GetInventory()[1].count == 1);

    // 文字列、末尾の余分な文字、範囲外の値を拒否し、入力終了を検出する。
    std::istringstream invalid("x\n2abc\n0\n2\n");
    std::ostringstream output;
    GameView view(invalid, output);
    assert(view.ReadChoice("", 1, 6) == 2);
    assert(view.ReadChoice("", 1, 6) == 0);
    // Escは入力終了と区別し、入力途中の文字も破棄する。
    std::istringstream escapeInput("12\x1b\n2\n");
    std::ostringstream escapeOutput;
    GameView escapeView(escapeInput, escapeOutput);
    assert(escapeView.ReadChoice("", 1, 6) == GameView::Cancelled);
    assert(escapeView.ReadChoice("", 1, 6) == 2);
    assert(escapeView.ReadChoice("", 1, 6) == GameView::InputEnded);
    std::istringstream spellEscape("\x1b\n");
    GameView spellEscapeView(spellEscape, escapeOutput);
    assert(spellEscapeView.ReadSpell(mage) == GameView::Cancelled);
    // 攻撃対象、魔法、魔法対象、アイテムから戻り、同じターンで終了できる。
    std::istringstream cancelCommands("1\n\x1b\n2\n\x1b\n2\n1\n\x1b\n3\n\x1b\n\x1b\n6\n");
    std::ostringstream cancelOutput;
    GameModel cancelModel;
    GameView cancelView(cancelCommands, cancelOutput);
    GameController cancelController(cancelModel, cancelView);
    cancelController.Run();
    assert(cancelOutput.str().find("ターン 2") == std::string::npos);
    assert(cancelModel.GetPlayer()->GetMp() == 30);
    assert(cancelModel.GetPlayer()->GetInventory().size() == 2);
    assert(cancelModel.GetPlayer()->GetInventory()[0].count == 1);
    assert(cancelModel.GetPlayer()->GetInventory()[1].count == 1);
    for (const auto& target : cancelModel.GetEnemies())
        assert(target->GetHp() == target->GetMaxHp());
    // キャンセル直後に入力が閉じられても、無限ループせず終了する。
    std::istringstream cancelEof("2\n\x1b\n");
    GameView cancelEofView(cancelEof, escapeOutput);
    GameController cancelEofController(cancelModel, cancelEofView);
    cancelEofController.Run();

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
