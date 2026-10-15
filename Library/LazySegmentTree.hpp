#pragma once

template<typename S, auto op, auto e, typename F, auto action, auto composition,auto id>
struct lazy_segtree{
  static_assert(std::is_convertible_v<decltype(op), std::function<S(S, S)>>, "op must work as S(S, S)");
  static_assert(std::is_convertible_v<decltype(e), std::function<S()>>, "e must work as S()");
  static_assert(std::is_convertible_v<decltype(action), std::function<S(F, S)>>, "mapping must work as S(F, S)");
  static_assert(std::is_convertible_v<decltype(composition), std::function<F(F, F)>>, "composition must work as F(F, F)");
  static_assert(std::is_convertible_v<decltype(id), std::function<F()>>, "id must work as F()");

  ll _n, sz, height;
  vector<S> data;
  vector<F> lazy;

  explicit lazy_segtree(ll n) : _n(n) {
    sz=bit_ceil(ull(n));
    height=countr_zero(ull(sz))+1;
    data.resize(sz*2,e());
    lazy.resize(sz*2,id());
  }

  void set(ll p, S x) {
    assert(0<=p&&p<_n);

    p+=sz;
    drep(i,height) push(p>>i);
    data[p]=x;
    rep(i,height) update(p>>i);
  }

  S get(ll p) {
    assert(0<=p&&p<_n);

    p+=sz;
    drep(i,height) push(p>>i);
    return reflected(p);
  }

  S prod(ll l, ll r) {
    assert(0<=l&&l<=_n);
    assert(0<=l&&r<=_n);
    assert(l<=r);

    if(l==r) return e();
    
    l+=sz;r+=sz;

    drep(i,height) push(l>>i);
    drep(i,height) push((r-1)>>i);

    S resl=e(),resr=e();
    while(l<r){
      if(l&1){
        resl=op(resl,reflected(l));
        l++;
      }
      if(r&1){
        r--;
        resr=op(reflected(r),resr);
      }
      l>>=1;r>>=1;
    }

    return op(resl,resr);
  }

  S all_prod() const {
    return reflected(1);
  }

  void apply(ll p, F f) {
    assert(0<=p&&p<_n);

    p+=sz;

    drep(i,height) push(p>>i);
    reflect(p,f);
    rep(i,height) update(p>>i);
  }

  void apply(ll l, ll r, F f){
    assert(0<=l&&l<=_n);
    assert(0<=r&&r<=_n);
    assert(l<=r);

    if(l==r) return;

    l+=sz;r+=sz;

    drep(i,height) push(l>>i);
    drep(i,height) push((r-1)>>i);

    {
      ll l2=l,r2=r;

      while(l<r){
        if(l&1){
          reflect(l,f);
          l++;
        }
        if(r&1){
          r--;
          reflect(r,f);
        }
        l>>=1;r>>=1;
      }

      l=l2;r=r2;
    }

    rep(i,height) update(l>>i);
    rep(i,height) update((r-1)>>i);

    return;
  }

  void debug() {
    rep(i,_n) cerr << get(i) << " ";
    cerr << '\n';
  }

  private:
  bool leaf(ll k) const {
    return k>=sz;
  }

  S reflected(ll k) const {
    return action(lazy[k],data[k]);
  }

  void update(ll k) {
    if(leaf(k)) return;
    data[k]=op(reflected(2*k),reflected(2*k+1));
  }

  void reflect(ll k, F f) {
    if(leaf(k)) data[k]=action(f,data[k]);
    else lazy[k]=composition(f,lazy[k]);
    return;
  }

  void push(ll k) {
    if(leaf(k)) return;

    reflect(2*k,lazy[k]);
    reflect(2*k+1,lazy[k]);

    data[k]=reflected(k);
    lazy[k]=id();

    return;
  }
};