#pragma once
#include <Library/core/common_setting.hpp>

inline ll popcountll(ll a){
  assert(a>=0);
  return popcount((ull)a);
}

inline ll floor_log2(ll x){
  assert(x>0);
  return bit_width((ull)x)-1;
}

inline ll ceil_log2(ll x){
  assert(x>0);
  return bit_width((ull)(x-1));
}