#pragma once

// mintが定義されている必要がある．
struct combination {
  ll n;
  vector<mint> fac, ifac;

  combination() : n(0) {}

  explicit combination(ll n_) : n(n_), fac(n_ + 1), ifac(n_ + 1) {
    assert(n_ >= 0);

    fac[0] = 1;
    for (ll i = 1; i <= n; i++) {
      fac[i] = fac[i - 1] * i;
    }

    ifac[n] = fac[n].inv();
    for (ll i = n; i >= 1; i--) {
      ifac[i - 1] = ifac[i] * i;
    }
  }

  // x!
  mint fact(ll x) const {
    assert(0 <= x && x <= n);
    return fac[x];
  }

  // 1 / x!
  mint inv_fact(ll x) const {
    assert(0 <= x && x <= n);
    return ifac[x];
  }

  // aCb
  mint C(ll a, ll b) const {
    if (a < 0 || b < 0 || a < b) return 0;
    assert(0 <= a && a <= n);
    return fac[a] * ifac[b] * ifac[a - b];
  }

  // aPb
  mint P(ll a, ll b) const {
    if (a < 0 || b < 0 || a < b) return 0;
    assert(0 <= a && a <= n);
    return fac[a] * ifac[a - b];
  }

  // 重複組合せ
  // a種類から重複を許してb個選ぶ: aHb = C(a+b-1, b)
  mint H(ll a, ll b) const {
    if (a < 0 || b < 0) return 0;
    if (b == 0) return 1;
    if (a == 0) return 0;
    return C(a + b - 1, b);
  }

  // 多項係数
  // (k1+k2+...+km)! / (k1! k2! ... km!)
  mint multinomial(const vector<ll>& ks) const {
    ll s = 0;
    for (ll x : ks) {
      if (x < 0) return 0;
      s += x;
    }

    assert(0 <= s && s <= n);

    mint res = fac[s];
    for (ll x : ks) {
      res *= ifac[x];
    }
    return res;
  }

  // カタラン数
  // Cat_x = C(2x,x) - C(2x,x-1)
  mint catalan(ll x) const {
    if (x < 0) return 0;
    assert(0 <= 2*x && 2*x <= n);
    return C(2 * x, x) - C(2 * x, x - 1);
  }
};