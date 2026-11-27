#include <Library/graph/bfs.hpp>
#include <Library/graph/restore_path.hpp>
#include <Library/graph/bfs01.hpp>
#include <Library/graph/dijkstra.hpp>

namespace {
using Graph = vvl;
constexpr ull seed=0xBF5ULL;
constexpr i128 inf=i128(1)<<120;

static_assert(is_same_v<decltype(bfs(declval<const Graph&>(),0LL)),vl>);
static_assert(is_same_v<decltype(bfs(declval<const Graph&>(),vl{})),vl>);
static_assert(is_same_v<decltype(bfs(declval<const Graph&>(),0LL,0LL)),ll>);
static_assert(is_same_v<decltype(bfs(declval<const Graph&>(),vl{},0LL)),ll>);
static_assert(is_same_v<decltype(bfs_with_path(declval<const Graph&>(),0LL)),pair<vl,vl>>);
static_assert(is_same_v<decltype(bfs_with_path(declval<const Graph&>(),vl{})),pair<vl,vl>>);
static_assert(is_same_v<decltype(is_reachable(declval<const Graph&>(),0LL,0LL)),bool>);

void check(bool ok,const char* label,ll trial,ll source=-1,ll target=-1){
  if(ok) return;
  cerr << label << " seed=" << seed << " trial=" << trial
       << " source=" << source << " target=" << target << '\n';
  exit(1);
}

vvc<i128> reference(const Graph& G){
  const ll n=ssize(G);
  vvc<i128> D(n,vc<i128>(n,inf));
  for(ll u=0;u<n;u++){
    D[u][u]=0;
    for(ll v:G[u]) D[u][v]=min(D[u][v],i128(1));
  }
  for(ll k=0;k<n;k++) for(ll u=0;u<n;u++) for(ll v=0;v<n;v++)
    D[u][v]=min(D[u][v],D[u][k]+D[k][v]);
  return D;
}

void verify_paths(const Graph& G,const vl& sources,const vl& dist,const vl& par,ll trial){
  const ll n=ssize(G);
  check(ssize(par)==n,"parent size",trial);
  vb root(n);
  for(ll s:sources){
    root[s]=true;
    check(par[s]==s,"source parent",trial,s);
  }

  for(ll t=0;t<n;t++){
    if(dist[t]==-1){
      check(par[t]==-1,"unreachable parent",trial,-1,t);
      check(restore_path(par,t).empty(),"unreachable path",trial,-1,t);
      continue;
    }
    ll v=t;
    vl expected_path{t};
    for(ll steps=0;!root[v];steps++){
      check(steps<n,"parent cycle",trial,-1,t);
      const ll p=par[v];
      check(0<=p && p<n && dist[p]>=0,"parent index",trial,-1,v);
      bool tight_edge=false;
      for(ll next:G[p])
        if(next==v && i128(dist[p])+1==dist[v]) tight_edge=true;
      check(tight_edge,"shortest path edge",trial,p,v);
      expected_path.push_back(p);
      v=p;
    }
    reverse(expected_path.begin(),expected_path.end());
    check(restore_path(par,t)==expected_path,"restored path",trial,v,t);
  }
}

void verify(const Graph& G,const vl& sources,ll trial){
  const ll n=ssize(G);
  const auto D=reference(G);
  vl expected(n,-1);
  for(ll t=0;t<n;t++){
    i128 best=inf;
    for(ll s:sources) best=min(best,D[s][t]);
    if(best==inf) continue;
    check(best<=numeric_limits<ll>::max(),"reference range",trial,-1,t);
    expected[t]=ll(best);
  }

  check(bfs(G,sources)==expected,"multi-source distances",trial);
  const auto [dist,par]=bfs_with_path(G,sources);
  check(dist==expected,"multi-source path distances",trial);
  verify_paths(G,sources,dist,par,trial);
  for(ll t=0;t<n;t++)
    check(bfs(G,sources,t)==expected[t],"multi-source target",trial,-1,t);

  for(ll s=0;s<n;s++){
    vl single(n,-1);
    for(ll t=0;t<n;t++) if(D[s][t]!=inf){
      check(D[s][t]<=numeric_limits<ll>::max(),"reference range",trial,s,t);
      single[t]=ll(D[s][t]);
    }
    check(bfs(G,s)==single,"single-source distances",trial,s);
    const auto [dist,par]=bfs_with_path(G,s);
    check(dist==single,"single-source path distances",trial,s);
    verify_paths(G,vl{s},dist,par,trial);
    for(ll t=0;t<n;t++){
      check(bfs(G,s,t)==single[t],"single-source target",trial,s,t);
      check(is_reachable(G,s,t)==(single[t]!=-1),"reachability",trial,s,t);
    }
  }
}

void cross_check(const Graph& G,const vl& sources,mt19937_64& rng,ll trial){
  vvc<pair<ll,ll>> unit(G.size()),binary(G.size());
  for(ll u=0;u<ssize(G);u++) for(ll v:G[u]){
    unit[u].emplace_back(v,1);
    binary[u].emplace_back(v,ll(rng()%2));
  }
  const vl expected=bfs(G,sources);
  check(bfs01(unit,sources)==expected,"unit BFS vs 0-1 BFS",trial);
  check(dijkstra(unit,sources)==expected,"unit BFS vs Dijkstra",trial);
  check(bfs01(binary,sources)==dijkstra(binary,sources),"binary 0-1 BFS vs Dijkstra",trial);
}

void random_cases(){
  mt19937_64 rng(seed);
  for(ll trial=0;trial<180;trial++){
    const ll n=1+ll(rng()%10);
    Graph G(n);
    const ll m=ll(rng()%(n*n+1));
    for(ll i=0;i<m;i++){
      const ll u=ll(rng()%n),v=ll(rng()%n);
      G[u].push_back(v);
      if(trial%2==0) G[v].push_back(u);
    }
    vl sources;
    const ll k=ll(rng()%(n+3));
    for(ll i=0;i<k;i++) sources.push_back(ll(rng()%n));
    verify(G,sources,trial);
    cross_check(G,sources,rng,trial);
  }
}

void large_case(){
  constexpr ll n=200'000;
  Graph G(n);
  for(ll v=1;v<n;v++) {
    G[v-1].push_back(v);
    G[v].push_back(v-1);
  }
  const auto [dist,par]=bfs_with_path(G,0LL);
  check(bfs(G,0LL,n-1)==n-1,"large target",-4,0,n-1);
  const vl path=restore_path(par,n-1);
  check(ssize(path)==n,"large path size",-4);
  for(ll v=0;v<n;v++)
    check(dist[v]==v && path[v]==v,"large distance and path",-4,0,v);
  const auto multi=bfs(G,vl{0,n-1,0});
  for(ll v=0;v<n;v++)
    check(multi[v]==min(v,n-1-v),"large multi-source distance",-4,-1,v);
}
}

int main(){
  verify(Graph{},{},-1);
  verify(Graph(1),{},-2);
  verify(Graph{{0,1,1},{2},{1},{}},{0,0,3},-3);
  random_cases();
  large_case();
  cout << "bfs tests passed.\n";
}
