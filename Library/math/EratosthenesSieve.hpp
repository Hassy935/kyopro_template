#pragma once

// 制約：n>=0．
// 0は素数でない扱い．
// 0以上n以下の整数が素数かどうか判定する．
struct sieve{
  private:
  vb prime;
  ll n;

  public:
  explicit sieve(ll n_): prime(n_+1,true),n(n_) {
    assert(n_>=0);

    prime[0]=false;
    if(n>=1) prime[1]=false;
    for(ll i=2; i*i<=n; i++){
      if(!prime[i]) continue;

      for(ll k=i*i; k<=n; k+=i) prime[k]=false;
    }
  }

  bool is_prime(ll i) const {
    assert(0<=i&&i<=n);

    return prime[i];
  }
};