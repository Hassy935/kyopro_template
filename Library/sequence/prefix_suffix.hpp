#pragma once
#include <Library/core/common_setting.hpp>

template<class T>
vector<T> prefix_min(const vector<T>& a){
  vector<T> res=a;
  ll n=ssize(a);
  for(ll i=1;i<n;i++) res[i]=min(res[i-1],res[i]);
  return res;
}

template<class T>
vector<T> prefix_max(const vector<T>& a){
  vector<T> res=a;
  ll n=ssize(a);
  for(ll i=1;i<n;i++) res[i]=max(res[i-1],res[i]);
  return res;
}

template<class T>
vector<T> suffix_min(const vector<T>& a){
  vector<T> res=a;
  ll n=ssize(a);
  for(ll i=n-2;i>=0;i--) res[i]=min(res[i],res[i+1]);
  return res;
}

template<class T>
vector<T> suffix_max(const vector<T>& a){
  vector<T> res=a;
  ll n=ssize(a);
  for(ll i=n-2;i>=0;i--) res[i]=max(res[i],res[i+1]);
  return res;
}
