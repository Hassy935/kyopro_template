#pragma once

#include <Library/core/common_setting.hpp>

class LCA{
  ll root_;
  vl depth_;
  vvl up_;
  vector<i128> prefix_;

  template<bool Weighted,class Graph>
  void build(const Graph& G){
    const ll n=ssize(G);
    assert(n>0 && 0<=root_ && root_<n);
    const int levels=max(1,std::bit_width(ull(n-1)));
    up_.resize(levels);
    vl& par=up_[0];
    par.assign(n,-1);
    depth_.resize(n);
    if constexpr(Weighted) prefix_.resize(n);

    vl order;
    order.reserve(n);
    par[root_]=root_;
    order.push_back(root_);
    for(ll i=0;i<ssize(order);i++){
      const ll v=order[i];
      for(const auto& edge:G[v]){
        const ll next=[&]{
          if constexpr(Weighted) return edge.first;
          else return edge;
        }();
        assert(0<=next && next<n);
        if(par[next]!=-1) continue;
        par[next]=v;
        depth_[next]=depth_[v]+1;
        if constexpr(Weighted) prefix_[next]=prefix_[v]+edge.second;
        order.push_back(next);
      }
    }
    assert(ssize(order)==n);

    // up_[j][v] は v の 2^j 個上の祖先．根では自分自身に留まる．
    for(int j=1;j<levels;j++){
      const vl& prev=up_[j-1];
      vl& curr=up_[j];
      curr.resize(n);
      for(ll v=0;v<n;v++) curr[v]=prev[prev[v]];
    }
  }

  bool valid(ll v) const { return 0<=v && v<size(); }

  ll lift(ll v,ll k) const {
    for(int j=0;k>0;j++,k>>=1)
      if(k&1) v=up_[j][v];
    return v;
  }

  ll edge_count(ll u,ll v,ll a) const {
    return (depth_[u]-depth_[a])+(depth_[v]-depth_[a]);
  }

public:
  LCA(const vvl& G,ll root=0) : root_(root){ build<false>(G); }
  LCA(const vvc<pair<ll,ll>>& G,ll root=0) : root_(root){ build<true>(G); }

  ll size() const { return ssize(depth_); }
  ll root() const { return root_; }

  ll depth(ll v) const {
    assert(valid(v));
    return depth_[v];
  }

  ll parent(ll v) const {
    assert(valid(v));
    return v==root_ ? -1 : up_[0][v];
  }

  ll lca(ll u,ll v) const {
    assert(valid(u) && valid(v));
    if(depth_[u]<depth_[v]) swap(u,v);
    u=lift(u,depth_[u]-depth_[v]);
    if(u==v) return u;
    for(ll j=ssize(up_)-1;j>=0;j--){
      if(up_[j][u]==up_[j][v]) continue;
      u=up_[j][u];
      v=up_[j][v];
    }
    return up_[0][u];
  }

  ll lca(ll u,ll v,ll r) const {
    assert(valid(r));
    const ll a=lca(u,v),b=lca(u,r),c=lca(v,r);
    const ll deeper=depth_[a]<depth_[b] ? b : a;
    return depth_[deeper]<depth_[c] ? c : deeper;
  }

  ll dist(ll u,ll v) const {
    const ll a=lca(u,v);
    return edge_count(u,v,a);
  }

  ll weighted_dist(ll u,ll v) const {
    const ll a=lca(u,v);
    if(prefix_.empty()) return edge_count(u,v,a);
    const i128 d=(prefix_[u]-prefix_[a])+(prefix_[v]-prefix_[a]);
    assert(numeric_limits<ll>::min()<=d && d<=numeric_limits<ll>::max());
    return ll(d);
  }

  ll kth_ancestor(ll v,ll k) const {
    assert(valid(v) && k>=0);
    if(k>depth_[v]) return -1;
    return lift(v,k);
  }

  ll jump(ll u,ll v,ll k) const {
    assert(k>=0);
    const ll a=lca(u,v);
    const ll up=depth_[u]-depth_[a],down=depth_[v]-depth_[a];
    if(k<=up) return lift(u,k);
    if(k-up>down) return -1;
    return lift(v,down-(k-up));
  }

  bool is_ancestor(ll u,ll v) const {
    assert(valid(u) && valid(v));
    return depth_[u]<=depth_[v] && lift(v,depth_[v]-depth_[u])==u;
  }

  bool on_path(ll u,ll v,ll x) const {
    assert(valid(x));
    const ll a=lca(u,v);
    // パスは a から両端への祖先列で構成される．
    return depth_[x]>=depth_[a] && (is_ancestor(x,u) || is_ancestor(x,v));
  }

  vl path(ll u,ll v) const {
    const ll a=lca(u,v);
    vl result;
    result.reserve(edge_count(u,v,a)+1);
    for(;u!=a;u=up_[0][u]) result.push_back(u);
    result.push_back(a);
    const ll suffix=ssize(result);
    for(;v!=a;v=up_[0][v]) result.push_back(v);
    reverse(result.begin()+suffix,result.end());
    return result;
  }
};

using lca=LCA;
