#pragma once

// 制約：n>=0．
// 0は素数でない扱い．
// 2以上n以下の整数kの素因数分解がO(log(k))でできる．
struct sieve{
  private:
  vl spf;
  vl primes;
  ll n;

  public:
  explicit sieve(ll n_): spf(n_+1,-1),n(n_) {
    assert(n_>=0);

    forc(i,2,n){
      if(spf[i]==-1){
        primes.pb(i);
        spf[i]=i;
      }

      forr(p,primes){
        if(i*p>n) break;
        if(p>spf[i]) break;

        spf[i*p]=p;
      }
    }
  }

  ll get_spf(ll i) const {
    assert(2<=i&&i<=n);

    return spf[i];
  }

  vl get_primes(void) const {
    return primes;
  }

  bool is_prime(ll i) const {
    assert(0<=i&&i<=n);
    
    if(i==0||i==1) return false;

    return spf[i]==i;
  }

  vector<pair<ll,ll>> factorize(ll x) const {
    assert(2<=x&&x<=n);

    vector<pair<ll,ll>> res;
    while(x>1){
      ll p=spf[x],e=0;
      while(x%p==0){
        e++;
        x/=p;
      }
      res.eb(p,e);
    }

    return res;
  }
};