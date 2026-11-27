#include <Library/graph/bfs01.hpp>
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
constexpr ull seed=0xB05012026ULL;

void split_path(){
  constexpr ll n=200'000, half=n/2;
  const array<ll,4> targets={0,8,half-1,n-1};
  const array<const char*,4> names={"self","near","far","unreachable"};
  Graph G(n);
  vl expected(n,-1);
  expected[0]=0;
  for(ll v=1;v<n;v++) if(v!=half){
    const ll cost=v%2;
    G[v-1].emplace_back(v,cost);
    G[v].emplace_back(v-1,cost);
    if(v<half) expected[v]=expected[v-1]+cost;
  }
  cout << "# split_path N=" << n << " M=" << 2*(n-2) << '\n';
  auto equal=[&](const vl& result){ return result==expected; };
  report_pair("path bfs01 ",
    [&]{ return measure([&]{ return bfs01(G,0LL); },equal); },
    [&]{
      return measure([&]{ return bfs01_with_path(G,0LL); },[&](const auto& result){
        return equal(result.first) && ssize(result.second)==n && result.second[half-1]==half-2;
      });
    });
  for(ll k:{1LL,10LL,1000LL,n}){
    const vl starts=make_sources(n,k);
    vl nearest(n,n);
    for(ll s:starts) nearest[s]=0;
    for(ll base:{0LL,half}){
      for(ll v=base+1;v<base+half;v++) nearest[v]=min(nearest[v],nearest[v-1]+v%2);
      for(ll v=base+half-1;v>base;v--) nearest[v-1]=min(nearest[v-1],nearest[v]+v%2);
    }
    for(ll& d:nearest) if(d==n) d=-1;
    report("path bfs01 multi "+to_string(k),[&]{ return bfs01(G,starts); },
           [&](const vl& result){ return result==nearest; });
  }
  for(size_t i=0;i<targets.size();i++){
    const ll t=targets[i], distance=expected[t];
    cout << "# target " << names[i] << " vertex=" << t << " distance=" << distance << '\n';
    report(string("path bfs01 target ")+names[i],[&]{ return bfs01(G,0LL,t); },
           [&](ll d){ return d==distance; });
  }
}

void distributions(){
  constexpr ll n=100'000,m=3*n;
  mt19937_64 rng(seed);
  uniform_int_distribution<ll> vertex(0,n-1);
  uniform_int_distribution<int> ticket(0,99);
  vector<tuple<ll,ll,int>> edges;
  edges.reserve(m);
  for(ll v=0;v<n;v++) edges.emplace_back(v,(v+1)%n,ticket(rng));
  for(ll i=n;i<m;i++){
    const ll u=vertex(rng),v=vertex(rng);
    edges.emplace_back(u,v,ticket(rng));
  }

  for(int percent:{0,10,50,90,100}){
    Graph G(n);
    ll zero_count=0;
    for(auto [u,v,draw]:edges){
      const ll cost=draw>=percent;
      G[u].emplace_back(v,cost);
      zero_count+=cost==0;
    }
    cout << "# random N=" << n << " M=" << m << " zero=" << zero_count
         << " one=" << m-zero_count << " seed=" << seed << '\n';
    const vl expected=dijkstra(G,0LL);
    report("random bfs01 p0="+to_string(percent),[&]{ return bfs01(G,0LL); },
           [&](const vl& result){ return result==expected; });
  }
}
}

int main(){
  cout << fixed << setprecision(6) << "operation\tmedian_us\tmin_us\tmax_us\n";
  split_path();
  distributions();
  cout << "[PASS] All 0-1 BFS results matched.\n";
}
