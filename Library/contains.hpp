#pragma once
#include <Library/common_setting.hpp>

template<class T>
bool contains(const vector<T>& A, T c){
  return find(nall(A),c)!=A.end();
}

inline bool contains(const string& S, char c){
  return S.find(c)!=string::npos;
}