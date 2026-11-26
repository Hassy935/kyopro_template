#pragma once
#include <Library/core/common_setting.hpp>

template<class T>
bool chmin(T& a, T b){
  if(a>b){
    a=b;
    return true;
  }
  return false;
}

template<class T>
bool chmax(T& a, T b){
  if(a<b){
    a=b;
    return true;
  }
  return false;
}