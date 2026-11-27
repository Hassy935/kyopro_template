#include <Library/data_structure/dual_segment_tree.hpp>
#include <cstdlib>
#include <iostream>
#include <random>
#include <optional>

// NDEBUG が有効でも検証を省略しない．
void check(bool ok){
  if(!ok){
    cerr << "DualSegmentTree test failed" << endl;
    std::abort();
  }
}

namespace affine_test {
constexpr ll MOD = 998244353;
struct F { ll a,b; };
ll mapping(F f,ll x){ return (f.a*x+f.b)%MOD; }
F composition(F f,F g){ return {f.a*g.a%MOD,(f.a*g.b+f.b)%MOD}; }
F id(){ return {1,0}; }
using Seg = DualSegmentTree<ll,F,mapping,composition,id>;

void verify(const Seg& seg,const vector<ll>& a){
  check(seg.size()==ssize(a));
  for(ll p=0;p<ssize(a);p++) check(seg.get(p)==a[p]);
}

void boundary(){
  Seg empty;
  check(empty.size()==0);
  empty.apply(0,0,{2,1});
  Seg empty2(0,3), empty3(vector<ll>{});
  verify(empty2,{}); verify(empty3,{});
  Seg single(1);
  verify(single,{0});
  single.apply(0,1,{0,7});
  single.apply(0,{2,1});
  verify(single,{15});
  single.set(0,4);
  verify(single,{4});
  for(ll n : {2,3,4,5,7,8,9,15,16,17}){
    Seg seg(n,3);
    vector<ll> a(n,3);
    auto update = [&](ll l,ll r,F f){
      seg.apply(l,r,f);
      for(ll p=l;p<r;p++) a[p]=mapping(f,a[p]);
      verify(seg,a);
    };
    update(0,1,{2,3});
    update(0,n,{3,7});
    update(1,n,{0,11});
    update(0,n,{2,5});
    update(0,n-1,{3,1});
    update(n,n,{0,99});
    update(0,0,{0,99});
    seg.set(n-1,8); a[n-1]=8; verify(seg,a);
    seg.apply(0,{0,12}); a[0]=12; verify(seg,a);
    update(0,n,id());
  }
}

void exhaustive(){
  // 3 要素の全区間（空区間を含む）と 3 種類の作用を組み合わせる．
  const vector<F> fs={{0,4},{2,1},{1,3}};
  struct Op { ll l,r; F f; };
  vector<Op> ops;
  for(ll l=0;l<=3;l++) for(ll r=l;r<=3;r++)
    for(F f:fs) ops.push_back({l,r,f});
  ll count=0;
  for(auto x:ops) for(auto y:ops) for(auto z:ops){
    vector<ll> a={1,2,3};
    Seg seg(a);
    for(auto op:{x,y,z}){
      seg.apply(op.l,op.r,op.f);
      for(ll p=op.l;p<op.r;p++) a[p]=mapping(op.f,a[p]);
      verify(seg,a);
    }
    count++;
  }
  cout << "Exhaustive: " << count << " sequences passed\n";
}

void random(){
  mt19937_64 rng(20261128);
  ll count=0;
  for(ll n:{1,2,3,4,5,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129}){
    vector<ll> a(n);
    for(auto& x:a) x=rng()%MOD;
    Seg seg(a);
    for(ll q=0;q<10000;q++){
      ll p=rng()%n, l=rng()%(n+1), r=rng()%(n+1);
      if(l>r) swap(l,r);
      F f={ll(rng()%MOD),ll(rng()%MOD)};
      if(q%5==0) f.a=0;
      if(q%7==0) f=id();
      switch(rng()%5){
        case 0:
          seg.apply(l,r,f);
          for(ll i=l;i<r;i++) a[i]=mapping(f,a[i]);
          break;
        case 1: seg.apply(p,f); a[p]=mapping(f,a[p]); break;
        case 2: a[p]=rng()%MOD; seg.set(p,a[p]); break;
        case 3: check(seg.get(p)==a[p]); break;
        case 4:
          seg.apply(0,n,f);
          for(auto& x:a) x=mapping(f,x);
          break;
      }
      verify(seg,a);
      count++;
    }
  }
  cout << "Affine random: " << count << " operations passed\n";
}
}

namespace assignment_test {
using F=optional<ll>;
ll mapping(F f,ll x){ return f.value_or(x); }
F composition(F f,F g){ return f.has_value()?f:g; }
F id(){ return nullopt; }
void run(){
  dual_segtree<ll,F,mapping,composition,id> seg(5,0);
  seg.apply(0,5,7);
  seg.apply(1,4,LLONG_MAX);
  seg.apply(2,5,LLONG_MIN);
  seg.apply(0,5,nullopt);
  seg.set(3,42);
  vector<ll> a={7,LLONG_MAX,LLONG_MIN,42,LLONG_MIN};
  for(ll p=0;p<5;p++) check(seg.get(p)==a[p]);
}
}

namespace bool_test {
bool mapping(bool f,bool x){ return f!=x; }
bool composition(bool f,bool g){ return f!=g; }
bool id(){ return false; }
void run(){
  DualSegmentTree<bool,bool,mapping,composition,id> seg(5,false);
  seg.apply(0,5,true);
  seg.apply(1,4,true);
  seg.set(2,true);
  seg.apply(4,true);
  vector<bool> a={true,false,true,false,false};
  for(ll p=0;p<5;p++) check(seg.get(p)==a[p]);
}
}

int main(){
  affine_test::boundary();
  assignment_test::run();
  bool_test::run();
  cout << "Boundary, alias, assignment and bool tests passed\n";
  affine_test::exhaustive();
  affine_test::random();
  cout << "All tests passed.\n";
}
