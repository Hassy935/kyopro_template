#pragma once
#include <Library/common_setting.hpp>

inline void adlist(const vvl& G){
#ifdef LOCAL
  ll N=ssize(G);
  cerr << N << '\n';
  rep(v,N){
    cerr << v << ' ' << ssize(G[v]) << ' ';
    forr(nv,G[v]){
      cerr << nv << ' ';
    }
    cerr << '\n';
  }
#else
  (void)G;
#endif
}
