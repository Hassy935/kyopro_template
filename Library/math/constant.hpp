#pragma once
#include <Library/core/common_setting.hpp>
#include <limits>

inline constexpr ld PI=numbers::pi_v<ld>;

template<class T>
inline constexpr T INFTY = std::numeric_limits<T>::max();

// 元のテンプレートと同様に書き換え可能．
inline array<ll,19> tens=[](){
  array<ll,19> a{};
  a[0]=1;
  for(ll i=1;i<19;i++) a[i]=a[i-1]*10;
  return a;
}();
