#include <Library/data_structure/interval_set.hpp>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <limits>
#include <optional>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

using P=pair<ll,ll>;
constexpr ull seed=935;
const char* phase="exhaustive";
ll step=0,op=-1,left_endpoint=0,right_endpoint=0;

[[noreturn]] void fail(const char* condition){
  cerr << "FAIL: " << phase << " step=" << step << " seed=" << seed
       << " condition: " << condition;
  if(op>=0) cerr << " op=" << op << " l=" << left_endpoint << " r=" << right_endpoint;
  cerr << '\n';
  exit(1);
}

void check(bool actual,const char* condition){
  if(!actual) fail(condition);
}

template<class A,class B>
void check_equal(const A& actual,const B& expected,const char* condition){
  if(actual==expected) return;
  cerr << "actual=" << actual << " expected=" << expected << '\n';
  fail(condition);
}

#define INTERVAL_CHECK(...) check((__VA_ARGS__),#__VA_ARGS__)
#define INTERVAL_EQ(actual, expected) check_equal((actual),(expected),#actual " == " #expected)

void verify(const IntervalSet& st,const set<ll>& ref){
  INTERVAL_EQ(st.size(),ssize(ref));
  INTERVAL_EQ(st.count(),ssize(ref));
  INTERVAL_EQ(st.empty(),ref.empty());
  if(!ref.empty()){
    INTERVAL_EQ(st.min(),*ref.begin());
    INTERVAL_EQ(st.max(),*ref.rbegin());
  }
  vector<P> intervals;
  for(ll x:ref){
    if(!intervals.empty() && intervals.back().second==x) intervals.back().second++;
    else intervals.emplace_back(x,x+1);
  }
  INTERVAL_EQ(st.interval_count(),ssize(intervals));
  INTERVAL_CHECK(vector<P>(st.begin(),st.end())==intervals);
  for(ll x=-11;x<=11;x++){
    optional<P> p;
    for(auto [l,r]:intervals) if(l<=x && x<r) p=P{l,r};
    INTERVAL_CHECK(st.get(x)==p);
    INTERVAL_EQ(st.contains(x),ref.contains(x));
    ll m=x;
    while(ref.contains(m)) m++;
    INTERVAL_EQ(st.mex(x),m);
    auto lb=ref.lower_bound(x),ub=ref.upper_bound(x);
    auto val=[&](auto it)->optional<ll>{
      if(it==ref.end()) return nullopt;
      return *it;
    };
    INTERVAL_CHECK(st.ge(x)==val(lb));
    INTERVAL_CHECK(st.gt(x)==val(ub));
    INTERVAL_CHECK(st.lt(x)==(lb==ref.begin()?nullopt:val(prev(lb))));
    INTERVAL_CHECK(st.le(x)==(ub==ref.begin()?nullopt:val(prev(ub))));
  }
}

void operation(IntervalSet& st,set<ll>& ref,ll action,ll l,ll r){
  ++step;
  op=action;
  left_endpoint=l;
  right_endpoint=r;
  bool all=true,any=false;
  for(ll x=l;x<r;x++){
    all&=ref.contains(x);
    any|=ref.contains(x);
  }
  INTERVAL_EQ(st.contains(l,r),all);
  INTERVAL_EQ(st.intersects(l,r),any);
  if(action==0){
    st.add(l,r);
    for(ll x=l;x<r;x++) ref.insert(x);
  }else if(action==1){
    INTERVAL_EQ(st.erase(l,r),any);
    for(ll x=l;x<r;x++) ref.erase(x);
  }else if(action==2){
    INTERVAL_EQ(st.remove(l,r),all);
    if(all) for(ll x=l;x<r;x++) ref.erase(x);
  }else if(action==3){
    st.add(l);
    ref.insert(l);
  }else if(action==4){
    INTERVAL_EQ(st.erase(l),ref.erase(l)!=0);
  }else if(action==5){
    st.clear();
    ref.clear();
  }
}

} // namespace

