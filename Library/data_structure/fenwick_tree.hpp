#pragma once

#include <Library/core/common_setting.hpp>

template<class T = ll>
class FenwickTree {
  ll n_;
  vector<T> bit_;

  static ll lowbit(ll i){ return i & -i; }

  // skip は，候補の累積和が探索の答えより手前にあるかを判定する．
  template<class Skip>
  ll bound(Skip skip) const {
    ll p=0;
    T prefix{};
    for(ll step=std::bit_floor(ull(n_));step>0;step>>=1){
      if(step>n_-p) continue;
      T next=prefix+bit_[p+step];
      if(skip(next)){
        p+=step;
        prefix=next;
      }
    }
    return p;
  }

public:
  explicit FenwickTree(ll n) : n_(n){
    assert(n>=0);
    bit_.resize(size_t(n)+1);
  }

  explicit FenwickTree(const vector<T>& v) : FenwickTree(ssize(v)){
    for(ll p=0;p<n_;p++) bit_[p+1]=v[p];
    for(ll p=0;p<n_;p++){
      const ll i=p+1, step=lowbit(i);
      if(step<=n_-i) bit_[i+step]+=bit_[i];
    }
  }

  ll size() const { return n_; }

  void add(ll p,const T& x){
    assert(0<=p && p<n_);
    for(ll i=p+1;;){
      bit_[i]+=x;
      const ll step=lowbit(i);
      if(step>n_-i) break;
      i+=step;
    }
  }

  void set(ll p,const T& x){ add(p,x-get(p)); }

  T get(ll p) const {
    assert(0<=p && p<n_);
    const ll i=p+1, left=i-lowbit(i);
    T x=bit_[i];
    // このノードが持つ区間から，p より左の部分を取り除く．
    for(ll j=i-1;j>left;j-=lowbit(j)) x-=bit_[j];
    return x;
  }

  T sum(ll r) const {
    assert(0<=r && r<=n_);
    T result{};
    for(;r>0;r-=lowbit(r)) result+=bit_[r];
    return result;
  }

  T sum(ll l,ll r) const {
    assert(0<=l && l<=r && r<=n_);
    return sum(r)-sum(l);
  }

  T all_sum() const { return sum(n_); }

  // 探索時は全要素が非負であること．該当なしなら size() を返す．
  ll lower_bound(const T& x) const {
    if(!(T{}<x)) return 0;
    return bound([&](const T& prefix){ return prefix<x; });
  }

  ll upper_bound(const T& x) const {
    if(x<T{}) return 0;
    return bound([&](const T& prefix){ return !(x<prefix); });
  }
};

template<class T = ll>
using fenwick_tree = FenwickTree<T>;
