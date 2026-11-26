# kyopro_template

競技プログラミング用のテンプレートおよびライブラリです．

個人で使うためのものであり，AtCoderのAI生成対策ルールに対応するために仕方なく公開しています．

他の方が自由に使用，改変していただいて構いませんが，正しく動くことを保証するものではありません．

## Directory Structure

| ディレクトリ | 内容 |
|---|---|
| `Library/core/` | 共通設定・型エイリアス・マクロ |
| `Library/io/` | 入出力・デバッグ |
| `Library/utility/` | 汎用補助関数 |
| `Library/sequence/` | 数列・配列操作 |
| `Library/math/` | 数学・整数演算 |
| `Library/grid/` | グリッド操作 |
| `Library/geometry/` | 幾何 |
| `Library/graph/` | グラフ・木 |
| `Library/data_structure/` | データ構造 |
| `Library/algorithm/` | 汎用アルゴリズム |
| `Library/string/` | 文字列処理 |
| `Library/other/` | その他 |
| `snippets/` | VSCode用スニペット |

`Library/template.hpp` には，普段使用するライブラリをまとめています．

なお，未整備のライブラリや旧実装も含まれています．

## Usage

C++23（GNU++23）での使用を想定しています．

ライブラリをまとめて読み込む場合は，以下のように記述します．

```cpp
#include <Library/template.hpp>
```

必要なライブラリだけを個別に読み込むこともできます．

```cpp
#include <Library/data_structure/indexed_set.hpp>
```

コンパイル時には，リポジトリのルートディレクトリをインクルードパスに指定してください．

```bash
g++ -std=gnu++23 -O2 -I. main.cpp
```

## expand.py

自作ライブラリのincludeを展開し，単一のC++ソースコードを生成するPythonスクリプトです．

リポジトリのルートディレクトリで以下のように実行します．

```bash
python3 expand.py main.cpp > expanded.cpp
```

### 仕様

- `#include <Library/...>` およびダブルクォートによるincludeを再帰的に展開する．
- 同一ヘッダの重複展開を防止する．
- 展開対象のヘッダには `#pragma once` が必要．
- 循環includeを検出する．
- 条件付きincludeの展開には対応しない．
- 標準ライブラリやACL，Boostなどの山括弧によるincludeは展開しない．
- `CPLUS_INCLUDE_PATH` が設定されている場合は，そのパスを使用する．

一般的なC++プリプロセッサの全機能に対応するものではありません．

展開に成功した場合，生成されたソースコードの最終行に以下のコメントが追加されます．

```cpp
// Expanded by https://github.com/Hassy935/kyopro_template/blob/main/expand.py
```

## License

コードは，特に断りのない限り [CC0 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/) の下で公開しています．

これらのコードは自由に使用・改変・再配布できます．著作権表示や出典の記載は必要ありません．

## Disclaimer

本ライブラリは，現状のまま（AS IS）提供されます．

各実装の正確性，動作，性能，特定の目的への適合性について，いかなる保証も行いません．

バグや未検証の実装が含まれている可能性があります．使用にあたっては，各自の責任で動作を確認してください．

本ライブラリの使用によって生じた損害について，法令で認められる最大限の範囲で責任を負いません．
