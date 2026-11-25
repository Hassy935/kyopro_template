#pragma once
#include <Library/common_setting.hpp>

// 結果が空集合になる場合に注意する．
inline pair<ll,ll> cap(ll l, ll r, ll nl, ll nr){
  return {max(l,nl),min(r,nr)};
}
