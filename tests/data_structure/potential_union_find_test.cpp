#include <Library/template.hpp>

#if __has_include(<atcoder/modint>)
#include <atcoder/modint>
#define P_DSU_TEST_ACL 1
#endif

// ACL がなくても加法群への一般化を検証できる型．
struct Mod7 {
  ll v=0;
  Mod7()=default;
  explicit Mod7(ll x): v((x%7+7)%7) {}
  friend Mod7 operator+(Mod7 a,Mod7 b){ return Mod7(a.v+b.v); }
  friend Mod7 operator-(Mod7 a,Mod7 b){ return Mod7(a.v-b.v); }
  friend bool operator==(Mod7 a,Mod7 b){ return a.v==b.v; }
};

// 制約をすべて辺として保持し、BFS で毎回再計算する愚直解．
struct Naive {
  ll n,mod;
  vvc<pair<ll,ll>> g;
  vl comp,pot,sz;
  vb bad;
  explicit Naive(ll n,ll mod): n(n),mod(mod),g(n){ rebuild(); }
  ll norm(ll x) const { return mod?(x%mod+mod)%mod:x; }
  void add(ll x,ll y,ll w){
    g[x].eb(y,norm(w));
    g[y].eb(x,norm(-w));
    rebuild();
  }
  void rebuild(){
    comp.assign(n,-1);
    pot.assign(n,0);
    bad.clear(); sz.clear();
    rep(s,n){
      if(comp[s]!=-1) continue;
      ll c=ssize(sz);
      bad.pb(false); sz.pb(0);
      queue<ll> q;
      q.push(s); comp[s]=c;
      while(!q.empty()){
        ll x=q.front(); q.pop(); sz[c]++;
        for(auto [y,w]:g[x]){
          ll p=norm(pot[x]+w);
          if(comp[y]==-1){
            comp[y]=c; pot[y]=p; q.push(y);
          }else if(pot[y]!=p){
            bad[c]=true;
          }
        }
      }
    }
  }
};

template<class T>
void verify(PotentialUnionFind<T>& uf,const Naive& nv){
  ll bc=count(nall(nv.bad),true);
  assert(uf.comp_cnt()==ssize(nv.sz));
  assert(uf.bad_comp_cnt()==bc);
  assert(uf.consistent()==(bc==0));
  rep(x,nv.n){
    ll c=nv.comp[x];
    assert(uf.size(x)==nv.sz[c]);
    assert(uf.consistent(x)==!nv.bad[c]);
    ll rt=uf.leader(x);
    assert(uf.leader(rt)==rt);
    assert(uf.weight(rt)==T{});
    if(!nv.bad[c]) assert(uf.weight(x)==T(nv.norm(nv.pot[x]-nv.pot[rt])));
    rep(y,nv.n){
      bool same=nv.comp[x]==nv.comp[y];
      assert(uf.same(x,y)==same);
      auto d=uf.try_diff(x,y);
      bool valid=same && !nv.bad[c];
      assert(d.has_value()==valid);
      if(valid){
        T want(nv.norm(nv.pot[y]-nv.pot[x]));
        assert(*d==want);
        assert(uf.diff(x,y)==want);
      }
    }
  }
  auto gs=uf.groups();
  vvl want(nv.sz.size());
  rep(x,nv.n) want[nv.comp[x]].pb(x);
  sort(nall(gs)); sort(nall(want));
  assert(gs==want);
}

template<class T>
void step(PotentialUnionFind<T>& uf,Naive& nv,ll x,ll y,ll w){
  bool should_merge=nv.comp[x]!=nv.comp[y];
  bool merged=uf.merge(x,y,T(w));
  assert(merged==should_merge);
  nv.add(x,y,w);
  verify(uf,nv);
}

template<class T>
void exhaustive(ll mod){
  using E=tuple<ll,ll,ll>;
  vc<E> es;
  rep(x,3) forn(y,x,3) forc(w,-1,1) es.eb(x,y,w);
  ll cases=0;
  for(auto [x,y,w]:es) for(auto [a,b,c]:es) for(auto [d,e,f]:es){
    PotentialUnionFind<T> uf(3);
    Naive nv(3,mod);
    step(uf,nv,x,y,w);
    step(uf,nv,a,b,c);
    step(uf,nv,d,e,f);
    cases++;
  }
  cout << "Exhaustive: " << cases << " sequences passed\n";
}

template<class T>
void randomized(ll mod){
  mt19937_64 rng(935);
  ll ops=0;
  rep(t,200){
    ll n=1+rng()%20;
    PotentialUnionFind<T> uf(n);
    Naive nv(n,mod);
    vl p(n);
    for(auto& x:p) x=ll(rng()%201)-100;
    rep(i,100){
      ll x=rng()%n,y=rng()%n;
      // 矛盾なしの試行と矛盾ありの試行の両方を検証．
      ll w=(t%2==0 || rng()%5!=0)?p[y]-p[x]:ll(rng()%201)-100;
      step(uf,nv,x,y,w);
      ops++;
    }
  }
  cout << "Random: " << ops << " operations passed\n";
}

void boundary(){
  p_dsu uf(0);
  static_assert(is_same_v<decltype(uf),PotentialUnionFind<ll>>);
  assert(uf.groups().empty() && uf.consistent() && uf.comp_cnt()==0);
  potential_union_find alias(1);
  assert(alias.diff(0,0)==0);
  p_dsu a(6);
  assert(!a.try_diff(0,1));
  assert(a.merge(0,1,3));
  assert(a.merge(2,3,5));
  assert(!a.merge(0,1,4));
  assert(!a.merge(2,3,6));
  assert(a.bad_comp_cnt()==2);
  assert(a.merge(1,3,7));
  assert(a.bad_comp_cnt()==1 && !a.consistent(2));
  assert(a.merge(4,0,2)); // 小さい成分から大きい成分への併合
  assert(a.bad_comp_cnt()==1 && !a.consistent(4));
  assert(!a.merge(5,5,1));
  assert(a.bad_comp_cnt()==2);
  assert(a.merge(5,4,0));
  assert(a.bad_comp_cnt()==1 && a.comp_cnt()==1);
  auto b=a;
  assert(b.bad_comp_cnt()==1 && b.groups()==a.groups());
  p_dsu<i128> large(3);
  i128 w=i128(1)<<90;
  assert(large.merge(0,1,w));
  assert(large.merge(2,1,-w));
  assert(large.diff(0,2)==2*w);
  cout << "Boundary and alias tests passed\n";
}

int main(){
  boundary();
  cout << "ll\n";
  exhaustive<ll>(0); randomized<ll>(0);
  cout << "Mod7\n";
  exhaustive<Mod7>(7); randomized<Mod7>(7);
#ifdef P_DSU_TEST_ACL
  cout << "ACL modint998244353\n";
  randomized<atcoder::modint998244353>(998244353);
#else
  cout << "ACL modint: skipped (header not installed)\n";
#endif
  cout << "All tests passed.\n";
}
