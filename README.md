# ouyou

コンソール RPG を MVC に分割しています。

| 役割 | ファイル | 責務 |
| --- | --- | --- |
| Model | `ouyou/GameModel.h/.cpp` | アクターの保持、敵生成、生存判定用の検索、行動順、死亡アクターの削除 |
| Model | `ouyou/Actor.h/.cpp`, `Item.h`, `Spell.h`, `DataTable.h`, `Factory.h`, `State.h` | ステータス、戦闘、魔法、回復、データと状態遷移 |
| View | `ouyou/GameView.h/.cpp` | コンソール表示、入力と入力形式の検証 |
| Controller | `ouyou/GameController.h/.cpp` | 操作の振り分け、敵の行動、ターン進行、終了と次の戦闘の制御 |
| 起動 | `ouyou/ouyou.cpp`, `ouyou/Game.h/.cpp` | Model・View・Controller を組み立てて実行 |

`ObjectPool.h` は現在ゲームから使われていない汎用ユーティリティです。
`cpp ouyou/Game.cpp` はプロジェクトに登録されていない旧版で、実行対象は `ouyou` フォルダーです。

Visual Studio 2022 で `ouyou.sln` を開いてビルドしてください。
ゲームの入力が終了した場合も終了します。ステータス確認・待機・失敗した操作は従来どおりターンを消費します。

`tests/MvcTests.cpp` は、モデルの戦闘・回復・行動順と、入力検証・終了操作を確認するテストです。
開発者コマンドプロンプトで以下を実行できます。

```bat
cl /nologo /EHsc /std:c++14 /I ouyou tests\MvcTests.cpp ouyou\Actor.cpp ouyou\GameModel.cpp ouyou\GameView.cpp ouyou\GameController.cpp /Fe:MvcTests.exe
MvcTests.exe
```
