#pragma once

struct BIT_ll {
  ll n;
  vector<ll> data;
  
  BIT_ll() : n(0) {}

  explicit BIT_ll(ll n_) : n(n_), data(n_ + 1, 0) {}
  
  // A[0] = A[1] = ... = A[n-1] = x
  explicit BIT_ll(ll n_, ll x) : n(n_), data(n_ + 1, 0) {
    for (ll i = 1; i <= n; i++) {
      data[i] = x * (i & -i);
    }
  }

  explicit BIT_ll(const vector<ll>& v) : n((ll)v.size()), data(n + 1, 0) {
    for (ll i = 0; i < n; i++) {
      data[i + 1] += v[i];
    }

    for (ll i = 1; i <= n; i++) {
      ll j = i + (i & -i);
      if (j <= n) data[j] += data[i];
    }
  }

  // A[p] += x
  inline void add(ll p, ll x) {
    assert(0 <= p && p < n);

    p++;
    while (p <= n) {
      data[p] += x;
      p += p & -p;
    }
  }

  // A[0] + ... + A[r-1]
  inline ll sum(ll r) const {
    assert(0 <= r && r <= n);

    ll res = 0;
    while (r > 0) {
      res += data[r];
      r -= r & -r;
    }

    return res;
  }

  // A[l] + ... + A[r-1]
  inline ll sum(ll l, ll r) const {
    assert(0 <= l && l <= r && r <= n);

    return sum(r) - sum(l);
  }

  inline ll all_sum() const {
    return sum(n);
  }

  inline ll get(ll p) const {
    assert(0 <= p && p < n);

    return sum(p, p + 1);
  }

  // A[p] = x
  inline void update(ll p, ll x) {
    assert(0 <= p && p < n);

    add(p, x - get(p));
  }

  // A[0] + ... + A[r] >= x となる最小の r を返す
  // 存在しない場合は n を返す
  // 各 A[i] >= 0 が必要
  inline ll lower_bound(ll x) const {
    if (x <= 0) return 0;
    if (all_sum() < x) return n;

    ll idx = 0;
    ll acc = 0;

    ll k = 1;
    while ((k << 1) <= n) k <<= 1;

    while (k > 0) {
      ll nxt = idx + k;

      if (nxt <= n && acc + data[nxt] < x) {
        acc += data[nxt];
        idx = nxt;
      }

      k >>= 1;
    }

    return idx;
  }

  // k 番目の要素の index を返す
  // k は 1-indexed
  inline ll kth(ll k) const {
    return lower_bound(k);
  }
};

template<class S>
struct BIT_sum {
  ll n;
  vector<S> data;
  
  BIT_sum() : n(0) {}

  explicit BIT_sum(ll n_) : n(n_), data(n_ + 1, S(0)) {}
  
  // A[0] = A[1] = ... = A[n-1] = x
  explicit BIT_sum(ll n_, S x) : n(n_), data(n_ + 1, S(0)) {
    for (ll i = 1; i <= n; i++) {
      data[i] = x * (i & -i);
    }
  }

  explicit BIT_sum(const vector<S>& v) : n((ll)v.size()), data(n + 1, S(0)) {
    for (ll i = 0; i < n; i++) {
      data[i + 1] += v[i];
    }

    for (ll i = 1; i <= n; i++) {
      ll j = i + (i & -i);
      if (j <= n) data[j] += data[i];
    }
  }

  // A[p] += x
  inline void add(ll p, S x) {
    assert(0 <= p && p < n);

    p++;
    while (p <= n) {
      data[p] += x;
      p += p & -p;
    }
  }

  // A[0] + ... + A[r-1]
  inline S sum(ll r) const {
    assert(0 <= r && r <= n);

    S res = S(0);
    while (r > 0) {
      res += data[r];
      r -= r & -r;
    }

    return res;
  }

  // A[l] + ... + A[r-1]
  inline S sum(ll l, ll r) const {
    assert(0 <= l && l <= r && r <= n);

    return sum(r) - sum(l);
  }

  inline S all_sum() const {
    return sum(n);
  }

  inline S get(ll p) const {
    assert(0 <= p && p < n);

    return sum(p, p + 1);
  }

  // A[p] = x
  inline void update(ll p, S x) {
    assert(0 <= p && p < n);

    add(p, x - get(p));
  }
};