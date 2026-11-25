#pragma once
#include <Library/common_setting.hpp>

// 行・列の変化量を返す．
inline pair<ll,ll> dir_to_pair(char c){
  assert(c=='U'||c=='D'||c=='L'||c=='R');
  if(c=='U') return {-1,0};
  if(c=='D') return {1,0};
  if(c=='L') return {0,-1};
  if(c=='R') return {0,1};
  assert(false);
  return {0,0};
}
