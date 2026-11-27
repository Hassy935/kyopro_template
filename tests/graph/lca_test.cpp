#include <Library/graph/lca.hpp>

namespace {
using Tree=vvc<pair<ll,ll>>;
constexpr ull seed=0x1CA2026ULL;
ll trial=0,root_case=-1;

static_assert(is_same_v<lca,LCA>);
static_assert(is_same_v<decltype(declval<const LCA&>().path(0,0)),vl>);
static_assert(is_same_v<decltype(declval<const LCA&>().lca(0,0,0)),ll>);

void check(bool ok,const char* label,ll u=-1,ll v=-1,ll k=-1){
  if(ok) return;
  cerr << label << " seed=" << seed << " case=" << trial << " root=" << root_case
       << " u=" << u << " v=" << v << " k=" << k << '\n';
  exit(1);
}

void add_edge(Tree& G,ll u,ll v,ll cost){
  G[u].emplace_back(v,cost);
  G[v].emplace_back(u,cost);
}

struct Reference{
  vl parent,depth;
  vector<i128> cost;
};

Reference rooted(const Tree& G,ll root){
  const ll n=ssize(G);
  Reference R{vl(n,-1),vl(n,-1),vector<i128>(n)};
  queue<ll> pending;
  R.depth[root]=0;
  pending.push(root);
  while(!pending.empty()){
    const ll u=pending.front();
    pending.pop();
    for(auto [v,cost]:G[u]){
      if(R.depth[v]!=-1) continue;
      R.parent[v]=u;
      R.depth[v]=R.depth[u]+1;
      R.cost[v]=R.cost[u]+i128(cost);
      pending.push(v);
    }
  }
  return R;
}

ll naive_ancestor(const Reference& R,ll v,ll k){
  if(k>R.depth[v]) return -1;
  while(k--) v=R.parent[v];
  return v;
}

ll naive_lca(const Reference& R,ll u,ll v){
  while(R.depth[u]>R.depth[v]) u=R.parent[u];
  while(R.depth[v]>R.depth[u]) v=R.parent[v];
  while(u!=v){
    u=R.parent[u];
    v=R.parent[v];
  }
  return u;
}

vl naive_path(const Reference& from_u,ll v){
  vl result;
  for(;v!=-1;v=from_u.parent[v]) result.push_back(v);
  reverse(result.begin(),result.end());
  return result;
}

void verify(const Tree& G,ll root){
  root_case=root;
  const ll n=ssize(G);
  vvl unweighted(n);
  Tree ones(n);
  vector<Reference> ref;
  for(ll u=0;u<n;u++){
    ref.push_back(rooted(G,u));
    for(auto [v,cost]:G[u]){
      unweighted[u].push_back(v);
      ones[u].emplace_back(v,1);
    }
  }
  const lca tree(G,root),plain(unweighted,root),unit(ones,root);
  const auto& from_root=ref[root];
  const ll largest=numeric_limits<ll>::max();
  for(const LCA* T:{&tree,&plain,&unit}){
    check(T->size()==n && T->root()==root,"size or root");
    for(ll v=0;v<n;v++){
      check(T->parent(v)==from_root.parent[v],"parent",v);
      check(T->depth(v)==from_root.depth[v],"depth",v);
      for(ll k=0;k<=from_root.depth[v]+1;k++)
        check(T->kth_ancestor(v,k)==naive_ancestor(from_root,v,k),"kth_ancestor",v,-1,k);
      check(T->kth_ancestor(v,largest)==-1,"huge ancestor",v);
    }
  }

  for(ll u=0;u<n;u++) for(ll v=0;v<n;v++){
    const ll a=naive_lca(from_root,u,v);
    const vl path=naive_path(ref[u],v);
    const ll length=ssize(path)-1;
    const i128 weight=ref[u].cost[v];
    if(numeric_limits<ll>::min()<=weight && weight<=largest)
      check(tree.weighted_dist(u,v)==ll(weight),"weighted distance",u,v);
    check(plain.weighted_dist(u,v)==length,"unweighted distance",u,v);
    check(unit.weighted_dist(u,v)==length,"unit weighted distance",u,v);
    for(const LCA* T:{&tree,&plain,&unit}){
      check(T->lca(u,v)==a,"lca",u,v);
      check(T->dist(u,v)==length,"distance",u,v);
      check(T->path(u,v)==path,"path",u,v);
      check(T->is_ancestor(u,v)==(a==u),"is_ancestor",u,v);
      for(ll r=0;r<n;r++)
        check(T->lca(u,v,r)==naive_lca(ref[r],u,v),"rerooted lca",u,v,r);
      for(ll k=0;k<=length;k++)
        check(T->jump(u,v,k)==path[k],"jump",u,v,k);
      check(T->jump(u,v,length+1)==-1 && T->jump(u,v,largest)==-1,"jump out of path",u,v);
      for(ll x=0;x<n;x++)
        check(T->on_path(u,v,x)==(find(path.begin(),path.end(),x)!=path.end()),"on_path",u,v,x);
    }
  }
  ++trial;
}

void small_cases(){
  for(ll n:{1,2,7,8,9,15,16,17}) for(int shape=0;shape<3;shape++){
    Tree G(n);
    for(ll v=1;v<n;v++){
      const ll parent=shape==0 ? v-1 : shape==1 ? 0 : (v-1)/2;
      add_edge(G,v,parent,v%5-2);
    }
    verify(G,0);
    if(n>1) verify(G,n-1);
  }
  mt19937_64 rng(seed);
  for(int test=0;test<100;test++){
    const ll n=1+ll(rng()%16);
    Tree G(n);
    vl order(n);
    iota(order.begin(),order.end(),0LL);
    shuffle(order.begin(),order.end(),rng);
    for(ll v=1;v<n;v++){
      const ll p=ll(rng()%v),cost=ll(rng()%201)-100;
      add_edge(G,order[v],order[p],cost);
    }
    verify(G,ll(rng()%n));
  }
}

void extreme_weights(){
  const ll hi=numeric_limits<ll>::max(),lo=numeric_limits<ll>::min();
  Tree G(7);
  add_edge(G,0,1,hi);
  add_edge(G,1,2,hi);
  add_edge(G,2,3,lo);
  add_edge(G,0,4,lo);
  add_edge(G,4,5,lo);
  add_edge(G,5,6,hi);
  verify(G,0);
  verify(G,2);
  const LCA T(G);
  check(T.weighted_dist(0,1)==hi && T.weighted_dist(0,4)==lo,"extreme edges");
  check(T.weighted_dist(1,3)==-1 && T.weighted_dist(4,6)==-1,"extreme cancellation");
  check(T.weighted_dist(2,3)==lo && T.weighted_dist(5,6)==hi,"prefix outside ll");
  check(T.weighted_dist(2,2)==0 && T.weighted_dist(5,5)==0,"extreme prefix same vertex");

  Tree H(4);
  add_edge(H,0,1,hi);
  add_edge(H,1,2,1);
  add_edge(H,1,3,2);
  const LCA U(H);
  check(U.weighted_dist(2,3)==3,"overflowing root sums and final cancellation");
}

void source_lifetime(){
  vvl G={{1},{0,2},{1}};
  const LCA T(G,2);
  G.clear();
  check(T.path(0,2)==vl({0,1,2}) && T.parent(0)==1,"modified input");
  const LCA W=[]{
    Tree H(2);
    add_edge(H,0,1,-5);
    return LCA(H);
  }();
  check(W.weighted_dist(0,1)==-5 && W.path(1,0)==vl({1,0}),"destroyed input");
}

void large_cases(){
  constexpr ll n=200'000;
  const ll largest=numeric_limits<ll>::max();
  root_case=0;
  {
    vvl G(n);
    for(ll v=1;v<n;v++){
      G[v-1].push_back(v);
      G[v].push_back(v-1);
    }
    const LCA T(G);
    check(T.depth(n-1)==n-1 && T.kth_ancestor(n-1,n-1)==0,"deep path");
    check(T.kth_ancestor(n-1,1LL<<17)==n-1-(1LL<<17),"highest doubling level");
    check(T.kth_ancestor(n-1,largest)==-1 && T.jump(0,n-1,largest)==-1,"large k");
    const vl rev=T.path(n-1,0);
    check(ssize(rev)==n,"long path size");
    for(ll v=0;v<n;v++) check(rev[v]==n-1-v,"long path order",v);
    for(ll k=0;k<n;k+=7)
      check(T.jump(0,n-1,k)==k && T.lca(k,n-1,0)==k,"large path queries",k);
  }
  ++trial;
  root_case=1;
  {
    vvl G(n);
    for(ll v=1;v<n;v++){
      G[0].push_back(v);
      G[v].push_back(0);
    }
    const LCA T(G,1);
    for(ll v=2;v<n;v++)
      check(T.lca(1,v)==1 && T.lca(2,v)==(v==2 ? v : 0) &&
            T.dist(1,v)==2 && T.jump(v,1,1)==0,"large star",v);
    check(T.path(n-1,1)==vl({n-1,0,1}) && T.is_ancestor(0,n-1),"large star path");
  }
  ++trial;
  root_case=0;
  {
    mt19937_64 rng(seed);
    Tree G(n);
    for(ll v=1;v<n;v++){
      const ll p=ll(rng()%v),cost=ll(rng()%2001)-1000;
      add_edge(G,v,p,cost);
    }
    const auto R=rooted(G,0);
    const LCA T(G);
    for(ll q=0;q<30'000;q++){
      const ll u=ll(rng()%n),v=ll(rng()%n);
      const ll a=naive_lca(R,u,v),p=R.depth[u]-R.depth[a],s=R.depth[v]-R.depth[a];
      const ll k=ll(rng()%(p+s+1));
      const ll next=k<=p ? naive_ancestor(R,u,k) : naive_ancestor(R,v,p+s-k);
      const i128 weight=(R.cost[u]-R.cost[a])+(R.cost[v]-R.cost[a]);
      check(T.lca(u,v)==a && T.dist(u,v)==p+s && T.weighted_dist(u,v)==ll(weight) &&
            T.jump(u,v,k)==next && T.on_path(u,v,next),"large random tree",u,v,k);
    }
  }
}
}

int main(){
  small_cases();
  extreme_weights();
  source_lifetime();
  large_cases();
  cout << "lca tests passed.\n";
}
