#include "Game.h"
#include "DataTable.h"
#include "Item.h"
#include "Spell.h"
#include <chrono>
#include <thread>
#include <algorithm>
#include <string>
#include <sstream>
#include <limits>
#include <random>
#include <vector>
#include <iostream>

namespace
{
    // 入力を安全に整数へ変換し、範囲チェックするヘルパー
    int ReadIntInRange(const std::string& prompt, int minVal, int maxVal)
    {
        while (true)
        {
            std::cout << prompt;
            std::string line;
            if (!std::getline(std::cin, line))
            {
                // 入力ストリームが閉じられた場合は最小値を返す
                return minVal;
            }

            std::istringstream iss(line);
            int v;
            if ((iss >> v) && v >= minVal && v <= maxVal)
            {
                return v;
            }
            std::cout << "無効な入力です。 " << minVal << "?" << maxVal << " の数字を入力してください。\n";
        }
    }

    // ランダム生成用ユーティリティ（重み付き）
    std::mt19937& GetRng()
    {
        static std::mt19937 rng(static_cast<unsigned int>(std::random_device{}()));
        return rng;
    }

    // ランダムで敵を生成して返す (重み付き: Slime:50, Goblin:30, Skeleton:15, Dragon:5)
    std::vector<std::shared_ptr<Actor>> SpawnRandomEnemies(int count)
    {
        const std::vector<std::string> types = { "Slime", "Goblin", "Skeleton", "Dragon" };
        const std::vector<int> weights = { 50, 30, 15, 5 };
        std::discrete_distribution<int> dist(weights.begin(), weights.end());

        std::vector<std::shared_ptr<Actor>> result;
        result.reserve(count);
        for (int i = 0; i < count; ++i)
        {
            int idx = dist(GetRng());
            result.push_back(ActorFactory::Create(types[idx]));
        }
        return result;
    }
}

Game& Game::Instance()
{
    static Game instance;
    return instance;
}

Game::Game()
{
    // 初期データテーブルは Factory が使うためここでは何もしない
}

void Game::AddActor(std::shared_ptr<Actor> actor)
{
    mActors.push_back(actor);
}

void Game::RemoveDeadActors()
{
    mActors.erase(
        std::remove_if(mActors.begin(), mActors.end(),
                       [](const std::shared_ptr<Actor>& a) { return a->IsDead(); }),
        mActors.end());
}

