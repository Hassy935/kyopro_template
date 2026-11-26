#pragma once
#include <Library/core/common_setting.hpp>

// 整数集合を重複・隣接しない半開区間 [l,r) で管理する．
class IntervalSet {
public:
  using P=pair<ll,ll>;
  using const_iterator=set<P>::const_iterator;
  using iterator=const_iterator;

private:
  static constexpr ll LO=numeric_limits<ll>::min();
  static constexpr ll HI=numeric_limits<ll>::max();
  set<P> st;
  i128 sum=0;

  void insert_interval(ll l,ll r){
    if(l==r) return;
    st.emplace(l,r);
    sum+=(i128)r-l;
  }

  const_iterator erase_interval(const_iterator it){
    sum-=(i128)it->second-it->first;
    return st.erase(it);
  }

  // x を含む区間の iterator（存在しなければ end()）．
  const_iterator locate(ll x) const {
    auto it=st.upper_bound({x,HI});
    if(it==st.begin()) return st.end();
    --it;
    return x<it->second ? it : st.end();
  }

public:
  IntervalSet()=default;
  IntervalSet(const IntervalSet&)=default;
  IntervalSet& operator=(const IntervalSet&)=default;
  IntervalSet(IntervalSet&& other) noexcept {
    st.swap(other.st);
    sum=exchange(other.sum,0);
  }
  IntervalSet& operator=(IntervalSet&& other) noexcept {
    if(this!=&other){
      clear();
      st.swap(other.st);
      sum=exchange(other.sum,0);
    }
    return *this;
  }

  // [l,r) を追加する．重なる区間・隣接する区間を統合する．
  void add(ll l,ll r){
    assert(l<=r && "IntervalSet::add(): invalid range");
    if(l==r) return;
    auto it=st.lower_bound({l,LO});
    if(it!=st.begin() && prev(it)->second>=l) --it;

    while(it!=st.end() && it->first<=r){
      l=std::min(l,it->first);
      r=std::max(r,it->second);
      it=erase_interval(it);
    }
    insert_interval(l,r);
  }

  void add(ll x){
    if(x==HI) throw out_of_range("IntervalSet::add(): endpoint overflow");
    add(x,x+1);
  }

  // [l,r) と交わる部分を削除する．1点以上削除した場合 true．
  bool erase(ll l,ll r){
    assert(l<=r && "IntervalSet::erase(): invalid range");
    if(l==r) return false;
    auto it=st.lower_bound({l,LO});
    if(it!=st.begin() && prev(it)->second>l) --it;

    bool erased=false;
    while(it!=st.end() && it->first<r){
      auto [a,b]=*it;
      it=erase_interval(it);
      erased=true;
      if(a<l) insert_interval(a,l);
      if(r<b){
        insert_interval(r,b);
        break;
      }
    }
    return erased;
  }

  bool erase(ll x){
    if(x==HI) return false;
    return erase(x,x+1);
  }

  bool contains(ll x) const {
    return locate(x)!=st.end();
  }

  bool contains(ll l,ll r) const {
    assert(l<=r && "IntervalSet::contains(): invalid range");
    if(l==r) return true;
    auto it=locate(l);
    return it!=st.end() && r<=it->second;
  }

  // [l,r) 全体が存在する場合のみ削除する．空区間は成功とする．
  bool remove(ll l,ll r){
    if(!contains(l,r)) return false;
    erase(l,r);
    return true;
  }

  ll size() const {
    if(sum>HI) throw overflow_error("IntervalSet::size(): size overflow");
    return (ll)sum;
  }
  ll count() const { return size(); }
  ll interval_count() const { return (ll)st.size(); }
  bool empty() const { return st.empty(); }

  void clear(){
    st.clear();
    sum=0;
  }

  // x を含む区間を返す．
  optional<P> get(ll x) const {
    auto it=locate(x);
    if(it==st.end()) return nullopt;
    return *it;
  }

  // x 以上の最小の未包含整数．
  ll mex(ll x=0) const {
    auto it=locate(x);
    return it==st.end() ? x : it->second;
  }

  ll min() const {
    assert(!empty() && "IntervalSet::min(): empty set");
    return st.begin()->first;
  }

  ll max() const {
    assert(!empty() && "IntervalSet::max(): empty set");
    return st.rbegin()->second-1;
  }

  // x 以下の最大の包含整数．
  optional<ll> le(ll x) const {
    auto it=st.upper_bound({x,HI});
    if(it==st.begin()) return nullopt;
    --it;
    return std::min(x,it->second-1);
  }

  optional<ll> lt(ll x) const {
    if(x==LO) return nullopt;
    return le(x-1);
  }

  // x 以上の最小の包含整数．
  optional<ll> ge(ll x) const {
    auto it=st.upper_bound({x,HI});
    if(it!=st.begin() && x<prev(it)->second) return x;
    if(it==st.end()) return nullopt;
    return it->first;
  }

  optional<ll> gt(ll x) const {
    if(x==HI) return nullopt;
    return ge(x+1);
  }

  bool intersects(ll l,ll r) const {
    assert(l<=r && "IntervalSet::intersects(): invalid range");
    if(l==r) return false;
    auto x=ge(l);
    return x && *x<r;
  }

  // 要素単位ではなく区間単位で走査する．
  const_iterator begin() const { return st.cbegin(); }
  const_iterator end() const { return st.cend(); }
};

using interval_set=IntervalSet;
