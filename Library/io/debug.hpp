#pragma once
#include <Library/core/common_setting.hpp>
#include <Library/io/io.hpp>

#ifdef LOCAL

inline void debug_out(){
  cerr << '\n';
}

template<class Head, class... Tail>
void debug_out(Head h, Tail... t){
  cerr << h;
  if constexpr(sizeof...(t)) cerr << ", ";
  debug_out(t...);
}

#define dbg(...) cerr << "[" << #__VA_ARGS__ << "] = ", debug_out(__VA_ARGS__)

inline void debug(const string& S){
  cerr << S << '\n';
}

template<class T>
void debug(const vector<T>& A){
  forr(x,A){
    cerr << x << ' ';
  }
  cerr << '\n';
}

inline void debug(const vb& A){
  forr(x,A){
    cerr << (int)x << ' ';
  }
  cerr << '\n';
}

inline void debug(const vector<string>& A){
  forr(s,A){
    cerr << s << '\n';
  }
}

template<class T>
void debug(const vector<vector<T>>& A){
  forr(row,A){
    debug(row);
  }
}

template<class T>
void debug(const deque<T>& A){
  forr(c,A){
    cerr << c << ' ';
  }
  cerr << '\n';
}

template<class T>
void debug(const set<T>& st){
  forr(c,st){
    cerr << c << ' ';
  }
  cerr << '\n';
}

template<class T>
void debug(const multiset<T>& st){
  forr(c,st){
    cerr << c << ' ';
  }
  cerr << '\n';
}

template<class T>
void debug(const unordered_set<T>& st){
  forr(c,st){
    cerr << c << ' ';
  }
  cerr << '\n';
}

template<class T>
void debug(const unordered_multiset<T>& st){
  forr(c,st){
    cerr << c << ' ';
  }
  cerr << '\n';
}

template<class T,class U>
void debug(const map<T,U>& mp){
  for(auto [k,v]:mp){
    cerr << "(" << k << "," << v << ") ";
  }
  cerr << '\n';
}

template<class T,class U>
void debug(const unordered_map<T,U>& mp){
  for(auto [k,v]:mp){
    cerr << "(" << k << "," << v << ") ";
  }
  cerr << '\n';
}

#else

#define dbg(...) ((void)0)
#define debug(...) ((void)0)

#endif