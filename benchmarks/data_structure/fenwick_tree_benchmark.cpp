#include <Library/data_structure/fenwick_tree.hpp>

namespace {
using Clock = chrono::steady_clock;
constexpr int Q = 1'000'000;
constexpr int trials = 5;
constexpr ull seed = 0xF311C2026ULL;

struct Query {
  int p, l, r, prefix;
  ll x, target;
};

struct Sample {
  double ms;
  ll checksum;
};

[[noreturn]] void fail(const char* label, ll n, int trial, ll p = -1){
  cerr << "[FAIL] " << label << " N=" << n << " trial=" << trial
       << " p=" << p << " seed=" << seed << '\n';
  exit(1);
}

ll verify(const FenwickTree<ll>& tree, const vector<ll>& expected,
          const char* label, int trial){
  const ll n = ssize(expected);
  if(tree.size() != n) fail(label,n,trial);
  ll checksum = 0;
  for(ll p=0;p<n;p++){
    const ll actual = tree.get(p);
    if(actual != expected[p]) fail(label,n,trial,p);
    checksum += actual;
  }
  return checksum;
}

template<class Measure>
void report(const char* label, int n, Measure measure){
  array<double,trials> times{};
  ll checksum = 0;
  for(int trial=0;trial<trials;trial++){
    const auto sample = measure(trial);
    if(trial > 0 && checksum != sample.checksum) fail(label,n,trial);
    checksum = sample.checksum;
    times[trial] = sample.ms;
  }
  sort(times.begin(),times.end());
  cout << left << setw(16) << label << right
       << setw(11) << times[trials/2] << setw(11) << times.front()
       << setw(11) << times.back() << setw(19) << checksum << '\n';
}

void benchmark(int n){
  mt19937_64 rng(seed+n);
  uniform_int_distribution<ll> value(0,99);
  uniform_int_distribution<int> index(0,n-1), endpoint(0,n);
  vector<ll> initial(n), zero(n);
  for(ll& x:initial) x = value(rng);
  const ll total = accumulate(initial.begin(),initial.end(),0LL);
  uniform_int_distribution<ll> target(0,total+1);

  vector<Query> queries(Q);
  auto add_state = initial, set_state = initial, mixed_state = initial;
  for(int i=0;i<Q;i++){
    auto& [p,l,r,prefix,x,t] = queries[i];
    p = index(rng);
    l = endpoint(rng);
    r = endpoint(rng);
    if(l > r) swap(l,r);
    prefix = endpoint(rng);
    x = value(rng);
    t = target(rng);
    add_state[p] += x;
    set_state[p] = x;
    if(i%2 == 0) mixed_state[p] += x;
  }

  cout << "\nN=" << n << '\n';
  cout << left << setw(16) << "operation" << right
       << setw(11) << "median ms" << setw(11) << "min ms"
       << setw(11) << "max ms" << setw(19) << "checksum" << '\n';

  auto construction = [&](const char* label, auto build, const vector<ll>& expected){
    report(label,n,[&](int trial){
      const auto start = Clock::now();
      auto tree = build();
      asm volatile("" : : "g"(&tree) : "memory");
      const auto finish = Clock::now();
      return Sample{chrono::duration<double,milli>(finish-start).count(),
                    verify(tree,expected,label,trial)};
    });
  };
  construction("zero",[&]{ return FenwickTree<ll>(n); },zero);
  construction("build",[&]{ return FenwickTree<ll>(initial); },initial);

  auto operations = [&](const char* label, auto op, const vector<ll>* expected = nullptr){
    report(label,n,[&](int trial){
      FenwickTree<ll> tree(initial);
      const auto start = Clock::now();
      ll checksum = 0;
      for(int i=0;i<Q;i++) checksum += op(tree,queries[i],i);
      asm volatile("" : : "g"(&tree), "g"(checksum) : "memory");
      const auto finish = Clock::now();
      if(expected) checksum += verify(tree,*expected,label,trial);
      return Sample{chrono::duration<double,milli>(finish-start).count(),checksum};
    });
  };

  operations("add",[](auto& t, const Query& q, int){
    t.add(q.p,q.x);
    return 0LL;
  },&add_state);
  operations("get",[](auto& t, const Query& q, int){ return t.get(q.p); });
  operations("set",[](auto& t, const Query& q, int){
    t.set(q.p,q.x);
    return 0LL;
  },&set_state);
  operations("sum(r)",[](auto& t, const Query& q, int){ return t.sum(q.prefix); });
  operations("sum(l,r)",[](auto& t, const Query& q, int){ return t.sum(q.l,q.r); });
  operations("lower_bound",[](auto& t, const Query& q, int){ return t.lower_bound(q.target); });
  operations("upper_bound",[](auto& t, const Query& q, int){ return t.upper_bound(q.target); });
  operations("add + sum",[](auto& t, const Query& q, int i){
    if(i%2 == 0){
      t.add(q.p,q.x);
      return 0LL;
    }
    return t.sum(q.l,q.r);
  },&mixed_state);
}
}

int main(){
  cout << fixed << setprecision(3);
  cout << "FenwickTree<ll>: Q=" << Q << " trials=" << trials
       << " seed=" << seed << '\n';
  for(int n:{200'000,1'000'000}) benchmark(n);
  cout << "\n[PASS] Checksums and final states matched.\n";
}
