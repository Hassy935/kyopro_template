#pragma once
#include <Library/core/common_setting.hpp>

class LinearSieve {
  ll n;
  vector<ll> spf, primes;

public:
  explicit LinearSieve(ll n_): n(n_) {
    assert(0<=n && n<numeric_limits<ll>::max());
    spf.assign(n+1,0);

    for(ll i=2;i<=n;i++){
      if(spf[i]==0){
        spf[i]=i;
        primes.push_back(i);
      }
      ll lim=min(spf[i],n/i);
      for(ll p:primes){
        if(p>lim) break;
        spf[i*p]=p;
      }
    }
  }

  ll limit() const { return n; }

  bool is_prime(ll x) const {
    assert(0<=x && x<=n);
    return x>=2 && spf[x]==x;
  }

  const vector<ll>& get_primes() const { return primes; }
  ll count_primes() const { return ssize(primes); }

  ll get_spf(ll x) const {
    assert(2<=x && x<=n);
    return spf[x];
  }

  // 素因数の昇順。1 の素因数分解は空列。
  vector<pair<ll,ll>> factorize(ll x) const {
    assert(1<=x && x<=n);
    vector<pair<ll,ll>> res;
    while(x>1){
      ll p=spf[x],e=0;
      do{
        x/=p;
        e++;
      }while(x>1 && spf[x]==p);
      res.emplace_back(p,e);
    }
    return res;
  }

  // 約数を重複なく列挙する。順序は保証しない。
  vector<ll> divisors(ll x) const {
    vector<ll> res{1};
    for(auto [p,e]:factorize(x)){
      ll k=ssize(res),v=1;
      for(ll i=1;i<=e;i++){
        v*=p;
        for(ll j=0;j<k;j++) res.push_back(res[j]*v);
      }
    }
    return res;
  }

  ll count_divisors(ll x) const {
    assert(1<=x && x<=n);
    ll res=1;
    while(x>1){
      ll p=spf[x],e=0;
      do{
        x/=p;
        e++;
      }while(x>1 && spf[x]==p);
      res*=e+1;
    }
    return res;
  }
};

using l_sieve = LinearSieve;
