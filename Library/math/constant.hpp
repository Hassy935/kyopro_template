#pragma once
#include <Library/core/common_setting.hpp>

inline constexpr ld PI=numbers::pi_v<ld>;

// 元のテンプレートと同様に書き換え可能．
inline array<ll,19> tens=[](){
  array<ll,19> a{};
  a[0]=1;
  for(ll i=1;i<19;i++) a[i]=a[i-1]*10;
  return a;
}();
