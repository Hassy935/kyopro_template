#include <Library/graph/restore_path.hpp>

namespace {
constexpr ull seed=0xA3570AEULL;
static_assert(is_same_v<decltype(restore_path(declval<const vl&>(),0LL)),vl>);

void check(bool ok,const char* label,ll trial,ll t){
  if(ok) return;
  cerr << label << " seed=" << seed << " trial=" << trial << " target=" << t << '\n';
  exit(1);
}
}

int main(){
  check(restore_path(vl{-1},0).empty(),"unreachable",-1,0);
  check(restore_path(vl{0},0)==vl{0},"root",-1,0);
  const vl forest={3,1,-1,3,0};
  check(restore_path(forest,4)==vl({3,0,4}),"path order",-1,4);
  check(restore_path(forest,1)==vl{1},"second root",-1,1);
  check(restore_path(forest,2).empty(),"forest unreachable",-1,2);

  mt19937_64 rng(seed);
  for(ll trial=0;trial<200;trial++){
    const ll n=1+ll(rng()%100);
    vl order(n),par(n,-1),reachable;
    vvl expected(n);
    iota(order.begin(),order.end(),0LL);
    shuffle(order.begin(),order.end(),rng);
    for(ll v:order){
      const ull action=rng()%4;
      if(action==0) continue;
      if(action==1 || reachable.empty()) par[v]=v;
      else{
        par[v]=reachable[rng()%reachable.size()];
        expected[v]=expected[par[v]];
      }
      expected[v].push_back(v);
      reachable.push_back(v);
    }
    for(ll t=0;t<n;t++) check(restore_path(par,t)==expected[t],"random forest",trial,t);
  }

  constexpr ll n=200'000;
  vl par(n);
  iota(par.begin(),par.end(),1LL);
  par.back()=n-1;
  const vl path=restore_path(par,0);
  check(ssize(path)==n,"long path size",-2,0);
  for(ll i=0;i<n;i++) check(path[i]==n-1-i,"long path order",-2,i);
  cout << "restore_path tests passed.\n";
}
