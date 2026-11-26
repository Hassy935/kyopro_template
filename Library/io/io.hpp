#pragma once
#include <Library/core/common_setting.hpp>

template<class T>
istream& operator>>(istream& is, vector<T>& v){
  for(auto& x : v) is >> x;
  return is;
}

template<class T, class U>
ostream& operator<<(ostream& os, const pair<T,U>& p){
  return os << "(" << p.first << ", " << p.second << ")";
}

inline void yes(){cout << "Yes\n";}
inline void no(){cout << "No\n";}

inline void iofaster(){
  ios_base::sync_with_stdio(false);
  cin.tie(nullptr);
}

inline void coutld(int n){
  assert(n>=0);
  cout << fixed << setprecision(n);
  cerr << fixed << setprecision(n);
}

inline void coutvl(const vl& A){
  ll N=ssize(A);
  rep(i,N) cout << A[i] << (i<N-1?' ':'\n');
  if(A.empty()) cout << '\n';
}

inline void coutvl_v(const vl& A){
  ll N=ssize(A);
  rep(i,N) cout << A[i] << '\n';
  if(A.empty()) cout << '\n';
}

template<class V, class... Vs>
void cin_zip(V& a, Vs&... b){
  ll n=ssize(a);
  assert(((ssize(b)==n)&&...));

  rep(i,n){
    cin >> a[i];
    ((cin >> b[i]),...);
  }
}

inline void show(){
  cout << '\n';
}

template<class T, class... Ts>
void show(const T& x, const Ts&... xs){
  cout << x;
  ((cout << ' ' << xs), ...);
  cout << '\n';
}