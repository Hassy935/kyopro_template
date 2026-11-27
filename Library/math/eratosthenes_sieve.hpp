#pragma once
#include <Library/core/common_setting.hpp>

class EratosthenesSieve {
  ll n;
  vector<bool> prime;

public:
  explicit EratosthenesSieve(ll n_): n(n_) {
    assert(0<=n && n<numeric_limits<ll>::max());
    prime.assign(n+1,true);
    prime[0]=false;
    if(n>=1) prime[1]=false;

    for(ll i=2;i<=n/i;i++){
      if(!prime[i]) continue;
      for(ll j=i*i;;j+=i){
        prime[j]=false;
        if(j>n-i) break;
      }
    }
  }

  ll limit() const { return n; }

  bool is_prime(ll x) const {
    assert(0<=x && x<=n);
    return prime[x];
  }

  // 素数列は必要時に生成する（昇順）。
  vector<ll> get_primes() const {
    vector<ll> res;
    for(ll i=2;i<=n;i++) if(prime[i]) res.push_back(i);
    return res;
  }

  ll count_primes() const {
    return count(prime.begin(),prime.end(),true);
  }
};

using e_sieve = EratosthenesSieve;
