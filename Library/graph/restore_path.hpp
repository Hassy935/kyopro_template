#pragma once

#include <Library/core/common_setting.hpp>

// par は始点の親が自分自身，未到達頂点の親が -1 の正しい親配列とする．
inline vl restore_path(const vl& par, ll t){
  assert(0<=t && t<ssize(par));
  if(par[t]==-1) return {};

  vl path;
  for(ll v=t;;v=par[v]){
    path.push_back(v);
    if(par[v]==v) break;
  }
  reverse(path.begin(),path.end());
  return path;
}
