#pragma once
#include <Library/core/common_setting.hpp>

class UnionFind {
  // 根は負の成分サイズ，それ以外は親の頂点番号．
  vl parent_or_size;
  ll cc=0;

  static size_t checked_size(ll n){
    if(n<0 || std::cmp_greater(n,vl{}.max_size())){
      assert(false && "UnionFind: invalid size");
      throw std::length_error("UnionFind: invalid size");
    }
    return static_cast<size_t>(n);
  }

public:
  UnionFind()=default;
  explicit UnionFind(ll n): parent_or_size(checked_size(n),-1),cc(n) {}

  UnionFind(const UnionFind&)=default;
  UnionFind& operator=(const UnionFind&)=default;

  UnionFind(UnionFind&& other) noexcept
      : parent_or_size(std::move(other.parent_or_size)),cc(std::exchange(other.cc,0)){
    other.parent_or_size.clear();
  }

  UnionFind& operator=(UnionFind&& other) noexcept {
    if(this!=&other){
      parent_or_size=std::move(other.parent_or_size);
      cc=std::exchange(other.cc,0);
      other.parent_or_size.clear();
    }
    return *this;
  }

  ll size() const { return std::ssize(parent_or_size); }
  bool empty() const { return parent_or_size.empty(); }
  ll comp_cnt() const { return cc; }

  ll leader(ll x){
    assert(0<=x && x<size());
    while(parent_or_size[x]>=0){
      const ll parent=parent_or_size[x],grandparent=parent_or_size[parent];
      if(grandparent<0) return parent;
      parent_or_size[x]=grandparent;
      x=parent;
    }
    return x;
  }

  bool merge(ll x,ll y){
    x=leader(x);
    y=leader(y);
    if(x==y) return false;
    if(parent_or_size[x]>parent_or_size[y]) std::swap(x,y);
    parent_or_size[x]+=parent_or_size[y];
    parent_or_size[y]=x;
    --cc;
    return true;
  }

  bool same(ll x,ll y){ return leader(x)==leader(y); }
  ll size(ll x){ return -parent_or_size[leader(x)]; }

  vvl groups() const {
    vvl result(cc);
    vl group_id(size(),-1);
    ll next=0;
    for(ll root=0;root<size();++root){
      if(parent_or_size[root]>=0) continue;
      result[next].reserve(-parent_or_size[root]);
      group_id[root]=next++;
    }

    // 各経路を既知のグループまでたどり，途中の所属を一時配列に記録する．
    for(ll x=0;x<size();++x){
      ll p=x;
      while(group_id[p]<0) p=parent_or_size[p];
      const ll id=group_id[p];
      for(p=x;group_id[p]<0;p=parent_or_size[p]) group_id[p]=id;
      result[id].push_back(x);
    }
    return result;
  }

#ifdef LOCAL
  vvl debug_view() const { return groups(); }
#endif
};

using union_find = UnionFind;
