#pragma once

// ヒルベルト曲線の順．
struct Mo{
  struct Query{
    ll l,r,id,ord;
  };

  ll n,pw=0;
  vector<Query> qs;

  // n: 配列長、q: クエリ数（省略可能）
  Mo(ll n,ll q=0):n(n){
    assert(0<=n&&n<(1LL<<31));
    assert(q>=0);

    // 2^pw > n を満たす最小の pw．
    while((1LL<<pw)<=n) pw++;

    qs.reserve(q);
  }

  // 2^pw × 2^pw の格子における
  // 座標 (x,y) のヒルベルト順を返す．
  static ll hilbert_order(ll x,ll y,ll pw,ll rot=0){
    if(pw==0) return 0;

    ll h=1LL<<(pw-1);

    ll seg;
    if(x<h){
      seg=(y<h?0:3);
    }else{
      seg=(y<h?1:2);
    }

    seg=(seg+rot)%4;

    const ll delta[4]={3,0,0,1};

    ll nx=x&(h-1),ny=y&(h-1);
    ll nrot=(rot+delta[seg])%4;

    ll sz=1LL<<(2*pw-2);
    ll sub=hilbert_order(nx,ny,pw-1,nrot);

    if(seg==1||seg==2){
      return seg*sz+sub;
    }else{
      return seg*sz+(sz-sub-1);
    }
  }

  // 半開区間 [l,r) を登録する．
  // 登録順のクエリIDを返す．
  // O(log n)
  ll add_query(ll l,ll r){
    assert(0<=l&&l<=r&&r<=n);

    ll id=qs.size();
    ll ord=hilbert_order(l,r,pw);

    qs.push_back({l,r,id,ord});
    return id;
  }

  // 左右で異なる処理を行う場合．
  template<class AddL,class AddR,class DelL,class DelR,class Answer>
  void run(AddL add_left,AddR add_right,DelL del_left,DelR del_right,Answer answer){

    // ヒルベルト順にソートする．
    sort(qs.begin(),qs.end(),
      [&](const Query& p,const Query& q){
        return p.ord<q.ord;
      }
    );

    ll L=0,R=0;

    for(const auto& q:qs){
      ll l=q.l,r=q.r,id=q.id;

      while(L>l) add_left(--L);
      while(R<r) add_right(R++);
      while(L<l) del_left(L++);
      while(R>r) del_right(--R);

      answer(id);
    }
  }

  // 左右で同じ処理を行う場合．
  template<class Add,class Del,class Answer>
  void run(Add add,Del del,Answer answer){
    run(add,add,del,del,answer);
  }
};


// ブロック分けの順．
struct Mo{
  struct Query{
    ll l,r,id;
  };

  ll n,width;
  vector<Query> qs;

  // n: 配列長、q: クエリ数（省略可能）
  // w: ブロック幅（-1なら自動設定）
  Mo(ll n,ll q=0,ll w=-1):n(n){
    assert(n>=0&&q>=0);
    assert(w==-1||w>=1);

    if(w!=-1){
      width=w;
    }else if(q==0){
      width=max(1LL,floor_sqrt(n));
    }else{
      width=max(1LL,(ll)(sqrtl(3.0L)*n/sqrtl(2.0L*q)));
    }

    qs.reserve(q);
  }

  // 半開区間 [l,r) を登録する．
  // 登録順のクエリIDを返す．
  // 償却 O(1)
  ll add_query(ll l,ll r){
    assert(0<=l&&l<=r&&r<=n);

    ll id=qs.size();
    qs.push_back({l,r,id});
    return id;
  }

  // 左右で異なる処理を行う場合．
  template<class AddL,class AddR,class DelL,class DelR,class Answer>
  void run(AddL add_left,AddR add_right,DelL del_left,DelR del_right,Answer answer){

    // 偶奇ブロック順にソートする．
    sort(qs.begin(),qs.end(),
      [&](const Query& p,const Query& q){
        ll p_b=p.l/width,q_b=q.l/width;

        if(p_b!=q_b) return p_b<q_b;
        if(p_b&1) return p.r>q.r;
        return p.r<q.r;
      }
    );

    ll L=0,R=0;

    for(const auto& q:qs){
      ll l=q.l,r=q.r,id=q.id;

      while(L>l) add_left(--L);
      while(R<r) add_right(R++);
      while(L<l) del_left(L++);
      while(R>r) del_right(--R);

      answer(id);
    }
  }

  // 左右で同じ処理を行う場合．
  template<class Add,class Del,class Answer>
  void run(Add add,Del del,Answer answer){
    run(add,add,del,del,answer);
  }
};