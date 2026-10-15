#pragma once

struct IntervalSet{
  using P = pair<ll,ll>;

  set<P> st;
  ll sum=0; // 区間の和集合の要素数

  // [l,r) を内部的に追加する．
  void insert_interval(ll l, ll r){
    if(l==r) return;
    st.emp(l,r);
    sum+=r-l;
  }

  // it の指す区間を内部的に削除する．
  // 削除した次の iterator を返す．
  auto erase_interval(set<P>::iterator it){
    auto [l,r]=*it;
    sum-=r-l;
    return st.erase(it);
  }

  // [l,r) を追加する．
  // 重なる，または隣接する区間は1つに統合する．
  // 償却 O(log Q)
  void add(ll l, ll r){
    assert(l<=r);
    if(l==r) return;

    auto it=st.lower_bound({l,numeric_limits<ll>::min()});

    if(it!=st.begin()){
      auto pit=prev(it);
      if(pit->second>=l){
        it=pit;
      }
    }

    while(it!=st.end()&&it->first<=r){
      l=min(l,it->first);
      r=max(r,it->second);
      it=erase_interval(it);
    }

    insert_interval(l,r);
  }

  // [l,r) と交わる部分をすべて削除する．
  // 実際に1点以上削除した場合 true を返す．
  // 償却 O(log Q)
  bool erase(ll l, ll r){
    assert(l<=r);
    if(l==r) return false;

    auto it=st.lower_bound({l,numeric_limits<ll>::min()});

    if(it!=st.begin()){
      auto pit=prev(it);
      if(pit->second>l){
        it=pit;
      }
    }

    bool erased=false;

    while(it!=st.end()&&it->first<r){
      auto [a,b]=*it;

      erased=true;
      it=erase_interval(it);

      if(a<l){
        insert_interval(a,l);
      }

      if(r<b){
        insert_interval(r,b);
        break;
      }
    }

    return erased;
  }

  // [l,r) を含む区間が存在するか調べる．
  // O(log Q)
  bool contains(ll l, ll r) const {
    assert(l<r);

    auto it=st.upper_bound({l,numeric_limits<ll>::max()});
    if(it==st.begin()) return false;
    it--;

    return r<=it->second;
  }

  // x を含む区間が存在するか調べる．
  // O(log Q)
  bool contains(ll x) const {
    auto it=st.upper_bound({x,numeric_limits<ll>::max()});
    if(it==st.begin()) return false;
    it--;

    return x<it->second;
  }

  // [l,r) を含む区間が存在する場合，[l,r) を削除する．
  // 存在して削除できた場合 true を返す．
  // O(log Q)
  bool remove(ll l, ll r){
    assert(l<r);

    auto it=st.upper_bound({l,numeric_limits<ll>::max()});
    if(it==st.begin()) return false;
    it--;

    auto [a,b]=*it;

    if(r>b) return false;

    erase_interval(it);

    if(a<l){
      insert_interval(a,l);
    }
    if(r<b){
      insert_interval(r,b);
    }

    return true;
  }

  // 区間の和集合の要素数を返す．
  // O(1)
  ll count() const {
    return sum;
  }

  // x以上で，集合に含まれていない最小の整数を返す．
  // O(log Q)
  ll mex(ll x) const {
    auto it=st.upper_bound({x,numeric_limits<ll>::max()});

    if(it==st.begin()) return x;
    it--;

    if(it->second>x) return it->second;
    return x;
  }
};