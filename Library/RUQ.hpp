#pragma once

struct RUQ{
  using P = pair<ll, ll>;
  vc<P> data;
  ll sz, time=0;

  RUQ(ll n, ll init=0){
    sz=bit_ceil((unsigned long long)n);
    data.assign(2*sz,P(init,0));
  }

  void update(ll l, ll r, ll val){
    time++;

    l+=sz;
    r+=sz;

    while(l<r){
      if(l&1){
        data[l]=P(val,time);
        l++;
      }
      if(r&1){
        r--;
        data[r]=P(val,time);
      }
      l/=2;
      r/=2;
    }

    return;
  }

  ll get(ll pos){
    pos+=sz;
    ll res=data[pos].fi, latest=data[pos].se;
    while(pos>=2){
      pos/=2;
      if(chmax(latest,data[pos].se)){
        res=data[pos].fi;
      }
    }

    return res;
  }
};

template<class T>
struct RUQ{
  using Node = pair<T, ll>;
  vc<Node> data;
  ll n_,sz, time=0;

  explicit RUQ(ll n, const T& init=T{}){
    n_=n;
    sz=bit_ceil((unsigned long long)n_);
    data.assign(2*sz,Node(init,0));
  }

  void update(ll l, ll r, const T& val){
    assert(0<=l&&l<=n_);
    assert(0<=r&&r<=n_);
    assert(l<=r);
    time++;

    l+=sz;
    r+=sz;

    while(l<r){
      if(l&1){
        data[l]=Node(val,time);
        l++;
      }
      if(r&1){
        r--;
        data[r]=Node(val,time);
      }
      l/=2;
      r/=2;
    }
    return;
  }

  T get(ll pos) const {
    assert(0<=pos&&pos<n_);
    pos+=sz;
    T res=data[pos].fi;
    ll latest=data[pos].se;
    while(pos>=2){
      pos/=2;
      if(chmax(latest,data[pos].se)){
        res=data[pos].fi;
      }
    }
    return res;
  }
};