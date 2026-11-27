#include <Library/algorithm/mo.hpp>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string_view>
#include <vector>

namespace mo_benchmark {
using Clock=std::chrono::steady_clock;
using U=unsigned long long;
constexpr ll K=4096;
constexpr int REPEATS=5;
constexpr U SEED=0x4D4F2026ULL;
struct Range { ll l,r; };
struct Result { double registration,run,total; U moves; };

void require(bool ok,const char* msg){
  if(!ok){ std::cerr << "[FAIL] " << msg << '\n'; std::exit(1); }
}
double ms(Clock::time_point a,Clock::time_point b){
  return std::chrono::duration<double,std::milli>(b-a).count();
}

// 値域 [0,K) の頻度を管理する BIT．他の自作ライブラリには依存しない．
struct Fenwick {
  std::vector<ll> bit=std::vector<ll>(K+1);
  void add(ll i,ll d){ for(++i;i<=K;i+=i&-i) bit[i]+=d; }
  ll sum(ll r) const { // [0,r)
    ll res=0;
    for(;r>0;r-=r&-r) res+=bit[r];
    return res;
  }
};

std::vector<Range> make_queries(ll n,ll q,std::string_view type,std::mt19937_64& rng){
  auto pick=[&](ll lo,ll hi){return std::uniform_int_distribution<ll>(lo,hi)(rng);};
  std::vector<Range> ranges;
  ranges.reserve(q);
  for(ll i=0;i<q;++i){
    ll l,r;
    if(type=="short"){
      l=pick(0,n); r=pick(l,std::min(n,l+std::max(1LL,n/100)));
    }else if(type=="long"){
      l=pick(0,n/10); r=pick(n-n/10,n);
    }else if(type=="clustered"){
      ll lo=pick(0,3)*n/4,hi=std::min(n,lo+std::max(1LL,n/20));
      l=pick(lo,hi); r=pick(lo,hi);
    }else if(type=="duplicates" && i>=128){
      auto e=ranges[pick(0,127)]; l=e.l; r=e.r;
    }else{ l=pick(0,n); r=pick(0,n); }
    if(l>r) std::swap(l,r);
    ranges.push_back({l,r});
  }
  return ranges;
}

template<bool Inversions>
Result measure(const std::vector<ll>& a,const std::vector<Range>& ranges,
               Mo::Order order,std::vector<ll>& ans){
  // 入力生成，集計領域の初期化，解答の検証・破棄は測定区間外．
  std::vector<ll> cnt(K);
  Fenwick fw;
  ll kind=0,inv=0,len=0,prev_l=0,prev_r=0;
  U moves=0;
  auto rem=[&](ll id){
    ans[id]=Inversions?inv:kind;
    auto [l,r]=ranges[id];
    moves+=static_cast<U>(std::abs(l-prev_l))+static_cast<U>(std::abs(r-prev_r));
    prev_l=l; prev_r=r;
  };
  auto t0=Clock::now();
  Mo m(a.size(),ranges.size(),order);
  for(auto [l,r]:ranges) m.add_query(l,r);
  auto t1=Clock::now();
  if constexpr(Inversions){
    auto add_left=[&](ll i){ll x=a[i]; inv+=fw.sum(x); fw.add(x,1); ++len;};
    auto add_right=[&](ll i){ll x=a[i]; inv+=len-fw.sum(x+1); fw.add(x,1); ++len;};
    auto del_left=[&](ll i){ll x=a[i]; inv-=fw.sum(x); fw.add(x,-1); --len;};
    auto del_right=[&](ll i){ll x=a[i]; inv-=len-fw.sum(x+1); fw.add(x,-1); --len;};
    m.run(add_left,add_right,del_left,del_right,rem);
  }else{
    auto add=[&](ll i){if(cnt[a[i]]++==0) ++kind;};
    auto del=[&](ll i){if(--cnt[a[i]]==0) --kind;};
    m.run(add,del,rem);
  }
  auto t2=Clock::now();
  return {ms(t0,t1),ms(t1,t2),ms(t0,t2),moves};
}

void self_test(){
  std::mt19937_64 rng(SEED);
  for(ll n:{0LL,1LL,16LL,40LL}){
    std::vector<ll> a(n);
    for(auto& x:a) x=rng()%7;
    std::vector<Range> ranges;
    for(ll l=0;l<=n;++l) for(ll r=l;r<=n;++r) ranges.push_back({l,r});
    std::shuffle(ranges.begin(),ranges.end(),rng);
    std::vector<ll> distinct(ranges.size()),inversions(ranges.size()),ans(ranges.size());
    for(size_t id=0;id<ranges.size();++id){
      auto [l,r]=ranges[id];
      std::vector<ll> cnt(7);
      for(ll i=l;i<r;++i){
        if(cnt[a[i]]++==0) ++distinct[id];
        for(ll j=i+1;j<r;++j) inversions[id]+=(a[i]>a[j]);
      }
    }
    for(auto order:{Mo::block,Mo::hilbert}){
      std::fill(ans.begin(),ans.end(),-1);
      measure<false>(a,ranges,order,ans);
      require(ans==distinct,"small distinct brute force");
      std::fill(ans.begin(),ans.end(),-1);
      measure<true>(a,ranges,order,ans);
      require(ans==inversions,"small inversion brute force");
    }
  }
  std::cout << "[PASS] Small brute force checks for both workloads.\n";
}

double median(std::vector<double> v){std::sort(v.begin(),v.end()); return v[v.size()/2];}

template<bool Inversions>
void bench(const std::vector<ll>& a,const std::vector<Range>& ranges){
  constexpr Mo::Order orders[]={Mo::block,Mo::hilbert};
  const char* names[]={"Block","Hilbert"};
  std::vector<Result> results[2];
  std::vector<ll> ans(ranges.size()),expected;
  for(int t=0;t<REPEATS;++t) for(int j=0;j<2;++j){
    int k=(j+t)%2;
    std::fill(ans.begin(),ans.end(),-1);
    auto res=measure<Inversions>(a,ranges,orders[k],ans);
    if(t==0 && j==0){
      for(ll x:ans) require(x>=0,"missing or negative answer");
      expected=ans;
    }else require(ans==expected,"answer vector mismatch");
    if(!results[k].empty()) require(res.moves==results[k][0].moves,"movement mismatch");
    results[k].push_back(res);
  }
  U checksum=0;
  for(ll x:expected) checksum=checksum*1000003ULL+static_cast<U>(x);
  std::cout << (Inversions?"Inversions (BIT)":"Distinct values") << '\n';
  std::cout << std::left << std::setw(10) << "Order" << std::right
            << std::setw(12) << "Reg ms" << std::setw(12) << "Run ms"
            << std::setw(12) << "Total ms" << std::setw(12) << "Min ms"
            << std::setw(12) << "Max ms" << std::setw(15) << "Moves" << '\n';
  for(int k=0;k<2;++k){
    std::vector<double> reg,run,total;
    for(auto t:results[k]){reg.push_back(t.registration); run.push_back(t.run); total.push_back(t.total);}
    std::cout << std::left << std::setw(10) << names[k] << std::right
              << std::setw(12) << median(reg) << std::setw(12) << median(run)
              << std::setw(12) << median(total)
              << std::setw(12) << *std::min_element(total.begin(),total.end())
              << std::setw(12) << *std::max_element(total.begin(),total.end())
              << std::setw(15) << results[k][0].moves << '\n';
  }
  std::cout << "Answer checksum=" << checksum << " [PASS]\n" << std::flush;
}
} // namespace mo_benchmark

int main(){
  using namespace mo_benchmark;
  std::mt19937_64 rng(SEED);
  constexpr ll N=100'000,Q=100'000;
  std::vector<ll> a(N);
  for(auto& x:a) x=rng()%K;
  std::cout << std::fixed << std::setprecision(3);
  std::cout << "Mo benchmark: seed=" << SEED << ", K=" << K << ", repeats=" << REPEATS << '\n';
  self_test();
  for(std::string_view type:{"uniform","short","long","clustered","duplicates"}){
    auto ranges=make_queries(N,Q,type,rng);
    std::cout << '\n' << type << " N=" << N << " Q=" << Q << '\n';
    bench<false>(a,ranges);
    bench<true>(a,ranges);
  }
  std::cout << "[SUCCESS] All answer vectors matched.\n";
}
