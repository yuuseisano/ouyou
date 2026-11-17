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
    // 入力終了と終了コマンドは、呼び出し元へ false で伝える。
    const int choice = mView.ReadAction(actor);
    if (choice == 0 || choice == 6) return false;
    if (choice == 1 || choice == 2)
    {
        int spell = 0;
        if (choice == 2)
        {
            spell = mView.ReadSpell();
            if (!spell) return false;
        }
        const auto enemies = mModel.GetEnemies();
        if (enemies.empty()) { mView.Message("対象の敵がいません。"); return true; }
        const int targetIndex = mView.ReadTarget(enemies);
        if (!targetIndex) return false;
        // 表示番号は１始まりなので、一覧の添字へ変換する。
        auto& target = *enemies[targetIndex - 1];
        if (choice == 1) Attack(actor, target);
        else if (actor.CastSpell(spell - 1, target))
        {
            const auto& selected = GetSpellBook()[spell - 1];
            mView.ShowAttack(actor, target, selected.power, selected.name);
        }
        else mView.Message("魔法を使えません。MPを確認してください。");
    }
    else if (choice == 3)
    {
        if (actor.GetInventory().empty()) mView.Message("アイテムがありません。");
        else
        {
            const int index = mView.ReadItem(actor);
            if (!index) return false;
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
        // 死亡した敵を削除する前にドロップを抽選し、主人公へ渡す。
        if (player)
            for (const auto& actor : mModel.GetActors())
                if (actor->IsDead() && actor->GetType() != "Player")
                {
                    auto drop = Item::CreateRandomDrop();
                    if (drop)
                    {
                        player->AddItem(drop);
                        mView.Message(actor->GetName() + " が " + drop->GetName() + " を落とした！");
                    }
                }
        // 死亡者を取り除き、同じ敵のドロップ抽選を繰り返さない。
        mModel.RemoveDeadActors();
        if (quit) { mView.Message("ゲームを終了します。"); break; }
        if (!player || player->IsDead()) { mView.Message("あなたは敗北しました..."); break; }
        if (mModel.GetEnemies().empty())
        {
            // 続行時は主人公の状態を引き継ぎ、新しい敵を１体追加する。
            if (!mView.ReadNextWave()) break;
            mView.ShowActors(mModel.SpawnEnemies(1));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    mView.Message("=== ゲーム終了 ===");
}
