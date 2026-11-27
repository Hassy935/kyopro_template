#include <Library/graph/dijkstra.hpp>

namespace {
using Clock = chrono::steady_clock;
constexpr int trials=5;

void check(bool ok){
  if(ok) return;
  cerr << "[FAIL] Result mismatch\n";
  exit(1);
}

template<class Run,class Verify>
double measure(Run run,Verify verify){
  const auto start=Clock::now();
  const auto result=run();
  asm volatile("" : : "g"(&result) : "memory");
  const auto finish=Clock::now();
  check(verify(result));
  return chrono::duration<double,micro>(finish-start).count();
}

void print(const string& label,array<double,trials> times){
  sort(times.begin(),times.end());
  cout << label << '\t' << times[trials/2] << '\t'
       << times.front() << '\t' << times.back() << '\n';
}

template<class Run,class Verify>
void report(const string& label,Run run,Verify verify){
  array<double,trials> times;
  for(double& elapsed:times) elapsed=measure(run,verify);
  print(label,times);
}

template<class First,class Second>
void report_pair(const string& prefix,First single,Second with_path){
  array<double,trials> a,b;
  for(int trial=0;trial<trials;trial++){
    if(trial%2==0){
      a[trial]=single();
      b[trial]=with_path();
    }else{
      b[trial]=with_path();
      a[trial]=single();
    }
  }
  print(prefix+"single",a);
  print(prefix+"with_path",b);
}

vl make_sources(ll n,ll k){
  vl sources;
  sources.reserve(k);
  const ll distinct=min(k,n-1000);
  for(ll i=0;i<k;i++) sources.push_back((i%distinct)*n/distinct);
  cout << "# sources K=" << k << " S=" << distinct << '\n';
  return sources;
}

using Graph = vvc<pair<ll,ll>>;
constexpr ull seed=0xD1A57AA26ULL;

void stale_candidates(){
  constexpr ll n=200'000, far=n-2;
  Graph G(n);
  // 星形の長い候補を，重み 1 のパスで順に改善する．
  for(ll v=1;v<=far;v++){
    G[0].emplace_back(v,2*n);
    G[v-1].emplace_back(v,1);
  }
  // 頂点 0 の隣接リストでも，直接辺の候補を先に登録する．
  swap(G[0][1],G[0].back());
  cout << "# stale N=" << n << " M=" << 2*far << " stale_from_0=" << far << '\n';

  vl expected(n,-1);
  iota(expected.begin(),expected.end()-1,0LL);
  auto equal=[&](const vl& result){ return result==expected; };
  report_pair("stale dijkstra ",
    [&]{ return measure([&]{ return dijkstra(G,0LL); },equal); },
    [&]{
      return measure([&]{ return dijkstra_with_path(G,0LL); },[&](const auto& result){
        return equal(result.first) && ssize(result.second)==n && result.second[far]==far-1;
      });
    });
  for(ll k:{1LL,10LL,1000LL,n}){
    const vl starts=make_sources(n,k);
    vl nearest(n,-1);
    for(ll s:starts) nearest[s]=0;
    for(ll v=1;v<=far;v++)
      if(nearest[v]<0) nearest[v]=nearest[v-1]+1;
    report("stale dijkstra multi "+to_string(k),[&]{ return dijkstra(G,starts); },
           [&](const vl& result){ return result==nearest; });
  }

  const array<ll,4> targets={0,8,far,n-1};
  const array<const char*,4> names={"self","near","far","unreachable"};
  for(size_t i=0;i<targets.size();i++){
    const ll t=targets[i], distance=expected[t];
    cout << "# target " << names[i] << " vertex=" << t << " distance=" << distance << '\n';
    report(string("stale dijkstra target ")+names[i],[&]{ return dijkstra(G,0LL,t); },
           [&](ll d){ return d==distance; });
  }
}

void random_cases(){
  constexpr ll n=100'000;
  mt19937_64 rng(seed);
  uniform_int_distribution<ll> vertex(0,n-1),weight(0,1000);
  Graph G(n);
  for(ll v=0;v<n;v++) G[v].emplace_back((v+1)%n,weight(rng));
  for(ll degree:{3LL,10LL}){
    const ll previous=degree==3 ? n : 3*n;
    for(ll i=previous;i<degree*n;i++){
      const ll u=vertex(rng),v=vertex(rng),cost=weight(rng);
      G[u].emplace_back(v,cost);
    }
    cout << "# random N=" << n << " M=" << degree*n << " weight=0..1000 seed=" << seed << '\n';
    const auto [expected,par]=dijkstra_with_path(G,0LL);
    check(all_of(expected.begin(),expected.end(),[](ll d){ return d>=0; }));
    const string prefix="random "+to_string(degree)+"N dijkstra ";
    report_pair(prefix,
      [&]{
        return measure([&]{ return dijkstra(G,0LL); },
                       [&](const vl& result){ return result==expected; });
      },
      [&]{
        return measure([&]{ return dijkstra_with_path(G,0LL); },
                       [&](const auto& result){ return result.first==expected && result.second==par; });
      });
  }
}
}

int main(){
  cout << fixed << setprecision(6) << "operation\tmedian_us\tmin_us\tmax_us\n";
  stale_candidates();
  random_cases();
  cout << "[PASS] All Dijkstra results matched.\n";
}
