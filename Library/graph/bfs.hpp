#pragma once

#include <Library/core/common_setting.hpp>

namespace bfs_detail {
template<bool WithPath=false>
auto run(const vvl& G, span<const ll> sources, optional<ll> target=nullopt){
  const ll n=ssize(G);
  const ll t=target.value_or(-1);
  assert(!target || (0<=t && t<n));
  vl dist(n,-1),par;
  if constexpr(WithPath) par.assign(n,-1);
  queue<ll> qe;

  for(ll s:sources){
    assert(0<=s && s<n);
    if(dist[s]!=-1) continue;
    dist[s]=0;
    if constexpr(WithPath) par[s]=s;
    qe.push(s);
  }

  while(!qe.empty()){
    const ll v=qe.front();
    qe.pop();
    if(v==t) break;
    for(ll nv:G[v]){
      assert(0<=nv && nv<n);
      if(dist[nv]!=-1) continue;
      dist[nv]=dist[v]+1;
      if constexpr(WithPath) par[nv]=v;
      qe.push(nv);
    }
  }

  if constexpr(WithPath) return pair<vl,vl>{move(dist),move(par)};
  else return dist;
}
} // namespace bfs_detail

inline vl bfs(const vvl& G, ll s){
  return bfs_detail::run(G,span<const ll>(&s,1));
}

inline vl bfs(const vvl& G, const vl& sources){
  return bfs_detail::run(G,sources);
}

inline ll bfs(const vvl& G, ll s, ll t){
  return bfs_detail::run(G,span<const ll>(&s,1),t)[t];
}

inline ll bfs(const vvl& G, const vl& sources, ll t){
  return bfs_detail::run(G,sources,t)[t];
}

inline pair<vl,vl> bfs_with_path(const vvl& G, ll s){
  return bfs_detail::run<true>(G,span<const ll>(&s,1));
}

inline pair<vl,vl> bfs_with_path(const vvl& G, const vl& sources){
  return bfs_detail::run<true>(G,sources);
}

// 頂点番号は 0-indexed．s, g および隣接頂点は範囲内とする．
inline bool is_reachable(const vvl& G, ll s, ll g){
  ll N=ssize(G);
  assert(0<=s&&s<N&&0<=g&&g<N);
  if(s==g) return true;
  queue<ll> qe;
  vb visited(N);
  visited[s]=true;
  qe.push(s);

  while(!qe.empty()){
    ll v=qe.front();
    qe.pop();
    if(v==g) return true;
    forr(nv,G[v]){
      assert(0<=nv && nv<N);
      if(visited[nv]) continue;
      visited[nv]=true;
      qe.push(nv);
    }
  }
  return false;
}
