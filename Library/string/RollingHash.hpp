#pragma once

struct RollingHash{
  static constexpr ll BASE0=269, BASE1=337;
  static constexpr ll MOD0=998244353, MOD1=1000000007;
  ll n;
  vl hash0,hash1,pow0,pow1;

  explicit RollingHash(const string& S){
    n=ssize(S);

    hash0.resize(n+1); hash1.resize(n+1);
    pow0.resize(n+1,1); pow1.resize(n+1,1);

    rep(i,n){
      hash0[i+1]=(hash0[i]*BASE0+S[i])%MOD0;
      hash1[i+1]=(hash1[i]*BASE1+S[i])%MOD1;
      pow0[i+1]=(pow0[i]*BASE0)%MOD0;
      pow1[i+1]=(pow1[i]*BASE1)%MOD1;
    }
  }

  using P = pair<ll, ll>;
  inline P get(ll l, ll r) const {
    assert(0<=l&&l<=n);
    assert(0<=r&&r<=n);
    assert(l<=r);

    ll res0=(hash0[r]-hash0[l]*pow0[r-l])%MOD0;
    if(res0<0) res0+=MOD0;

    ll res1=(hash1[r]-hash1[l]*pow1[r-l])%MOD1;
    if(res1<0) res1+=MOD1;

    return {res0,res1};
  }

  inline P get_all() const {
    return get(0,n);
  }
};