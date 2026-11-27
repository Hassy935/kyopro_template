#pragma once
#include <Library/core/common_setting.hpp>

template<class T>
struct compress {
  vector<T> xs;

  compress() {}

  explicit compress(const vector<T>& v) : xs(v) {
    build();
  }

  template<class... Args>
  void add(const T& x, const Args&... args){
    xs.push_back(x);
    (xs.push_back(args), ...);
  }

  void build(){
    sort(nall(xs));
    xs.erase(unique(nall(xs)),xs.end());
  }

  // x の圧縮後 index．未登録なら assert．
  ll get(const T& x) const {
    auto it=lower_bound(nall(xs),x);
    assert(it!=xs.end()&&*it==x);
    return it-xs.begin();
  }

  ll operator()(const T& x) const {
    return get(x);
  }

  typename vector<T>::const_reference operator[](ll i) const {
    assert(0<=i&&i<ssize(xs));
    return xs[i];
  }

  ll size() const { return ssize(xs); }
  bool empty() const { return xs.empty(); }

  bool contains(const T& x) const {
    auto it=lower_bound(nall(xs),x);
    return it!=xs.end()&&*it==x;
  }

  ll lower_bound_index(const T& x) const {
    return lower_bound(nall(xs),x)-xs.begin();
  }

  ll upper_bound_index(const T& x) const {
    return upper_bound(nall(xs),x)-xs.begin();
  }
};
