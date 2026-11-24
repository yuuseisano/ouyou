#include "GameController.h"
#include "Item.h"
#include <chrono>
#include <thread>


void GameController::Attack(Actor& attacker, Actor& target)
{
    const int damage = attacker.Attack(target);
    mView.ShowAttack(attacker, target, damage);
}


bool GameController::HandlePlayer(Actor& actor)
{
    // キャンセル時は行動を確定せず、同じターンのコマンド選択へ戻る。
    for (;;)
    {
        // 入力終了と終了コマンドは、呼び出し元へ false で伝える。
        const int choice = mView.ReadAction(actor);
        if (choice == GameView::Cancelled) continue;
        if (choice == GameView::InputEnded || choice == 6) return false;
        if (choice == 1 || choice == 2)
        {
            int spell = 0;
            if (choice == 2)
            {
                spell = mView.ReadSpell(actor);
                if (spell == GameView::Cancelled) continue;
                if (spell == GameView::InputEnded) return false;
            }
            const auto enemies = mModel.GetEnemies();
            if (enemies.empty()) { mView.Message("対象の敵がいません。"); return true; }
            const int targetIndex = mView.ReadTarget(enemies);
            if (targetIndex == GameView::Cancelled) continue;
            if (targetIndex == GameView::InputEnded) return false;
            // 表示番号は１始まりなので、一覧の添字へ変換する。
            auto& target = *enemies[targetIndex - 1];
            if (choice == 1) Attack(actor, target);
            else if (actor.CastSpell(spell - 1, target))
            {
                const auto& selected = GetSpellBook()[spell - 1];
                mView.ShowAttack(actor, target, selected.GetDamage(target.GetType()), selected.name);
            }
            else mView.Message("魔法を使えません。MPを確認してください。");
        }
        else if (choice == 3)
        {
            if (actor.GetInventory().empty()) mView.Message("アイテムがありません。");
            else
            {
                const int index = mView.ReadItem(actor);
                if (index == GameView::Cancelled) continue;
                if (index == GameView::InputEnded) return false;
                // 回復上限を反映するため、使用前後の差を表示する。
                const int hp = actor.GetHp(), mp = actor.GetMp();
                if (actor.UseItem(index - 1)) mView.ShowRecovery(actor, actor.GetHp() - hp, actor.GetMp() - mp);
                else mView.Message("アイテムの使用に失敗しました。");
            }
        }
        else if (choice == 4) mView.ShowActors(mModel.GetActors());
        else mView.Message(actor.GetName() + " は様子を見ている。");
        // ステータス確認、待機、操作の失敗でもターンを消費する。
        return true;
    }
}

void GameController::Run()
{
    mModel.Initialize();
    mView.Message("=== 出現した敵 ===");
    mView.ShowActors(mModel.GetEnemies());
    mView.Message("=== コンソールRPG 開始 ===");
    for (int turn = 1;; ++turn)
    {
        mView.ShowTurn(turn);
        bool quit = false;
        // ターン開始時の行動順を使い、途中で倒されたアクターは飛ばす。
        for (const auto& actor : mModel.GetTurnOrder())
        {
            if (actor->IsDead()) continue;
            if (actor->GetType() == "Player")
            {
                if (!HandlePlayer(*actor)) { quit = true; break; }
            }
            else
            {
                auto player = mModel.GetPlayer();
                if (player && !player->IsDead()) Attack(*actor, *player);
            }
            actor->Update();
        }
        auto player = mModel.GetPlayer();
        // 削除前に討伐経験値とドロップを処理する。死亡した主人公には経験値を与えない。
        if (player)
            for (const auto& actor : mModel.GetActors())
                if (actor->IsDead() && actor->GetType() != "Player")
                {
                    const int previousLevel = player->GetHeroStats()->level;
                    const int experience = player->GainExperienceFrom(*actor);
                    if (experience > 0)
                        mView.Message(player->GetName() + " は " + std::to_string(experience) + " EXP を獲得した！");
                    if (player->GetHeroStats()->level > previousLevel)
                        mView.Message("レベルアップ！ Lv." + std::to_string(player->GetHeroStats()->level)
                            + " になった！ HP・MPが全回復した！");
                    // 一度に複数レベル上がっても、習得レベルを通過した魔法を通知する。
                    for (const auto& spell : GetSpellBook())
                        if (previousLevel < spell.requiredLevel && player->GetHeroStats()->level >= spell.requiredLevel)
                            mView.Message(spell.name + " を覚えた！");
                    auto drop = Item::CreateRandomDrop();
                    if (drop)
                    {
                        player->AddItem(drop);
                        mView.Message(actor->GetName() + " が " + drop->GetName() + " を落とした！");
                    }
                }
        // 死亡者を取り除き、同じ敵のドロップ抽選を繰り返さない。
        mModel.RemoveDeadActors();
        mModel.UpdateEnemyDifficulty();
        if (quit) { mView.Message("ゲームを終了します。"); break; }
        if (!player || player->IsDead()) { mView.Message("あなたは敗北しました..."); break; }
        if (mModel.GetEnemies().empty())
        {
            // 続行時は主人公の状態を引き継ぎ、新しい敵を２体追加する。
            if (!mView.ReadNextWave()) break;
            mView.ShowActors(mModel.SpawnEnemies(2));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    mView.Message("=== ゲーム終了 ===");
}
