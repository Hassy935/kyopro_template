#include <Library/template.hpp>

using P=pair<ll,ll>;

void verify(const IntervalSet& st,const set<ll>& ref){
  assert(st.size()==ssize(ref));
  assert(st.count()==ssize(ref));
  assert(st.empty()==ref.empty());
  if(!ref.empty()){
    assert(st.min()==*ref.begin());
    assert(st.max()==*ref.rbegin());
  }
  vector<P> intervals;
  for(ll x:ref){
    if(!intervals.empty() && intervals.back().second==x) intervals.back().second++;
    else intervals.emplace_back(x,x+1);
  }
  assert(st.interval_count()==ssize(intervals));
  assert(vector<P>(st.begin(),st.end())==intervals);
  for(ll x=-11;x<=11;x++){
    optional<P> p;
    for(auto [l,r]:intervals) if(l<=x && x<r) p=P{l,r};
    assert(st.get(x)==p);
    assert(st.contains(x)==ref.contains(x));
    ll m=x;
    while(ref.contains(m)) m++;
    assert(st.mex(x)==m);
    auto lb=ref.lower_bound(x),ub=ref.upper_bound(x);
    auto val=[&](auto it)->optional<ll>{
      if(it==ref.end()) return nullopt;
      return *it;
    };
    assert(st.ge(x)==val(lb));
    assert(st.gt(x)==val(ub));
    assert(st.lt(x)==(lb==ref.begin()?nullopt:val(prev(lb))));
    assert(st.le(x)==(ub==ref.begin()?nullopt:val(prev(ub))));
  }
}

void operation(IntervalSet& st,set<ll>& ref,ll op,ll l,ll r){
  bool all=true,any=false;
  for(ll x=l;x<r;x++){
    all&=ref.contains(x);
    any|=ref.contains(x);
  }
  assert(st.contains(l,r)==all);
  assert(st.intersects(l,r)==any);
  if(op==0){
    st.add(l,r);
    for(ll x=l;x<r;x++) ref.insert(x);
  }else if(op==1){
    assert(st.erase(l,r)==any);
    for(ll x=l;x<r;x++) ref.erase(x);
  }else if(op==2){
    assert(st.remove(l,r)==all);
    if(all) for(ll x=l;x<r;x++) ref.erase(x);
  }else if(op==3){
    st.add(l);
    ref.insert(l);
  }else if(op==4){
    assert(st.erase(l)==(ref.erase(l)!=0));
  }else if(op==5){
    st.clear();
    ref.clear();
  }
}

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

  mt19937_64 rng(935);
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

  constexpr ll lo=numeric_limits<ll>::min(),hi=numeric_limits<ll>::max();
  interval_set st;
  assert(!st.lt(lo) && !st.gt(hi));
  st.add(lo);
  st.add(hi-1);
  assert(st.size()==2 && st.interval_count()==2);
  assert(st.min()==lo && st.max()==hi-1);
  assert(st.mex(lo)==lo+1 && st.mex(hi-1)==hi);
  assert(st.ge(lo)==lo && st.gt(lo)==hi-1);
  assert(st.le(hi)==hi-1 && st.lt(hi)==hi-1);
  assert(!st.ge(hi) && !st.lt(lo) && !st.gt(hi));
  assert(!st.erase(hi) && !st.contains(hi));
  bool threw=false;
  try{ st.add(hi); }catch(const out_of_range&){ threw=true; }
  assert(threw && st.size()==2);
  st.add(lo,hi);
  assert(st.interval_count()==1 && st.contains(lo,hi));
  assert(st.mex(lo)==hi && st.le(hi)==hi-1);
  threw=false;
  try{ (void)st.size(); }catch(const overflow_error&){ threw=true; }
  assert(threw);
  threw=false;
  try{ (void)st.count(); }catch(const overflow_error&){ threw=true; }
  assert(threw);
  assert(st.erase(lo,0));
  assert(st.size()==hi && st.min()==0);
  assert(st.remove(1,hi-1));
  assert(st.size()==2 && st.interval_count()==2);
  assert(st.contains(hi,hi) && st.remove(hi,hi) && !st.intersects(hi,hi));
  assert(!st.erase(hi,hi));
  auto copy=st;
  interval_set moved=std::move(st);
  assert(st.empty() && st.size()==0 && moved.size()==2);
  st.add(-1);
  st=std::move(moved);
  assert(moved.empty() && moved.size()==0 && st.size()==2);
  assert(vector<P>(st.begin(),st.end())==vector<P>(copy.begin(),copy.end()));
  cout << "Boundary and copy/move tests passed\n";
  cout << "All tests passed.\n";
}
