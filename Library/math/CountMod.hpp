#pragma once

// 区間[L,R)にあるxmodMOD=remの個数を求める．
// 半開区間であることに注意する．
// 0<=rem<MODでなくても正しく動く．
ll count_mod(ll L, ll R, ll MOD, ll rem=0){
  assert(MOD>0);
  L-=rem;R-=rem;
  ll l=divc(L,MOD), r=divc(R,MOD);
  return r-l;
}