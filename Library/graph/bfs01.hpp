#pragma once

#include <Library/core/common_setting.hpp>

namespace bfs01_detail {
template<bool WithPath=false>
auto run(const vvc<pair<ll,ll>>& G, span<const ll> sources, optional<ll> target=nullopt){
  const ll n=ssize(G);
  const ll t=target.value_or(-1);
  assert(!target || (0<=t && t<n));
  vl dist(n,-1),par;
  if constexpr(WithPath) par.assign(n,-1);
  deque<pair<ll,ll>> qe;

  for(ll s:sources){
    assert(0<=s && s<n);
    if(dist[s]!=-1) continue;
    dist[s]=0;
    if constexpr(WithPath) par[s]=s;
    qe.emplace_back(0,s);
  }

  // 候補を距離順に処理し，更新前の候補は走査せずに捨てる．
  while(!qe.empty()){
    const auto [d,v]=qe.front();
    qe.pop_front();
    if(d!=dist[v]) continue;
    if(v==t) break;
    for(const auto& [nv,cost]:G[v]){
      assert(0<=nv && nv<n);
      assert(cost==0 || cost==1);
      const ll nd=d+cost;
      if(dist[nv]!=-1 && dist[nv]<=nd) continue;
      dist[nv]=nd;
      if constexpr(WithPath) par[nv]=v;
      if(cost==0) qe.emplace_front(nd,nv);
      else qe.emplace_back(nd,nv);
    }
  }

  if constexpr(WithPath) return pair<vl,vl>{move(dist),move(par)};
  else return dist;
}
} // namespace bfs01_detail

inline vl bfs01(const vvc<pair<ll,ll>>& G, ll s){
  return bfs01_detail::run(G,span<const ll>(&s,1));
}

inline vl bfs01(const vvc<pair<ll,ll>>& G, const vl& sources){
  return bfs01_detail::run(G,sources);
}

inline ll bfs01(const vvc<pair<ll,ll>>& G, ll s, ll t){
  return bfs01_detail::run(G,span<const ll>(&s,1),t)[t];
}

inline ll bfs01(const vvc<pair<ll,ll>>& G, const vl& sources, ll t){
  return bfs01_detail::run(G,sources,t)[t];
}

inline pair<vl,vl> bfs01_with_path(const vvc<pair<ll,ll>>& G, ll s){
  return bfs01_detail::run<true>(G,span<const ll>(&s,1));
}

inline pair<vl,vl> bfs01_with_path(const vvc<pair<ll,ll>>& G, const vl& sources){
  return bfs01_detail::run<true>(G,sources);
}
