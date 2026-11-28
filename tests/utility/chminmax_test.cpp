#include <Library/utility/chminmax.hpp>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <utility>

namespace {

template<class T>
void check(const char* label,bool updated,const T& actual,bool expected_update,
           const std::type_identity_t<T>& expected){
  if(updated==expected_update && actual==expected) return;
  std::cerr << label << ": updated=" << updated << ", expected=" << expected_update
            << ", value matches=" << (actual==expected) << '\n';
  std::exit(1);
}

} // namespace

int main(){
  ll x=10;
  bool updated=chmin(x,7LL);
  check("ll and ll",updated,x,true,7LL);
  int value=4;
  updated=chmin(x,value);
  check("ll and int",updated,x,true,4LL);
  updated=chmax(x,8);
  check("ll and integer literal",updated,x,true,8LL);
  updated=chmin(x,9);
  check("unchanged minimum",updated,x,false,8LL);
  updated=chmax(x,7);
  check("unchanged maximum",updated,x,false,8LL);
  updated=chmin(x,8);
  check("equal minimum",updated,x,false,8LL);
  updated=chmax(x,8LL);
  check("equal maximum",updated,x,false,8LL);

  i128 large=i128(1)<<100;
  updated=chmin(large,-6LL);
  check("i128 and ll",updated,large,true,i128(-6));
  updated=chmax(large,7LL);
  check("i128 maximum",updated,large,true,i128(7));

  double real=1.5;
  updated=chmax(real,2);
  check("double and int",updated,real,true,2.0);
  updated=chmin<double>(real,0);
  check("explicit template argument",updated,real,true,0.0);
  x=1;
  updated=chmax(x,1.9);
  check("truncate before maximum",updated,x,false,1LL);
  updated=chmin(x,0.5);
  check("truncate and update minimum",updated,x,true,0LL);
  updated=chmin(x,-0.5);
  check("truncate before minimum",updated,x,false,0LL);
  updated=chmax<ll>(x,3);
  check("explicit ll",updated,x,true,3LL);
  updated=chmax(x,{4});
  check("braced scalar",updated,x,true,4LL);

  std::pair<int,ll> pair{2,7};
  updated=chmin(pair,{2,6});
  check("braced pair",updated,pair,true,{2,6});
  updated=chmax(pair,std::pair<int,ll>{2,6});
  check("equal pair",updated,pair,false,{2,6});
  std::tuple<int,ll,int> tuple{1,2,3};
  updated=chmax(tuple,{1,3,0});
  check("braced tuple",updated,tuple,true,{1,3,0});
  updated=chmin(tuple,std::tuple<int,ll,int>{1,1,9});
  check("same-type tuple",updated,tuple,true,{1,1,9});

  std::cout << "chminmax: all checks passed\n";
}
