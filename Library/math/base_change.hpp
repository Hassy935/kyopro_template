#pragma once
#include <Library/core/common_setting.hpp>

// S: a 進数表記の非負整数．各桁は 0 以上 a 未満．
// 2 <= a,b <= 10．O(|S|^2)．
inline string base_change(string S, int a, int b){
  assert(2<=a&&a<=10);
  assert(2<=b&&b<=10);
  assert(!S.empty());
  for(char c:S) assert('0'<=c&&c<'0'+a);

  if(S=="0") return "0";
  string T;
  while(!(S.size()==1&&S[0]=='0')){
    string next;
    int rem=0;
    for(char c:S){
      rem=rem*a+(c-'0');
      int q=rem/b;
      rem%=b;
      if(!next.empty()||q!=0) next.push_back(char('0'+q));
    }
    T.push_back(char('0'+rem));
    if(next.empty()) next="0";
    S=next;
  }
  reverse(nall(T));
  return T;
}
