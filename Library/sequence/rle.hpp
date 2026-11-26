#pragma once
#include <Library/core/common_setting.hpp>

template<class T>
vector<pair<T,ll>> rle(const vector<T>& A){
  ll N=ssize(A);
  vector<pair<T,ll>> ret;

  rep(i,N){
    if(i==0 || A[i]!=ret.back().first){
      ret.eb(A[i],1);
    }else{
      ret.back().second++;
    }
  }

  return ret;
}

inline vector<pair<char,ll>> rle(const string& S){
  ll N=ssize(S);
  vector<pair<char,ll>> ret;

  rep(i,N){
    if(i==0 || S[i]!=ret.back().first){
      ret.eb(S[i],1);
    }else{
      ret.back().second++;
    }
  }

  return ret;
}