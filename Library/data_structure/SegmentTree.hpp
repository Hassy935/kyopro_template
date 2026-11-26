#pragma once

template <typename S, auto op, auto e>
struct segtree{
  static_assert(std::is_convertible_v<decltype(op), std::function<S(S, S)>>, "op must work as S(S, S)");
  static_assert(std::is_convertible_v<decltype(e), std::function<S()>>, "e must work as S()");

  ll _n,sz;
  vector<S> data;

  explicit segtree(ll n): _n(n) {
    sz=bit_ceil(ull(n));
    data.assign(2*sz,e());
  }

  void set(ll p, S x){
    assert(0<=p&&p<_n);

    
    p+=sz;
    data[p]=x;

    while(p>1){
      p>>=1;

      data[p]=op(left_val(p),right_val(p));
    }

    return;
  }

  S get(ll p) const {
    assert(0<=p&&p<_n);

    return data[sz+p];
  }

  S prod(ll l, ll r) const {
    assert(0<=l&&l<=_n);
    assert(0<=r&&r<=_n);
    assert(l<=r);

    l+=sz;r+=sz;

    S resl=e(),resr=e();

    while(l<r){
      if(l&1){
        resl=op(resl,data[l]);
        l++;
      }
      if(r&1){
        r--;
        resr=op(data[r],resr);
      }
      l>>=1;r>>=1;
    }

    return op(resl,resr);
  }

  S all_prod() const {
    return data[1];
  }

  private:
  S left_val(ll p) const {
    return data[2*p];
  }

  S right_val(ll p) const {
    return data[2*p+1];
  }
};