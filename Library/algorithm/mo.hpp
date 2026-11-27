#pragma once
#include <Library/core/common_setting.hpp>

class Mo {
public:
  enum Order { block, hilbert };

private:
  struct Query {
    ll l,r,id,ord;
  };

  ll n,width=-1;
  Order order;
  vector<Query> qs;

  // 非再帰版ヒルベルト順．反転は残りの4進数の各桁に反映する．
  static ll hilbert_order(ll x,ll y,ll pw){
    ll ord=0,rot=0;
    bool rev=false;
    constexpr ll delta[]={3,0,0,1};
    while(pw>0){
      ll h=1LL<<(--pw);
      ll seg=(x<h?(y<h?0:3):(y<h?1:2));
      seg=(seg+rot)&3;
      ord=(ord<<2)|(rev?3-seg:seg);
      rev^=(seg==0||seg==3);
      rot=(rot+delta[seg])&3;
      x&=h-1;
      y&=h-1;
    }
    return ord;
  }

  void sort_queries(){
    if(qs.empty()) return;
    if(order==hilbert){
      ll pw=0;
      while((1LL<<pw)<=n) pw++;
      for(auto& q:qs) q.ord=hilbert_order(q.l,q.r,pw);
    }else{
      ll w=width;
      if(w==-1){
        w=max(1LL,(ll)(sqrtl(1.5L)*n/sqrtl((long double)qs.size())));
      }
      for(auto& q:qs){
        ll b=q.l/w;
        q.ord=(b<<32)|((b&1)?n-q.r:q.r);
      }
    }
    sort(qs.begin(),qs.end(),[](const Query& p,const Query& q){
      return p.ord<q.ord;
    });
  }

public:
  // q は予約容量．自動ブロック幅には実際の登録数を使用する．
  explicit Mo(ll n_,ll q=0,Order order_=block):n(n_),order(order_){
    assert(0<=n && n<(1LL<<31));
    assert(q>=0);
    assert(order==block || order==hilbert);
    qs.reserve(q);
  }

  Mo(ll n_,ll q,ll w):Mo(n_,q,block){
    assert(w>=1);
    width=w;
  }

  ll size() const { return ssize(qs); }

  // 半開区間 [l,r) を登録し，登録順のIDを返す．
  ll add_query(ll l,ll r){
    assert(0<=l && l<=r && r<=n);
    ll id=size();
    qs.push_back({l,r,id,0});
    return id;
  }

  // 呼び出し前に集計状態を空区間 [0,0) に初期化すること．
  // 終了時に集計状態を初期化しない．
  template<class AddL,class AddR,class DelL,class DelR,class Rem>
  void run(AddL&& add_left,AddR&& add_right,DelL&& del_left,DelR&& del_right,Rem&& rem){
    sort_queries();
    ll L=0,R=0;
    for(const auto& q:qs){
      while(L>q.l) add_left(--L);
      while(R<q.r) add_right(R++);
      while(L<q.l) del_left(L++);
      while(R>q.r) del_right(--R);
      rem(q.id);
    }
  }

  template<class Add,class Del,class Rem>
  void run(Add&& add,Del&& del,Rem&& rem){
    run(add,add,del,del,rem);
  }
};
