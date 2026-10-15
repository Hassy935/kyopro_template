#pragma once

// rootを根とした各頂点の部分木サイズを返す．
// Gは無向木の隣接リスト．
// 時間計算量 O(N)，空間計算量 O(N)．
vl subtree_size(const vvl& G, ll root=0){
  ll N=ssize(G);
  if(N==0) return {};
  assert(0<=root&&root<N);

  vl par(N,-1),order;
  order.reserve(N);

  par[root]=root;
  order.pb(root);

  // 親と走査順序を求める．
  for(ll i=0;i<ssize(order);i++){
    ll v=order[i];
    forr(nv,G[v]){
      if(par[nv]!=-1) continue;
      par[nv]=v;
      order.pb(nv);
    }
  }

  // 逆順に部分木サイズを計算する．
  vl sz(N,1);
  for(ll i=N-1;i>0;i--){
    ll v=order[i];
    sz[par[v]]+=sz[v];
  }

  return sz;
}