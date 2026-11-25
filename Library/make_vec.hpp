#pragma once
#include <Library/common_setting.hpp>

template<class T>
auto make_vec(size_t n, const T& x){
  return vector<T>(n,x);
}

template<class T, class... Args>
requires (sizeof...(Args)>=1)
auto make_vec(size_t n, size_t m, Args... args){
  return vector(n,make_vec<T>(m,args...));
}