# ouyou

コンソール RPG を MVC に分割しています。

| 役割 | ファイル | 責務 |
| --- | --- | --- |
| Model | `ouyou/GameModel.h/.cpp` | アクターの保持、敵生成、生存判定用の検索、行動順、死亡アクターの削除 |
| Model | `ouyou/Actor.h/.cpp`, `Item.h`, `Spell.h`, `Stats.h`, `DataTable.h`, `HeroDataTable.h`, `EnemyDataTable.h`, `Factory.h`, `State.h` | ステータス、戦闘、魔法、回復、データと状態遷移 |
| View | `ouyou/GameView.h/.cpp` | コンソール表示、入力と入力形式の検証 |
| Controller | `ouyou/GameController.h/.cpp` | 操作の振り分け、敵の行動、ターン進行、終了と次の戦闘の制御 |
| 起動 | `ouyou/ouyou.cpp`, `ouyou/Game.h/.cpp` | Model・View・Controller を組み立てて実行 |

能力値は主人公用の `HeroDataTable.h` と敵用の `EnemyDataTable.h` で個別に管理します。主人公は `HeroStats.h` の `HeroStats`（初期レベル１）、敵は `EnemyStats.h` の `EnemyStats`（報酬経験値）を保持します。`DataTable::GetHero()` と `GetEnemy()` はそれぞれ専用の型を返します。敵を倒すと経験値を獲得します（スライム5、ゴブリン10、スケルトン8、ドラゴン50 EXP）。次のレベルに必要な経験値は現在レベル×20で、余剰分は持ち越します。レベルアップ時は最大HP＋10、最大MP＋5、攻撃力＋3、防御力＋2、素早さ＋1となり、HP・MPが全回復します。上限はレベル99です。

`ObjectPool.h` は現在ゲームから使われていない汎用ユーティリティです。
`cpp ouyou/Game.cpp` はプロジェクトに登録されていない旧版で、実行対象は `ouyou` フォルダーです。

Visual Studio 2022 で `ouyou.sln` を開いてビルドしてください。
初戦・再戦ともに敵は２体出現します。開始時の所持品はHPポーション（HP30回復）とMPポーション（MP20回復）が各１個です。再戦では所持品を引き継ぎ、初期アイテムは再配布しません。
ゲームの入力が終了した場合も終了します。ステータス確認・待機・失敗した操作は従来どおりターンを消費します。

`tests/MvcTests.cpp` は、モデルの戦闘・回復・行動順と、入力検証・終了操作を確認するテストです。
開発者コマンドプロンプトで以下を実行できます。

```bat
cl /nologo /EHsc /std:c++14 /I ouyou tests\MvcTests.cpp ouyou\Actor.cpp ouyou\GameModel.cpp ouyou\GameView.cpp ouyou\GameController.cpp /Fe:MvcTests.exe
MvcTests.exe
```

アイテムは種類と回復量が同じものをまとめ、一覧に所持数を表示します。入手時に１個増え、使用時に１個減ります。０個になると一覧から削除します。

主人公がLv5・10・15…に達するごとに、敵の最大HPは基礎値の10％（最低１）ずつ、攻撃力・防御力・素早さは各１ずつ上昇します。生存中の敵にもターン終了時に反映し、受けたダメージは維持します。報酬経験値は変えません。再戦の敵も同じ強化段階で出現します。
Lv10になると「ドラゴンブレイカー」（消費MP30、威力120、ドラゴンへの威力240）を習得し、習得メッセージと魔法一覧に表示します。

攻撃対象・魔法・魔法の対象・アイテムの選択中はEscキーで行動コマンドへ戻れます。WindowsコンソールではEnter不要です。キャンセルではターン・MP・アイテムを消費しません。行動コマンドでEscを押した場合は、そのまま選択を続けます。