int main(){
  // 全256状態 × 全45区間 × 3更新操作．
  for(ll mask=0;mask<256;mask++){
    IntervalSet base;
    set<ll> model;
    for(ll i=0;i<8;i++) if(mask>>i&1){
      base.add(i-4);
      model.insert(i-4);
    }
    for(ll l=-4;l<=4;l++) for(ll r=l;r<=4;r++) for(ll op=0;op<3;op++){
      auto st=base;
      auto ref=model;
      operation(st,ref,op,l,r);
      verify(st,ref);
    }
  }
  cout << "Exhaustive: 34560 cases passed\n";

  phase="random";
  step=0;
  mt19937_64 rng(seed);
  for(ll t=0;t<100;t++){
    interval_set st;
    set<ll> ref;
    for(ll q=0;q<1000;q++){
      ll l=(ll)(rng()%21)-10,r=(ll)(rng()%21)-10;
      if(l>r) swap(l,r);
      ll op=rng()%100;
      op=op<90?op%5:(op==99?5:6);
      operation(st,ref,op,l,r);
      verify(st,ref);
    }
  }
  cout << "Random: 100000 operations passed\n";

  phase="boundary and copy/move";
  step=0;
  op=-1;
  constexpr ll lo=numeric_limits<ll>::min(),hi=numeric_limits<ll>::max();
  interval_set st;
  INTERVAL_CHECK(!st.lt(lo) && !st.gt(hi));
  st.add(lo);
  st.add(hi-1);
  INTERVAL_CHECK(st.size()==2 && st.interval_count()==2);
  INTERVAL_CHECK(st.min()==lo && st.max()==hi-1);
  INTERVAL_CHECK(st.mex(lo)==lo+1 && st.mex(hi-1)==hi);
  INTERVAL_CHECK(st.ge(lo)==lo && st.gt(lo)==hi-1);
  INTERVAL_CHECK(st.le(hi)==hi-1 && st.lt(hi)==hi-1);
  INTERVAL_CHECK(!st.ge(hi) && !st.lt(lo) && !st.gt(hi));
  INTERVAL_CHECK(!st.erase(hi) && !st.contains(hi));
  bool threw=false;
  try{ st.add(hi); }catch(const out_of_range&){ threw=true; }
  INTERVAL_CHECK(threw && st.size()==2);
  st.add(lo,hi);
  INTERVAL_CHECK(st.interval_count()==1 && st.contains(lo,hi));
  INTERVAL_CHECK(st.mex(lo)==hi && st.le(hi)==hi-1);
  threw=false;
  try{ (void)st.size(); }catch(const overflow_error&){ threw=true; }
  INTERVAL_CHECK(threw);
  threw=false;
  try{ (void)st.count(); }catch(const overflow_error&){ threw=true; }
  INTERVAL_CHECK(threw);
  INTERVAL_CHECK(st.erase(lo,0));
  INTERVAL_EQ(st.size(),hi);
  INTERVAL_EQ(st.min(),0);
  INTERVAL_CHECK(st.remove(1,hi-1));
  INTERVAL_CHECK(st.size()==2 && st.interval_count()==2);
  INTERVAL_CHECK(st.contains(hi,hi) && st.remove(hi,hi) && !st.intersects(hi,hi));
  INTERVAL_CHECK(!st.erase(hi,hi));
  auto copy=st;
  interval_set moved=std::move(st);
  INTERVAL_CHECK(st.empty() && st.size()==0 && moved.size()==2);
  st.add(-1);
  st=std::move(moved);
  INTERVAL_CHECK(moved.empty() && moved.size()==0 && st.size()==2);
  INTERVAL_CHECK(vector<P>(st.begin(),st.end())==vector<P>(copy.begin(),copy.end()));
  cout << "Boundary and copy/move tests passed\n";
  cout << "All tests passed.\n";
}
