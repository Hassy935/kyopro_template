#pragma once
#include <Library/core/common_setting.hpp>

// 素数 mod p に対し、0 <= n < p の範囲で階乗・逆階乗を管理する。
// dynamic_modint の場合、インスタンスの使用中は mod を変更しない。
template<class Mint>
class Combination {
  ll p=Mint::mod();
  vector<Mint> fac{Mint(1)},ifac{Mint(1)};

public:
  explicit Combination(ll N=0){
    assert(p>=2);
    extend(N);
  }

  // 事前計算済みの最大の添字（要素数ではない）。
  ll limit() const { return ssize(fac)-1; }

  // 必要に応じて倍増し、p-1 を超えないようにする。
  void extend(ll N){
    assert(Mint::mod()==p && "Combination: modulus changed");
    assert(0<=N && N<p);
    ll old=limit();
    if(N<=old) return;

    ll nxt=max(N,(ll)min((i128)p-1,(i128)2*old+1));
    fac.resize(nxt+1);
    ifac.resize(nxt+1);

    for(ll i=old+1;i<=nxt;i++) fac[i]=fac[i-1]*i;
    ifac[nxt]=fac[nxt].inv();
    for(ll i=nxt;i>old+1;i--) ifac[i-1]=ifac[i]*i;
  }

  Mint fact(ll n){
    extend(n);
    return fac[n];
  }

  Mint inv_fact(ll n){
    extend(n);
    return ifac[n];
  }

  Mint C(ll n,ll k){
    if(n<0 || k<0 || n<k) return 0;
    extend(n);
    return fac[n]*ifac[k]*ifac[n-k];
  }

  Mint P(ll n,ll k){
    if(n<0 || k<0 || n<k) return 0;
    extend(n);
    return fac[n]*ifac[n-k];
  }

  Mint H(ll n,ll k){
    if(n<0 || k<0) return 0;
    if(k==0) return 1;
    if(n==0) return 0;
    i128 s=(i128)n+k-1;
    assert(s<p);
    return C((ll)s,k);
  }

  Mint multinomial(const vector<ll>& ks){
    i128 s=0;
    for(ll k:ks){
      if(k<0) return 0;
      s+=k;
    }
    assert(s<p);
    extend((ll)s);
    Mint res=fac[(ll)s];
    for(ll k:ks) res*=ifac[k];
    return res;
  }

  Mint catalan(ll n){
    if(n<0) return 0;
    i128 s=(i128)2*n;
    assert(s<p);
    return C((ll)s,n)-C((ll)s,n-1);
  }

  // n は ll の全非負範囲、min(k,n-k) < p を要求する。
  Mint C_naive(ll n,ll k){
    if(n<0 || k<0 || n<k) return 0;
    k=min(k,n-k);
    extend(k);
    Mint res=ifac[k];
    for(ll i=1;i<=k;i++) res*=Mint(n-k+i);
    return res;
  }
};

template<class Mint>
using combination=Combination<Mint>;
