#pragma once
#include <Library/common_setting.hpp>

inline ll sq(ll x){
  return x*x;
}

inline ll divf(ll N, ll d){
  assert(d!=0);

  if(((N<0&&d>0)||(N>0&&d<0))&&N%d!=0)
    return N/d-1;

  return N/d;
}

inline ll divc(ll N, ll d){
  assert(d!=0);

  if(((N>0&&d>0)||(N<0&&d<0))&&N%d!=0)
    return N/d+1;

  return N/d;
}

inline ll mod_pos(ll N, ll d){
  assert(d>0);

  ll r=N%d;
  if(r<0) r+=d;
  return r;
}

inline ll floor_sqrt(ll n){
  assert(n>=0);

  ll x=sqrtl((long double)n);

  while((i128)(x+1)*(x+1)<=n) x++;
  while((i128)x*x>n) x--;

  return x;
}

inline ll ceil_sqrt(ll n){
  assert(n>=0);

  ll x=floor_sqrt(n);
  if((i128)x*x<n) x++;

  return x;
}

inline bool is_sq(ll n){
  ll x=floor_sqrt(n);
  return (i128)x*x==n;
}

inline ll nC2(ll x){
  assert(0<=x&&x<=4294967296LL);

  if(x%2==0) return x/2*(x-1);
  else return (x-1)/2*x;
}