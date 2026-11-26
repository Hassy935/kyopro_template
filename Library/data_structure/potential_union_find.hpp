#pragma once
#include <Library/core/common_setting.hpp>

// merge(x,y,w): P(y)-P(x)=w という制約を追加する．
// T は加法可換群（T{} が零元）を表す型とする．
template<class T=ll>
class PotentialUnionFind {
  ll _n,cc,bad_cc=0;
  vl par,sz;
  vc<T> diff_weight;
  vb bad;

  void check(ll x) const {
    assert(0<=x && x<_n);
  }

  ll root(ll x){
    if(par[x]==-1) return x;
    ll p=par[x];
    ll l=root(p);
    diff_weight[x]=diff_weight[x]+diff_weight[p];
    return par[x]=l;
  }

public:
  explicit PotentialUnionFind(ll n): _n(n),cc(n){
    assert(n>=0);
    par.assign(n,-1);
    sz.assign(n,1);
    diff_weight.assign(n,T{});
    bad.assign(n,false);
  }

  ll leader(ll x){
    check(x);
    return root(x);
  }

  ll size(ll x){
    return sz[leader(x)];
  }

  // 管理している全域木上の P(x)-P(leader(x))．
  // 矛盾のある成分では全制約を満たす値ではない．
  T weight(ll x){
    leader(x);
    return diff_weight[x];
  }

  bool same(ll x,ll y){
    return leader(x)==leader(y);
  }

  // 戻り値は実際に異なる成分を併合したかどうか．
  bool merge(ll x,ll y,T w){
    ll rx=leader(x),ry=leader(y);

    if(rx==ry){
      if(!bad[rx] && !(diff_weight[y]-diff_weight[x]==w)){
        bad[rx]=true;
        bad_cc++;
      }
      return false;
    }

    w=w+diff_weight[x]-diff_weight[y];
    if(sz[rx]<sz[ry]){
      swap(rx,ry);
      w=T{}-w;
    }

    if(bad[rx] && bad[ry]) bad_cc--;
    bad[rx]=bad[rx] || bad[ry];
    bad[ry]=false;

    par[ry]=rx;
    sz[rx]+=sz[ry];
    diff_weight[ry]=w;
    cc--;
    return true;
  }

  // 同一成分かつ矛盾がないことが前提．
  T diff(ll x,ll y){
    ll rx=leader(x),ry=leader(y);
    assert(rx==ry && !bad[rx]);
    return diff_weight[y]-diff_weight[x];
  }

  optional<T> try_diff(ll x,ll y){
    ll rx=leader(x),ry=leader(y);
    if(rx!=ry || bad[rx]) return nullopt;
    return diff_weight[y]-diff_weight[x];
  }

  bool consistent(ll x){
    return !bad[leader(x)];
  }

  bool consistent() const {
    return bad_cc==0;
  }

  ll comp_cnt() const {
    return cc;
  }

  ll bad_comp_cnt() const {
    return bad_cc;
  }

  vvl groups(){
    vl leader_id(_n),group_size(_n);
    rep(i,_n){
      leader_id[i]=leader(i);
      group_size[leader_id[i]]++;
    }

    vvl res(_n);
    rep(i,_n) res[i].reserve(group_size[i]);
    rep(i,_n) res[leader_id[i]].pb(i);

    res.erase(remove_if(nall(res),[](const vl& v){
      return v.empty();
    }),res.end());
    return res;
  }
};

template<class T=ll>
using potential_union_find=PotentialUnionFind<T>;

template<class T=ll>
using p_dsu=PotentialUnionFind<T>;
