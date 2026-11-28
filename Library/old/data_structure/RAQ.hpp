#pragma once

struct RAQ{
  ll n,sz;
  vl lazy;

  // 初期値は0．
  // O(N)．
  RAQ(ll n):n(n),sz(1){
    while(sz<n) sz*=2;
    lazy.assign(2*sz,0);
  }

  // 配列aで初期化する．
  // O(N)．
  RAQ(const vl& a):RAQ(ssize(a)){
    rep(i,n) lazy[sz+i]=a[i];
  }

  // [l,r)にxを加算する．
  // O(log N)．
  void add(ll l, ll r, ll x){
    assert(0<=l&&l<=r&&r<=n);

    l+=sz;r+=sz;
    while(l<r){
      if(l&1) lazy[l++]+=x;
      if(r&1) lazy[--r]+=x;
      l>>=1;r>>=1;
    }
  }

  // p番目にxを加算する．
  // O(1)．
  void add(ll p, ll x){
    assert(0<=p&&p<n);
    lazy[sz+p]+=x;
  }

  // p番目をxに変更する．
  // O(log N)．
  void set(ll p, ll x){
    assert(0<=p&&p<n);
    lazy[sz+p]+=x-get(p);
  }

  // p番目の値を取得する．
  // O(log N)．
  ll get(ll p) const{
    assert(0<=p&&p<n);

    ll res=0;
    p+=sz;
    while(p>0){
      res+=lazy[p];
      p>>=1;
    }
    return res;
  }
};