void Game::Run()
{
    // プレイヤー生成
    auto player = ActorFactory::Create("Player");
    AddActor(player);

    // 敵をランダムに2体スポーン
    auto initialEnemies = SpawnRandomEnemies(2);
    for (auto& e : initialEnemies)
    {
        AddActor(e);
    }

    // 生成された敵を一覧表示（名前・タイプ・HP・速さ）
    std::cout << "=== 出現した敵 ===\n";
    for (auto& e : initialEnemies)
    {
        std::cout << e->GetName() << " [" << e->GetType() << "] HP:" << e->GetHp()
                  << " SPD:" << e->GetSpeed() << "\n";
    }

    std::cout << "=== コンソールRPG 開始 ===\n";
    std::cout << "プレイヤー操作：攻撃・魔法・アイテム・ステータス確認・待機・終了が選べます。\n";

    int turn = 1;
    bool quitRequested = false;
    while (true)
    {
        std::cout << "\n--- ターン " << turn << " ---\n";

        // ラウンド開始時の行動対象スナップショットを作成（生存者のみ）
        std::vector<std::shared_ptr<Actor>> turnOrder;
        for (auto& a : mActors)
        {
            if (!a->IsDead()) turnOrder.push_back(a);
        }

        // 速さが高い順にソート（同速は名前で安定ソート）
        std::sort(turnOrder.begin(), turnOrder.end(),
                  [](const std::shared_ptr<Actor>& a, const std::shared_ptr<Actor>& b) {
                      if (a->GetSpeed() != b->GetSpeed()) return a->GetSpeed() > b->GetSpeed();
                      return a->GetName() < b->GetName();
                  });

        for (auto& actor : turnOrder)
        {
            // 既に倒れている場合やゲームから削除されている場合はスキップ
            if (actor->IsDead()) continue;

            if (actor->GetType() == "Player")
            {
                // プレイヤー入力メニュー（MP表示追加）
                std::cout << "\nあなたの番: " << actor->GetName() << " (HP: " << actor->GetHp()
                          << " MP: " << actor->GetMp() << " SPD:" << actor->GetSpeed() << ")\n";
                std::cout << "1) 攻撃  2) 魔法  3) アイテム  4) ステータス  5) 待機  6) 終了\n";
                int choice = ReadIntInRange("選択> ", 1, 6);

                if (choice == 1)
                {
                    // 攻撃対象を列挙
                    std::vector<std::shared_ptr<Actor>> enemies;
                    for (auto& a : mActors)
                    {
                        if (a->GetType() != "Player" && !a->IsDead())
                            enemies.push_back(a);
                    }

                    if (enemies.empty())
                    {
                        std::cout << "攻撃対象がいません。\n";
                    }
                    else
                    {
                        std::cout << "攻撃対象：\n";
                        for (size_t idx = 0; idx < enemies.size(); ++idx)
                        {
                            std::cout << (idx + 1) << ") " << enemies[idx]->GetName() << " (HP: " << enemies[idx]->GetHp()
                                      << " SPD:" << enemies[idx]->GetSpeed() << ")\n";
                        }
                        int t = ReadIntInRange("対象番号> ", 1, static_cast<int>(enemies.size()));
                        actor->Attack(*enemies[t - 1]);
                    }
                }
                else if (choice == 2)
                {
                    // 魔法（スペル選択→対象選択）
                    const auto& spells = GetSpellBook();
                    std::cout << "使用可能な魔法：\n";
                    for (size_t s = 0; s < spells.size(); ++s)
                    {
                        std::cout << (s + 1) << ") " << spells[s].name
                                  << " (MP:" << spells[s].cost << " 威力:" << spells[s].power << ")\n";
                    }
                    int sidx = ReadIntInRange("使用する魔法番号> ", 1, static_cast<int>(spells.size()));
                    // 攻撃対象を列挙
                    std::vector<std::shared_ptr<Actor>> enemies;
                    for (auto& a : mActors)
                    {
                        if (a->GetType() != "Player" && !a->IsDead())
                            enemies.push_back(a);
                    }
                    if (enemies.empty())
                    {
                        std::cout << "敵がいない。\n";
                    }
                    else
                    {
                        std::cout << "魔法の対象：\n";
                        for (size_t idx = 0; idx < enemies.size(); ++idx)
                        {
                            std::cout << (idx + 1) << ") " << enemies[idx]->GetName() << " (HP: " << enemies[idx]->GetHp() << ")\n";
                        }
                        int t = ReadIntInRange("対象番号> ", 1, static_cast<int>(enemies.size()));
                        actor->CastSpell(static_cast<size_t>(sidx - 1), *enemies[t - 1]);
                    }
                }
                else if (choice == 3)
                {
                    // アイテム使用
                    const auto& inv = actor->GetInventory();
                    if (inv.empty())
                    {
                        std::cout << "インベントリにアイテムがありません。\n";
                    }
                    else
                    {
                        std::cout << "アイテム一覧：\n";
                        for (size_t i = 0; i < inv.size(); ++i)
                        {
                            std::cout << (i + 1) << ") " << inv[i]->GetName() << " (回復量:" << inv[i]->GetValue() << ")\n";
                        }
                        int idx = ReadIntInRange("使用するアイテム番号> ", 1, static_cast<int>(inv.size()));
                        if (actor->UseItem(static_cast<size_t>(idx - 1)))
                        {
                            std::cout << "アイテムを使用した。\n";
                        }
                        else
                        {
                            std::cout << "アイテムの使用に失敗した。\n";
                        }
                    }
                }
                else if (choice == 4)
                {
                    std::cout << "=== ステータス ===\n";
                    for (auto& a : mActors)
                    {
                        std::cout << a->GetName() << " [" << a->GetType() << "] HP:" << a->GetHp()
                                  << " MP:" << a->GetMp()
                                  << " ATK:" << a->GetAttack() << " DEF:" << a->GetDefense()
                                  << " SPD:" << a->GetSpeed()
                                  << (a->IsDead() ? " (倒)" : "") << "\n";
                    }
                }
                else if (choice == 5)
                {
                    std::cout << actor->GetName() << " は様子を見ている。\n";
                }
                else if (choice == 6)
                {
                    std::cout << "プレイヤーがゲーム終了を要求しました。\n";
                    quitRequested = true;
                    break;
                }
            }
            else
            {
                // 敵のAI：最も近いプレイヤーを攻撃
                std::shared_ptr<Actor> playerTarget = nullptr;
                for (auto& a : mActors)
                {
                    if (a->GetType() == "Player" && !a->IsDead()) { playerTarget = a; break; }
                }
                if (playerTarget)
                {
                    std::cout << actor->GetName() << " (SPD:" << actor->GetSpeed() << ") の行動：\n";
                    actor->Attack(*playerTarget);
                }
            }

            // 各ターンごとに状態更新
            actor->Update();

            if (quitRequested) break;
        }

        // 倒された敵へアイテムドロップ処理（まだドロップしていない敵のみ）
        for (auto& a : mActors)
        {
            if (a->IsDead() && a->GetType() != "Player")
            {
                // mHasDropped は Actor 内部メンバ（フレンド等でアクセス可能）
                // ここは Game から直接アクセスせず、安易に dynamic_cast せずに
                // Actor にフラグが露出しているためアクセス可能と仮定して処理。
                // （本来はイベント/コールバックで処理するのが望ましい）
                // キャストによる安全なアクセス
                // 代替：一時的に public なメソッドがないため、ここはフレンドや public accessor が必要。
            }
        }

        // 実装上簡潔にするため、Actor の mHasDropped は非公開だが Game.cpp 内で直接扱えるように、
        // mHasDropped を使う代わりに死んでいる敵に対して一度だけドロップ処理を行い、
        // その後 RemoveDeadActors() で削除する流れにする（つまり本ループで必ずドロップが発生する）。
        // 実運用では重複処理を避けるフラグを Actor に追加しておくのが安全。
        // ここではシンプルに「倒れている敵には必ず1回ドロップ」を行う。

        // プレイヤー参照取得
        std::shared_ptr<Actor> playerRef = nullptr;
        for (auto& a : mActors)
        {
            if (a->GetType() == "Player") { playerRef = a; break; }
        }

        if (playerRef)
        {
            for (auto& a : mActors)
            {
                if (a->IsDead() && a->GetType() != "Player")
                {
                    auto drop = Item::CreateRandomDrop();
                    if (drop)
                    {
                        std::cout << a->GetName() << " は " << drop->GetName() << " を落とした！\n";
                        playerRef->AddItem(drop);
                    }
                }
            }
        }

        RemoveDeadActors();

        // 終了判定
        bool playerAlive = false;
        bool anyEnemyAlive = false;
        for (auto& a : mActors)
        {
            if (a->GetType() == "Player" && !a->IsDead()) playerAlive = true;
            if (a->GetType() != "Player" && !a->IsDead()) anyEnemyAlive = true;
        }

        if (quitRequested)
        {
            std::cout << "\nプレイヤーによりゲームを終了します。\n";
            break;
        }

        if (!playerAlive)
        {
            std::cout << "\nあなたは敗北しました...\n";
            break;
        }

        if (!anyEnemyAlive)
        {
            // 敵が全滅 → 次の敵を出すかどうか確認（スペースで次の敵が出る）
            std::cout << "\n敵を全滅させました！ スペースキーを押すと次の敵が出現します。その他キーで終了。\n";
            std::string line;
            if (!std::getline(std::cin, line))
            {
                std::cout << "入力が読み取れません。ゲーム終了。\n";
                break;
            }
            if (!line.empty() && line[0] == ' ')
            {
                auto newEnemies = SpawnRandomEnemies(1);
                for (auto& e : newEnemies)
                {
                    AddActor(e);
                    std::cout << e->GetName() << " が現れた！ (HP:" << e->GetHp() << " SPD:" << e->GetSpeed() << ")\n";
                }
            }
            else
            {
                std::cout << "\nゲームを終了します。\n";
                break;
            }
        }

        ++turn;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    std::cout << "=== ゲーム終了 ===\n";
}
