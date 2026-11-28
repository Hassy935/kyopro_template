#include <Library/data_structure/union_find.hpp>
#include <Library/core/common_setting.hpp>
#include <Library/old/data_structure/UnionFind.hpp>
#include <Library/data_structure/potential_union_find.hpp>
#include <Library/io/debug.hpp>
#include <atcoder/dsu>

namespace {
constexpr ull seed=0x554e494f4e46494e;
mt19937_64 rng(seed);
ll trial=0, step=0;

void check(bool ok,const char* message){
  if(ok) return;
  cerr<<message<<" seed="<<seed<<" trial="<<trial<<" step="<<step<<'\n';
  exit(1);
}

static_assert(is_same_v<UnionFind,union_find>);
static_assert(is_same_v<UnionFind,dsu>);
static_assert(is_same_v<decltype(declval<UnionFind&>().merge(0,0)),bool>);
static_assert(is_same_v<decltype(declval<const UnionFind&>().groups()),vvl>);
static_assert(is_nothrow_move_constructible_v<UnionFind> && is_nothrow_move_assignable_v<UnionFind>);
static_assert(!is_convertible_v<ll,UnionFind>);

struct Naive {
  vl labels;
  ll count;
  explicit Naive(ll n): labels(n),count(n){ iota(labels.begin(),labels.end(),0LL); }
  ll leader(ll x) const {return labels[x];}
  ll size(ll x) const {return std::count(labels.begin(),labels.end(),labels[x]);}
  bool merge(ll x,ll y){
    ll a=labels[x],b=labels[y];
    if(a==b) return false;
    if(size(x)<size(y)) swap(a,b);
    for(ll& label:labels) if(label==b) label=a;
    --count;
    return true;
  }
  vvl groups() const {
    vvl v(labels.size());
    for(ll i=0;i<ssize(labels);++i) v[labels[i]].push_back(i);
    erase_if(v,[](const vl& a){return a.empty();});
    return v;
  }
};

void verify(UnionFind& a,const Naive& expected){
  const auto groups=as_const(a).groups();
  check(a.size()==ssize(expected.labels) && a.comp_cnt()==expected.count &&
        ssize(groups)==a.comp_cnt() && groups==expected.groups(),"groups/order/count");
  for(ll x=0;x<a.size();++x)
    check(a.size(x)==expected.size(x) && a.leader(x)==expected.leader(x),"point state");
}

void expect_empty(const UnionFind& a){
  check(a.empty() && a.size()==0 && a.comp_cnt()==0 && a.groups().empty(),"empty state");
}

void special(){
  UnionFind empty,zero(0),one(1);
  expect_empty(empty); expect_empty(zero);
  check(!one.empty() && !one.merge(0,0) && one.same(0,0) &&
        one.leader(0)==0 && one.size(0)==1 && one.comp_cnt()==1,"single vertex");
  union_find a(6);
  check(a.merge(0,1) && !a.merge(1,0) && a.leader(1)==0,"root zero");
  check(a.merge(5,4) && a.merge(4,1) && a.leader(0)==5,"tie follows first component");
  check(a.merge(2,0) && a.leader(2)==5 && a.groups()==vvl{{3},{0,1,2,4,5}},"size ordering");
  auto copy=a;
  copy=static_cast<const UnionFind&>(copy);
  check(copy.groups()==a.groups(),"self copy");
  check(copy.merge(3,5) && copy.comp_cnt()==1 && a.comp_cnt()==2,"copy independence");
  UnionFind assigned;
  assigned=copy;
  check(assigned.groups()==copy.groups(),"copy assignment");
  UnionFind moved(std::move(copy));
  expect_empty(copy);
  check(moved.groups()==assigned.groups(),"move construction");
  a=std::move(moved);
  expect_empty(moved);
  check(a.comp_cnt()==1 && a.size(0)==6,"move assignment");
  auto& alias=a;
  a=std::move(alias);
  check(a.comp_cnt()==1 && a.size(0)==6,"self move");
  moved=UnionFind(3);
  check(moved.comp_cnt()==3 && moved.size()==3,"reassign moved source");
}

void exhaustive(){
  constexpr ll n=4;
  for(trial=0;trial<4096;++trial){
    UnionFind a(n);
    Naive b(n);
    ll code=trial;
    for(step=0;step<3;++step){
      ll x=code%4,y=code/4%4;
      code/=16;
      check(a.merge(x,y)==b.merge(x,y),"exhaustive merge");
      verify(a,b);
    }
  }
}

void random_sequences(){
  for(ll n:{1,2,7,33,63}){
    ::dsu a(n);
    UnionFind without_groups(n);
    DSU old(n);
    atcoder::dsu acl(static_cast<int>(n));
    p_dsu<ll> potential(n);
    Naive expected(n);
    for(step=0;step<1000;++step){
      ll x=ll(rng()%n),y=ll(rng()%n);
      if(step%3!=0){
        bool should_merge=expected.merge(x,y);
        check(a.merge(x,y)==should_merge && without_groups.merge(x,y)==should_merge &&
              old.merge(x,y)==should_merge && potential.merge(x,y,0)==should_merge,"merge comparison");
        acl.merge(int(x),int(y));
      }else{
        check(a.same(x,y)==(expected.leader(x)==expected.leader(y)) &&
              a.leader(x)==acl.leader(int(x)) && a.leader(y)==potential.leader(y) &&
              a.size(x)==acl.size(int(x)) && a.size(y)==old.size(y),"query comparison");
      }
      if(step%11==0) check(as_const(a).groups()==expected.groups(),"const groups during trace");
      check(a.comp_cnt()==expected.count && a.comp_cnt()==without_groups.comp_cnt(),"component count");
    }
    verify(a,expected);
    check(a.groups()==without_groups.groups() && a.groups()==old.groups() &&
          a.groups()==potential.groups(),"post-snapshot updates and reference groups");
  }
}

void large(){
  constexpr ll n=1<<17;
  UnionFind a(n);
  for(ll stride=1;stride<n;stride*=2)
    for(ll x=0;x<n;x+=2*stride) check(a.merge(x+stride,x),"balanced merge");
  const auto result=as_const(a).groups();
  check(result.size()==1 && ssize(result.front())==n,"large group length");
  for(ll x=0;x<n;++x){
    check(result.front()[x]==x && a.leader(x)==n-1 && a.size(x)==n,"large leader and order");
    check(!a.merge(x,n-1) && a.same(x,0),"compressed repeated query");
  }
  check(a.comp_cnt()==1,"large count");
}

void debugging(){
  UnionFind a(4);
  a.merge(1,0); a.merge(3,2);
  [[maybe_unused]] const auto& t=a;
  const UnionFind empty;
  ostringstream capture;
  auto* old=cerr.rdbuf(capture.rdbuf());
  int hits=0;
  debug(t); debug(empty);
  dbg(t,empty,vector<UnionFind>{t},pair{t,tuple{empty}},++hits);
  cerr.rdbuf(old);
#ifdef LOCAL
  check(t.debug_view()==vvl{{0,1},{2,3}},"debug view");
  check(hits==1 && capture.str()==
      "0 1 \n2 3 \n[t,empty,vector<UnionFind>{t},pair{t,tuple{empty}},++hits] = "
      "[[0, 1], [2, 3]], [], [[[0, 1], [2, 3]]], ([[0, 1], [2, 3]], ([])), 1\n","debug formatting");
#else
  check(hits==0 && capture.str().empty(),"non-LOCAL");
#endif
}

bool invalid(string_view mode){
  if(mode=="negative") (void)UnionFind(-1);
  else if(mode=="overflow") (void)UnionFind(numeric_limits<ll>::max());
#ifndef NDEBUG
  else if(mode=="index") (void)UnionFind(1).leader(1);
#endif
  else return false;
  return true;
}
}

int main(int argc,char** argv){
  if(argc==2) return invalid(argv[1]) ? 0 : 2;
  special(); exhaustive(); random_sequences(); large(); debugging();
  cout<<"union_find tests passed\n";
}
