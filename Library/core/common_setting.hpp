#pragma once

// https://github.com/Hassy935/kyopro_template

#ifdef LOCAL
#define _GLIBCXX_DEBUG
#endif

#include <bits/stdc++.h>
#include <cassert>

using namespace std;

#define emp emplace

#define pb push_back
#define eb emplace_back

#define pf push_front
#define ef emplace_front

#define fi first
#define se second

#define rep(i, n) for (ll i = 0; i < (ll)(n); i++)
#define drep(i, n) for (ll i = (ll)(n) - 1; i >= 0; i--)
#define forn(i, s, n) for (ll i = (s); i < (ll)(n); i++)
#define forc(i, s, n) for (ll i = (s); i <= (ll)(n); i++)
#define dforc(i, s, g) for (ll i = (s); i >= (ll)(g); i--)
#define forr(c, A) for (auto&& c: A)

#define nall(v) (v).begin(), (v).end()
#define rall(v) (v).rbegin(), (v).rend()

using ll=long long;
using ld=long double;
using i128=__int128;

using uint=unsigned int;
using ull=unsigned long long;

using str=string;
using vstr=vector<string>;
using vvstr=vector<vstr>;

using vl=vector<ll>;
using vvl=vector<vl>;
using vvvl=vector<vvl>;

using vb=vector<char>;
using vvb=vector<vb>;
using vvvb=vector<vvb>;

using vld=vector<ld>;
using vvld=vector<vld>;
using vvvld=vector<vvld>;

template<typename T> using vc=vector<T>;
template<typename T> using vvc=vc<vc<T>>;
template<typename T> using vvvc=vc<vvc<T>>;

template<class T>
using uod_set = unordered_set<T>;

template<class T>
using uod_multiset = unordered_multiset<T>;

template<class K, class V>
using uod_map = unordered_map<K, V>;

template<class T>
using priority_queue_g=priority_queue<T,vc<T>,greater<T>>;