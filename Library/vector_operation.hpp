#pragma once
#include <Library/common_setting.hpp>

inline ll sumvl(const vl& A){
  return accumulate(nall(A),0LL);
}

inline ll maxvl(const vl& A){
  assert(!A.empty());
  return *max_element(nall(A));
}

inline ll minvl(const vl& A){
  assert(!A.empty());
  return *min_element(nall(A));
}

// vectorがsort済みであることが必要．
template<class T>
void unique_erase(vector<T>& A){
  A.erase(unique(nall(A)),A.end());
}

// vl
inline vl& operator+=(vl& A, ll x){
  forr(a,A) a+=x;
  return A;
}

inline vl& operator-=(vl& A, ll x){
  forr(a,A) a-=x;
  return A;
}

inline vl& operator++(vl& A){
  return A+=1;
}

inline void operator++(vl& A, int){
  A+=1;
}

inline vl& operator--(vl& A){
  return A-=1;
}

inline void operator--(vl& A, int){
  A-=1;
}


// vvl
inline vvl& operator+=(vvl& A, ll x){
  forr(row,A) row+=x;
  return A;
}

inline vvl& operator-=(vvl& A, ll x){
  forr(row,A) row-=x;
  return A;
}

inline vvl& operator++(vvl& A){
  return A+=1;
}

inline void operator++(vvl& A, int){
  A+=1;
}

inline vvl& operator--(vvl& A){
  return A-=1;
}

inline void operator--(vvl& A, int){
  A-=1;
}

// 正方行列を in-place で転置する．
template<class Row>
void transpose(vc<Row>& A){
  ll N=ssize(A);

  forr(row,A) assert(ssize(row)==N);

  rep(i,N)forn(j,i+1,N){
    iter_swap(A[i].begin()+j,A[j].begin()+i);
  }
}