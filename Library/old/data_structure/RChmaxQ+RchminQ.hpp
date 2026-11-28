#pragma once

template<class T>
struct RChmaxQ {
  vc<T> data;
  ll n_, sz;

  explicit RChmaxQ(ll n, const T& init) {
    n_ = n;
    sz = bit_ceil((unsigned long long)n_);
    data.assign(2 * sz, init);
  }

  // [l,r) を val で chmax
  void chmax(ll l, ll r, const T& val) {
    assert(0 <= l && l <= n_);
    assert(0 <= r && r <= n_);
    assert(l <= r);

    l += sz;
    r += sz;

    while (l < r) {
      if (l & 1) {
        if (data[l] < val) data[l] = val;
        l++;
      }
      if (r & 1) {
        r--;
        if (data[r] < val) data[r] = val;
      }
      l /= 2;
      r /= 2;
    }
  }

  // pos を val で chmax
  void chmax(ll pos, const T& val) {
    assert(0 <= pos && pos < n_);

    pos += sz;
    if (data[pos] < val) data[pos] = val;
  }

  T get(ll pos) const {
    assert(0 <= pos && pos < n_);

    pos += sz;

    T res = data[pos];
    while (pos >= 2) {
      pos /= 2;
      if (res < data[pos]) res = data[pos];
    }

    return res;
  }
};

template<class T>
struct RChminQ {
  vc<T> data;
  ll n_, sz;

  explicit RChminQ(ll n, const T& init) {
    n_ = n;
    sz = bit_ceil((unsigned long long)n_);
    data.assign(2 * sz, init);
  }

  // [l,r) を val で chmin
  void chmin(ll l, ll r, const T& val) {
    assert(0 <= l && l <= n_);
    assert(0 <= r && r <= n_);
    assert(l <= r);

    l += sz;
    r += sz;

    while (l < r) {
      if (l & 1) {
        if (data[l] > val) data[l] = val;
        l++;
      }
      if (r & 1) {
        r--;
        if (data[r] > val) data[r] = val;
      }

      l /= 2;
      r /= 2;
    }
  }

  // pos を val で chmin
  void chmin(ll pos, const T& val) {
    assert(0 <= pos && pos < n_);

    pos += sz;
    if (data[pos] > val) data[pos] = val;
  }

  T get(ll pos) const {
    assert(0 <= pos && pos < n_);

    pos += sz;

    T res = data[pos];

    while (pos >= 2) {
      pos /= 2;
      if (res > data[pos]) res = data[pos];
    }

    return res;
  }
};