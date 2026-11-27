#include <Library/data_structure/dual_segment_tree_preset.hpp>
#include <cstdlib>
#include <iostream>
#include <random>
#include <type_traits>

namespace preset_test {
constexpr ll LO=std::numeric_limits<ll>::lowest();
constexpr ll HI=std::numeric_limits<ll>::max();
constexpr unsigned long long SEED=20261128;

void check(bool ok,const char* name,ll n=0,ll q=-1,ll p=-1){
  if(ok) return;
  std::cerr << "FAIL: " << name << " seed=" << SEED
            << " n=" << n << " q=" << q << " p=" << p << '\n';
  std::abort();
}

static_assert(std::is_same_v<dual_segtree_add<>,DualSegmentTreeAdd<ll>>);
static_assert(std::is_same_v<dual_segtree_update<>,DualSegmentTreeUpdate<ll>>);
static_assert(std::is_same_v<dual_segtree_chmin<>,DualSegmentTreeChmin<ll>>);
static_assert(std::is_same_v<dual_segtree_chmax<>,DualSegmentTreeChmax<ll>>);

template<class Seg>
void verify(const Seg& seg,const std::vector<ll>& a,const char* name,ll q=-1){
  ll n=std::ssize(a);
  check(seg.size()==n,name,n,q);
  for(ll p=0;p<n;p++) check(seg.get(p)==a[p],name,n,q,p);
}

template<class Seg,class Oracle>
void run(const char* name,Oracle oracle,bool extreme){
  // 空の構築と空区間，初期値付き構築，一点操作を確認する．
  Seg empty,empty2(0),empty3(std::vector<ll>{});
  for(auto* s:{&empty,&empty2,&empty3}){
    s->apply(0,0,3);
    verify(*s,{},name);
  }
  Seg single(1),filled(5,-3);
  verify(single,{0},name);
  verify(filled,{-3,-3,-3,-3,-3},name);
  single.apply(0,1,5);
  single.apply(0,-2);
  verify(single,{oracle(-2,oracle(5,0))},name);
  single.set(0,9);
  verify(single,{9},name);

  if(extreme){
    std::vector<ll> a={LO,0,HI,1,-1};
    Seg seg(a);
    auto update=[&](ll l,ll r,ll f){
      seg.apply(l,r,f);
      for(ll p=l;p<r;p++) a[p]=oracle(f,a[p]);
      verify(seg,a,name);
    };
    update(0,5,HI); update(1,4,LO); update(0,5,0);
    update(2,5,HI); update(0,4,LO); update(0,5,HI);
    seg.set(2,LO); a[2]=LO; verify(seg,a,name);
    seg.apply(2,HI); a[2]=oracle(HI,a[2]); verify(seg,a,name);
    seg.set(1,HI); a[1]=HI;
    seg.apply(1,LO); a[1]=oracle(LO,a[1]); verify(seg,a,name);
  }

  // 3 要素の全区間と 3 種類の値からなる長さ 3 の操作列を全探索する．
  struct Op{ ll l,r,f; };
  std::vector<Op> ops;
  for(ll l=0;l<=3;l++) for(ll r=l;r<=3;r++)
    for(ll f:{-3,0,4}) ops.push_back({l,r,f});
  ll sequences=0;
  for(auto u:ops) for(auto v:ops) for(auto w:ops){
    std::vector<ll> a={-1,0,2};
    Seg seg(a);
    for(auto op:{u,v,w}){
      seg.apply(op.l,op.r,op.f);
      for(ll p=op.l;p<op.r;p++) a[p]=oracle(op.f,a[p]);
      verify(seg,a,name,sequences);
    }
    sequences++;
  }

  // 固定シードで区間作用・一点作用・一点代入・取得を混在させる．
  std::mt19937_64 rng(SEED);
  ll operations=0;
  for(ll n:{1,2,3,4,5,7,8,9,15,16,17,31,32,33,63,64,65,127,128,129}){
    std::vector<ll> a(n);
    for(auto& x:a) x=ll(rng()%201)-100;
    Seg seg(a);
    for(ll q=0;q<5000;q++){
      ll l=rng()%(n+1),r=rng()%(n+1),p=rng()%n;
      if(l>r) std::swap(l,r);
      ll f=ll(rng()%201)-100;
      if(extreme && q%17==0) f=HI;
      if(extreme && q%19==0) f=LO;
      switch(rng()%5){
        case 0:
          seg.apply(l,r,f);
          for(ll i=l;i<r;i++) a[i]=oracle(f,a[i]);
          break;
        case 1: seg.apply(p,f); a[p]=oracle(f,a[p]); break;
        case 2: seg.set(p,f); a[p]=f; break;
        case 3: check(seg.get(p)==a[p],name,n,q,p); break;
        case 4:
          seg.apply(0,n,f);
          for(auto& x:a) x=oracle(f,x);
          break;
      }
      verify(seg,a,name,q);
      operations++;
    }
  }
  std::cout << name << ": " << sequences << " sequences, "
            << operations << " random operations passed\n";
}

void other_types(){
  dual_segtree_add<int> a(3,2);
  a.apply(0,3,-5); check(a.get(2)==-3,"int add");
  dual_segtree_update<bool> b(3,false);
  b.apply(0,3,true); b.apply(1,false);
  b.apply(0,3,std::nullopt);
  check(b.get(0) && !b.get(1) && b.get(2),"bool update");
  dual_segtree_chmin<unsigned> c(3,7);
  c.apply(0,3,0); check(c.get(1)==0,"unsigned chmin");
  dual_segtree_chmax<unsigned> d(3,0);
  d.apply(0,3,std::numeric_limits<unsigned>::max());
  check(d.get(0)==std::numeric_limits<unsigned>::max(),"unsigned chmax");
  const double inf=std::numeric_limits<double>::infinity();
  DualSegmentTreeChmin<double> e(3,inf);
  DualSegmentTreeChmax<double> f(3,-inf);
  check(e.get(0)==inf && f.get(0)==-inf,"floating identity");
  e.apply(0,3,-inf); f.apply(0,3,inf);
  check(e.get(1)==-inf && f.get(1)==inf,"floating infinity update");
}
}

int main(){
  using namespace preset_test;
  run<DualSegmentTreeAdd<>>("Add",[](ll f,ll x){ return x+f; },false);
  run<DualSegmentTreeUpdate<>>("Update",[](ll f,ll){ return f; },true);
  run<DualSegmentTreeChmin<>>("Chmin",[](ll f,ll x){ return x<f?x:f; },true);
  run<DualSegmentTreeChmax<>>("Chmax",[](ll f,ll x){ return x>f?x:f; },true);
  other_types();
  std::cout << "All preset tests passed.\n";
}
