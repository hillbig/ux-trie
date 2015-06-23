UX tutorial (Japanese)

## はじめに ##

uxはコンパクトなtrieを構築・操作するためのライブラリです．従来のtrieの実装に比べて1/5～1/20の作業領域料で辞書を保持することができ、数億～数十億キーワードの大規模な辞書を効率的に扱うことができます。
Trieはキー集合に対する処理を効率的に行うことができるデータ構造であり、キーが辞書中に存在しているか、キーの接頭辞に一致する全ての辞書を列挙するなどの操作を効率的に行うことができます．

## txからの相違点 ##

uxは[tx](http://code.google.com/p/tx-trie/)の後継のライブラリです．
txからの相違点は以下のとおりです。

  * 構築時にファイルを作成する必要が無い
  * 連想配列を標準でサポート
  * 構築後の作業領域量はtxの1/2～1/3
  * ソースコードの大幅なリファクタリング


## インストール方法 ##
[download\_list](http://code.google.com/p/ux-trie/downloads/list)から最新版をux-x.x.x.tar.bz2をダウンロードします．

次のようにしてインストールします．
```
> tar xvjf ux-x.x.x.tar.bz2
> cd ux-x.x.x
> ./waf configure
> ./waf
> sudo ./waf install
```

## プログラムの利用方法 ##

プログラムuxが利用可能です。uxを利用して、辞書を作成すること、また作成した辞書を利用して、探索操作を行うことが可能です。

```
% ux
usage: ux --index=string [options] ...
options:
  -w, --wordlist      word list (string [=])
  -i, --index         index (string)
  -l, --limit         limit at search (int [=10])
  -u, --uncompress    tail is uncompressed
  -e, --enumerate     enumerate all keyword
  -v, --verbose       verbose mode (int [=0])
  -h, --help          this message

```
オプション無しで起動した場合、操作方法が表示されます。

以下に代表的な例を示します。

まず一行に一キーワードファイルを用意します
```
% head keylist
吉祥寺
改築
三越
円
..
```

これに対するux辞書を作成します。-k でキーワードファイル、-i で保存するファイルを指定します。
```
% ux -k keylist -i index
```
これでkeylistに対する辞書を作成し、その結果をindexに保存しました。

```
% ux -i index
```
-iのみを指定した場合は対話的に辞書に対する検索を行ないます。


```
% ux -i index
read:477917 keys
>特異点
query:[特異点]
prefixSearch: 特異点    (id=217401)
commonPrefixSearch: 1 found.
特異点  (id=217401)
predictiveSearch: 2 found.
特異点  (id=217401)
特異点定理      (id=251731)
```

構築済みの辞書からキーワードリストを復元したい場合には-eを指定します
```
% ux -i index -e 
特異点
特異点定理
...
```

一行一キーワード形式で出力されます。なお入力時の順序は保存されません。

## C++ APIの利用方法 ##

使用したいC++プログラムからux::Trieを構築し、これを介して利用します．
ux/ux.hppをインクルードし、uxライブラリを読み込みます。

ux::Trieはtrieデータ構造であり、キー集合を管理し、それらに対する操作を実現します。
ux::Mapは連想配列であり、キーを介して値を操作できます．内部ではux::Trieを利用しています。

以下にサンプルコードを示します．

```
#include <ux/ux.hpp>
...

vector<string> keyList;
keyList.push_back(string("レッドブル")) // 辞書登録したいキーワード集合
keyList.push_back(string("ウォッカ")) // 辞書登録したいキーワード集合
...

ux::Trie trie(keyList); // keyListを元にtrieを構築 trie.build(keyList)でも構築可能

string line = "レッドブルとウォッカを両方飲むとやばい";
size_t retLen = 0;
ux::Trie::id_t id = trie.prefixSearch(line.c_str(), line.size(), retLen);
if (id == ux::Trie::NOTFOUND){
  cerr << "not found any keyword" << endl; // 見つからない場合はNOTFOUNDが返る
} else {
  string key = trie.decodeKeyword(id); // idを利用してキーワードは復元できる
  assert(key == "レッドブル");
}


if (trie.save("index") != ux::UX::SUCCESS){ ; // indexに保存
  cerr << "save error" << endl; 
  return -1 
}

ux::Trie anotherTrie;
if (anotherTrie.load("index") != ux::UX::SUCCESS){
  cerr << "load error" << endl;
  return -1;
}

```

## C++ API ##

具体的なAPIについては以下のヘッダファイルを参照してください。
  * [uxTrie.hpp](http://code.google.com/p/ux-trie/source/browse/trunk/src/uxTrie.hpp)
  * [uxMap.hpp](http://code.google.com/p/ux-trie/source/browse/trunk/src/uxMap.hpp)

## 謝辞 ##

tkngさんにpkgconfig対応をしていただきました。ありがとうございます。
nobu-kさんにバグ修正のパッチをいただきました。ありがとうございます。