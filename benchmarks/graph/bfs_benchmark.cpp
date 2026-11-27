#include <Library/graph/bfs.hpp>
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

vl make_sources(ll n,ll k){
  vl sources;
  sources.reserve(k);
  const ll distinct=min(k,n-1000);
  for(ll i=0;i<k;i++) sources.push_back((i%distinct)*n/distinct);
  cout << "# sources K=" << k << " S=" << distinct << '\n';
  return sources;
}

using WeightedGraph = vvc<pair<ll,ll>>;
constexpr ull seed=0x5A071E57ULL;

struct Timing {
  string label;
  function<double()> sample;
};

template<size_t Count>
void compare(const array<Timing,Count>& group){
  array<array<double,trials>,Count> times;
  // 試行ごとに最初に測る対象を交代する．
  for(int trial=0;trial<trials;trial++)
    for(size_t i=0;i<Count;i++){
      const size_t id=(trial+i)%Count;
      times[id][trial]=group[id].sample();
    }
  for(size_t i=0;i<Count;i++) print(group[i].label,times[i]);
}

double reach_self(const vvl& G){
  constexpr int repeat=20'000;
  int successes=0;
  const auto start=Clock::now();
  for(int i=0;i<repeat;i++){
    asm volatile("" : : "g"(&G) : "memory");
    const bool found=is_reachable(G,0,0);
    asm volatile("" : : "g"(found) : "memory");
    successes+=found;
  }
  const auto finish=Clock::now();
  check(successes==repeat);
  return chrono::duration<double,micro>(finish-start).count()/repeat;
}

void split_path(){
  constexpr ll n=200'000, half=n/2;
  const array<ll,4> targets={0,8,half-1,n-1};
  const array<const char*,4> names={"self","near","far","unreachable"};
  vvl G(n);
  for(ll v=1;v<n;v++) if(v!=half){
    G[v-1].push_back(v);
    G[v].push_back(v-1);
  }
  cout << "# split_path N=" << n << " M=" << 2*(n-2) << '\n';
  vl expected(n,-1);
  iota(expected.begin(),expected.begin()+half,0LL);
  auto equal=[&](const vl& result){ return result==expected; };
  compare(array{
    Timing{"path bfs single",[&]{ return measure([&]{ return bfs(G,0LL); },equal); }},
    Timing{"path bfs with_path",[&]{
      return measure([&]{ return bfs_with_path(G,0LL); },[&](const auto& result){
        return equal(result.first) && ssize(result.second)==n && result.second[half-1]==half-2;
      });
    }}
  });

  for(ll k:{1LL,10LL,1000LL,n}){
    const vl starts=make_sources(n,k);
    vl nearest(n,n);
    for(ll s:starts) nearest[s]=0;
    for(ll base:{0LL,half}){
      for(ll v=base+1;v<base+half;v++) nearest[v]=min(nearest[v],nearest[v-1]+1);
      for(ll v=base+half-1;v>base;v--) nearest[v-1]=min(nearest[v-1],nearest[v]+1);
    }
    for(ll& d:nearest) if(d==n) d=-1;
    report("path bfs multi "+to_string(k),[&]{ return bfs(G,starts); },
           [&](const vl& result){ return result==nearest; });
  }

  for(size_t i=0;i<targets.size();i++){
    const ll t=targets[i], distance=expected[t];
    cout << "# target " << names[i] << " vertex=" << t << " distance=" << distance << '\n';
    compare(array{
      Timing{string("path bfs target ")+names[i],[&]{
        return measure([&]{ return bfs(G,0LL,t); },[&](ll d){ return d==distance; });
      }},
      Timing{string("path reachable ")+names[i],[&]{
        if(t==0) return reach_self(G);
        return measure([&]{ return is_reachable(G,0,t); },[&](bool b){ return b==(distance>=0); });
      }}
    });
  }
}

void grid_comparison(){
  constexpr ll rows=400,cols=500,n=rows*cols;
  vvl G(n);
  for(ll r=0;r<rows;r++) for(ll c=0;c<cols;c++){
    const ll v=r*cols+c;
    if(r+1<rows){ G[v].push_back(v+cols); G[v+cols].push_back(v); }
    if(c+1<cols){ G[v].push_back(v+1); G[v+1].push_back(v); }
  }
  constexpr ll m=2*((rows-1)*cols+rows*(cols-1));
  cout << "# grid N=" << n << " M=" << m << " source=0 target=" << n-1 << '\n';
  mt19937_64 rng(seed);
  WeightedGraph unit(n),binary(n);
  vl expected(n);
  ll zero_count=0;
  for(ll v=0;v<n;v++){
    expected[v]=v/cols+v%cols;
    for(ll to:G[v]){
      unit[v].emplace_back(to,1);
      const ll cost=ll(rng()%2);
      binary[v].emplace_back(to,cost);
      zero_count+=cost==0;
    }
  }

  cout << "# grid binary zero=" << zero_count << " one=" << m-zero_count << '\n';
  auto unit_dist=[&](const vl& dist){ return dist==expected; };
  compare(array{
    Timing{"unit bfs all",[&]{ return measure([&]{ return bfs(G,0LL); },unit_dist); }},
    Timing{"unit bfs01 all",[&]{ return measure([&]{ return bfs01(unit,0LL); },unit_dist); }},
    Timing{"unit dijkstra all",[&]{ return measure([&]{ return dijkstra(unit,0LL); },unit_dist); }}
  });
  auto unit_target=[&](ll d){ return d==expected.back(); };
  compare(array{
    Timing{"unit bfs target",[&]{ return measure([&]{ return bfs(G,0LL,n-1); },unit_target); }},
    Timing{"unit bfs01 target",[&]{ return measure([&]{ return bfs01(unit,0LL,n-1); },unit_target); }},
    Timing{"unit dijkstra target",[&]{ return measure([&]{ return dijkstra(unit,0LL,n-1); },unit_target); }}
  });

  const vl binary_ref=dijkstra(binary,0LL);
  auto binary_dist=[&](const vl& dist){ return dist==binary_ref; };
  compare(array{
    Timing{"binary bfs01 all",[&]{ return measure([&]{ return bfs01(binary,0LL); },binary_dist); }},
    Timing{"binary dijkstra all",[&]{ return measure([&]{ return dijkstra(binary,0LL); },binary_dist); }}
  });
  auto binary_target=[&](ll d){ return d==binary_ref.back(); };
  compare(array{
    Timing{"binary bfs01 target",[&]{ return measure([&]{ return bfs01(binary,0LL,n-1); },binary_target); }},
    Timing{"binary dijkstra target",[&]{ return measure([&]{ return dijkstra(binary,0LL,n-1); },binary_target); }}
  });
  cout << "# grid target distances unit=" << expected.back()
       << " binary=" << binary_ref.back() << " seed=" << seed << '\n';
}
}

int main(){
  cout << fixed << setprecision(6) << "operation\tmedian_us\tmin_us\tmax_us\n";
  split_path();
  grid_comparison();
  cout << "[PASS] All BFS and cross-algorithm results matched.\n";
}
