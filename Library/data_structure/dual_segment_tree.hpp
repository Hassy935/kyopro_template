#pragma once

#include <Library/core/common_setting.hpp>

// composition(f,g)は先にg，後にfを適用する作用を表す．
// mapping(composition(f,g),x) == mapping(f,mapping(g,x)) を満たすことが必要．
template<class S, class F, auto mapping, auto composition, auto id>
class DualSegmentTree {
  ll n_ = 0, size_ = 1, log_ = 0;
  vector<S> data_;
  vector<F> lazy_;

  void init(){
    while(size_ < n_){
      size_ <<= 1;
      log_++;
    }
    lazy_.assign(size_, id());
  }

  void all_apply(ll k, F f){
    if(k < size_){
      lazy_[k] = composition(f, F(lazy_[k]));
    }else if(k - size_ < n_){
      data_[k - size_] = mapping(f, S(data_[k - size_]));
    }
  }

  void push(ll k){
    F f = lazy_[k];
    all_apply(2*k, f);
    all_apply(2*k+1, f);
    lazy_[k] = id();
  }

  void push_path(ll k){
    dforc(h,log_,1) push(k>>h);
  }

public:
  DualSegmentTree() : DualSegmentTree(vector<S>{}) {}

  explicit DualSegmentTree(ll n, S x = S{}) : n_(n){
    assert(n >= 0);
    data_.assign(n, x);
    init();
  }

  explicit DualSegmentTree(vector<S> v)
      : n_(ssize(v)), data_(move(v)){
    init();
  }

  ll size() const { return n_; }

  S get(ll p) const {
    assert(0 <= p && p < n_);
    F f = id();
    for(ll k=(p+size_)>>1; k>=1; k>>=1){
      f = composition(F(lazy_[k]), f);
    }
    return mapping(f, S(data_[p]));
  }

  void set(ll p, S x){
    assert(0 <= p && p < n_);
    push_path(p+size_);
    data_[p] = x;
  }

  void apply(ll p, F f){
    assert(0 <= p && p < n_);
    p += size_;
    push_path(p);
    all_apply(p, f);
  }

  void apply(ll l, ll r, F f){
    assert(0 <= l && l <= r && r <= n_);
    if(l == r) return;

    l += size_;
    r += size_;

    dforc(h,log_,1){
      if(((l>>h)<<h) != l) push(l>>h);
      if(((r>>h)<<h) != r) push((r-1)>>h);
    }

    while(l < r){
      if(l&1) all_apply(l++, f);
      if(r&1) all_apply(--r, f);
      l >>= 1;
      r >>= 1;
    }
  }
};

template<class S, class F, auto mapping, auto composition, auto id>
using dual_segtree = DualSegmentTree<S, F, mapping, composition, id>;
