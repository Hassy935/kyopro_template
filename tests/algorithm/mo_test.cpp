#include <Library/algorithm/mo.hpp>
#include <cstdlib>
#include <iostream>
#include <random>
#include <set>
#include <memory>

namespace mo_test {
constexpr unsigned long long SEED=0x4D4F2026ULL;
using P=std::pair<ll,ll>;
void check(bool ok,const char* what){
  if(ok) return;
  std::cerr << "[FAIL] " << what << " seed=" << SEED << '\n';
  std::exit(1);
}

void verify_distinct(Mo& m,const std::vector<ll>& a,const std::vector<P>& qs){
  std::vector<ll> cnt(8),active(a.size()),seen(qs.size());
  ll kind=0;
  auto add=[&](ll i){
    check(0<=i && i<std::ssize(a),"add index");
    check(active[i]++==0,"duplicate add");
    if(cnt[a[i]]++==0) kind++;
  };
  auto del=[&](ll i){
    check(0<=i && i<std::ssize(a),"del index");
    check(active[i]--==1,"invalid deletion");
    if(--cnt[a[i]]==0) kind--;
  };
  auto rem=[&](ll id){
    check(0<=id && id<std::ssize(qs),"query id");
    check(seen[id]++==0,"duplicate rem");
    auto [l,r]=qs[id];
    std::set<ll> ref;
    for(ll i=0;i<std::ssize(a);i++){
      check(active[i]==(l<=i && i<r),"active interval");
      if(l<=i && i<r) ref.insert(a[i]);
    }
    check(kind==std::ssize(ref),"distinct count");
  };
  m.run(add,del,rem);
  for(ll v:seen) check(v==1,"missing rem");
}

// 左右独立の更新を検証する．頻度表から転倒数を増減する．
void verify_inversions(Mo& m,const std::vector<ll>& a,const std::vector<P>& qs){
  std::vector<ll> cnt(8),seen(qs.size());
  ll L=0,R=0,inv=0;
  auto less_count=[&](ll x){ll res=0; for(ll j=0;j<x;j++) res+=cnt[j]; return res;};
  auto greater_count=[&](ll x){ll res=0; for(ll j=x+1;j<8;j++) res+=cnt[j]; return res;};
  auto add_left=[&](ll i){
    check(i==L-1 && i>=0,"add_left index");
    --L; inv+=less_count(a[i]); cnt[a[i]]++;
  };
  auto add_right=[&](ll i){
    check(i==R && i<std::ssize(a),"add_right index");
    ++R; inv+=greater_count(a[i]); cnt[a[i]]++;
  };
  auto del_left=[&](ll i){
    check(i==L && L<R,"del_left index");
    ++L; cnt[a[i]]--; inv-=less_count(a[i]);
  };
  auto del_right=[&](ll i){
    check(i==R-1 && L<R,"del_right index");
    --R; cnt[a[i]]--; inv-=greater_count(a[i]);
  };
  auto rem=[&](ll id){
    check(0<=id && id<std::ssize(qs),"inversion query id");
    check(seen[id]++==0,"inversion duplicate rem");
    auto [l,r]=qs[id];
    check(L==l && R==r,"interval endpoints");
    ll ref=0;
    for(ll i=l;i<r;i++) for(ll j=i+1;j<r;j++) ref+=a[i]>a[j];
    check(inv==ref,"inversion count");
  };
  m.run(add_left,add_right,del_left,del_right,rem);
  for(ll v:seen) check(v==1,"inversion missing rem");
}

void verify(Mo m,const std::vector<ll>& a,std::vector<P> qs){
  check(m.size()==0,"initial size");
  auto fail=[](ll){check(false,"callback for zero queries");};
  m.run(fail,fail,fail);
  for(ll i=0;i<std::ssize(qs);i++){
    auto [l,r]=qs[i];
    check(m.add_query(l,r)==i,"add_query id");
  }
  check(m.size()==std::ssize(qs),"size");
  verify_distinct(m,a,qs);
  // 同じオブジェクトを再実行（集計状態は新しく初期化）．
  verify_inversions(m,a,qs);
  // ソート後も登録順IDを維持し，実際のクエリ数で幅を再計算する．
  ll n=std::ssize(a);
  check(m.add_query(0,n)==std::ssize(qs),"append after run");
  qs.emplace_back(0,n);
  check(m.add_query(n,n)==std::ssize(qs),"append empty query");
  qs.emplace_back(n,n);
  check(m.size()==std::ssize(qs),"size after append");
  verify_distinct(m,a,qs);
  verify_inversions(m,a,qs);
}

void run(){
  std::mt19937_64 rng(SEED);
  for(ll n=0;n<=24;n++){
    std::vector<ll> a(n);
    for(auto& x:a) x=rng()%8;
    std::vector<P> qs;
    for(ll l=0;l<=n;l++) for(ll r=l;r<=n;r++) qs.emplace_back(l,r);
    qs.emplace_back(n,n); qs.emplace_back(0,n);
    std::shuffle(qs.begin(),qs.end(),rng);
    verify(Mo(n),a,qs);
    verify(Mo(n,1,Mo::block),a,qs);
    verify(Mo(n,1000,Mo::hilbert),a,qs);
    for(ll w:{1LL,2LL,7LL,n+1}) verify(Mo(n,0,w),a,qs);
  }
  for(ll ti=0;ti<100;ti++){
    ll n=rng()%129,q=rng()%301;
    std::vector<ll> a(n);
    for(auto& x:a) x=rng()%8;
    std::vector<P> qs;
    for(ll i=0;i<q;i++){
      ll l=rng()%(n+1),r=rng()%(n+1);
      if(l>r) std::swap(l,r);
      qs.emplace_back(l,r);
    }
    verify(Mo(n,q),a,qs);
    verify(Mo(n,q,Mo::hilbert),a,qs);
    verify(Mo(n,q,1LL+(ll)(rng()%200)),a,qs);
  }
  // 最大許容Nでキーを生成する（区間移動自体は小さく保つ）．
  for(auto order:{Mo::block,Mo::hilbert}){
    Mo m((1LL<<31)-1,0,order);
    m.add_query(0,2); m.add_query(1,1);
    ll sum=0,rem_count=0;
    m.run([&](ll){++sum;},[&](ll){--sum;},[&](ll id){
      check(sum==(id==0?2:0),"maximum N"); ++rem_count;
    });
    check(rem_count==2,"maximum N rem count");
  }
  // 左右共通コールバックをコピーしないことも確認する．
  Mo m(3);
  m.add_query(1,3);
  auto add=[p=std::make_unique<ll>(0)](ll) mutable {++*p;};
  auto del=[](ll){};
  ll rem_count=0;
  m.run(add,del,[&](ll){++rem_count;});
  check(rem_count==1,"move-only callback");
}
}

int main(){
  mo_test::run();
  std::cout << "mo: all tests passed (seed=" << mo_test::SEED << ")\n";
}
