#pragma once
#include <Library/common_setting.hpp>

// 頂点番号は 0-indexed．s, g および隣接頂点は範囲内とする．
inline bool is_reachable(const vvl& G, ll s, ll g){
  ll N=ssize(G);
  assert(0<=s&&s<N&&0<=g&&g<N);
  queue<ll> qe;
  vb visited(N);
  visited[s]=true;
  qe.push(s);

  while(!qe.empty()){
    ll v=qe.front();
    qe.pop();
    if(v==g) return true;
    forr(nv,G[v]){
      if(visited[nv]) continue;
      visited[nv]=true;
      qe.push(nv);
    }
  }
  return false;
}

// 未到達の頂点への距離は -1．
inline vl get_dist(const vvl& G, ll s){
  ll N=ssize(G);
  assert(0<=s&&s<N);
  queue<ll> qe;
  vl dist(N,-1);
  dist[s]=0;
  qe.push(s);

  while(!qe.empty()){
    ll v=qe.front();
    qe.pop();
    forr(nv,G[v]){
      if(dist[nv]!=-1) continue;
      dist[nv]=dist[v]+1;
      qe.push(nv);
    }
  }
  return dist;
}
