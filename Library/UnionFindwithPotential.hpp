#pragma once

struct DSU{
  ll _n,cc;
  vl par;
  vl sz;

  DSU(ll n){
    _n=n;
    cc=n;
    par.resize(n,-1);
    sz.resize(n,1);
  }

  ll leader(ll x){
    if(par[x]==-1) return x;
    else return par[x]=leader(par[x]);
  }

  ll size(ll x){
    return sz[leader(x)];
  }

  bool same(ll x, ll y){
    return leader(x)==leader(y);
  }

  bool merge(ll x, ll y){
    x=leader(x); y=leader(y);

    if(x==y) return false;

    cc--;

    if(sz[x]<sz[y]) swap(x,y);

    par[y]=x;
    sz[x]=sz[x]+sz[y];

    return true;
  }

  ll comp_cnt()const{
    return cc;
  }

  vvl groups(){
    vl leader_id(_n),group_size(_n);
    rep(i,_n){
      leader_id[i]=leader(i);
      group_size[leader_id[i]]++;
    }

    vvl res(_n);
    rep(i,_n){
      res[i].reserve(group_size[i]);
    }
    rep(i,_n){
      res[leader_id[i]].pb(i);
    }

    res.erase(remove_if(nall(res),[](vl& v){
      return v.empty();
    }),res.end());

    return res;
  }
};