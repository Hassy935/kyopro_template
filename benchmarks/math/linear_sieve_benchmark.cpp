#include <Library/math/linear_sieve.hpp>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <optional>
#include <random>
#include <vector>

namespace sieve_benchmark {
using Clock = std::chrono::steady_clock;
using U = unsigned long long;
constexpr ll Q = 2'000'000;
constexpr int REPEATS = 5;
constexpr U SEED = 0x51E7EULL;

struct Result { double ms; U checksum; };

double elapsed(Clock::time_point start) {
  return std::chrono::duration<double,std::milli>(Clock::now()-start).count();
}

[[noreturn]] void fail(const char* message) {
  std::cerr << "[FAIL] " << message << '\n';
  std::exit(1);
}

template<class F>
Result measure(F&& f) {
  auto start=Clock::now();
  U sum=f();
  return {elapsed(start),sum};
}

template<class F>
U report(const char* label,ll operations,F&& f) {
  std::vector<double> times;
  U expected=0;
  for(int r=0;r<REPEATS;++r){
    auto result=f();
    if(r==0) expected=result.checksum;
    else if(expected!=result.checksum) fail(label);
    times.push_back(result.ms);
  }
  std::sort(times.begin(),times.end());
  double med=times[REPEATS/2];
  std::cout << std::left << std::setw(26) << label << std::right
            << " median=" << std::setw(10) << med << " ms"
            << "  min=" << std::setw(10) << times.front() << " ms"
            << "  max=" << std::setw(10) << times.back() << " ms";
  if(operations>0) std::cout << "  " << std::setw(10) << med*1e6/operations << " ns/op";
  std::cout << "  checksum=" << expected << '\n';
  return expected;
}

std::vector<ll> queries(ll n) {
  std::mt19937_64 rng(SEED);
  std::uniform_int_distribution<ll> dist(0,n);
  std::vector<ll> xs(Q);
  for(auto& x:xs) x=dist(rng);
  return xs;
}

void run(ll n,ll expected_count,U expected_sum) {
  const auto xs=queries(n);
  std::cout << "\nN=" << n << " Q=" << Q << '\n';
  auto built=report("Build",0,[&]{
    std::optional<LinearSieve> s;
    auto start=Clock::now();
    s.emplace(n);
    double ms=elapsed(start);
    // 構築結果の検証と破棄は測定区間外。
    if(s->limit()!=n || s->is_prime(0) || s->is_prime(1) || !s->is_prime(2))
      fail("build boundary");
    return Result{ms,(U)s->count_primes()};
  });
  if(built!=(U)expected_count) fail("prime count after build");

  const LinearSieve s(n);
  report("is_prime",Q,[&]{
    return measure([&]{
      U sum=0;
      for(ll x:xs) sum+=s.is_prime(x);
      return sum;
    });
  });

  // GCC/Clang のコンパイラバリアで、不変な O(1) 取得のループ外移動を抑止。
  // 実測値にはループ・チェックサムのコストも含まれる。
  auto listed=report("get_primes (ref)",Q,[&]{
    return measure([&]{
      U sum=0;
      for(ll i=0;i<Q;++i){
        asm volatile("" : : "g"(&s) : "memory");
        const auto& ps=s.get_primes();
        sum+=ps.size();
      }
      return sum;
    });
  });
  if(listed!=(U)expected_count*Q) fail("get_primes size");
  auto prime_sum=report("get_primes + sum",0,[&]{
    return measure([&]{
      const auto& ps=s.get_primes();
      return std::accumulate(ps.begin(),ps.end(),U{0});
    });
  });
  if(prime_sum!=expected_sum) fail("prime sum");
  auto count=report("count_primes",Q,[&]{
    return measure([&]{
      U sum=0;
      for(ll i=0;i<Q;++i){
        asm volatile("" : : "g"(&s) : "memory");
        sum+=s.count_primes();
      }
      return sum;
    });
  });
  if(count!=(U)expected_count*Q) fail("count_primes");

  constexpr ll FACT_Q=200'000;
  // 乱数生成と有効範囲への調整を測定区間外で行う。
  std::mt19937_64 rng(SEED+1);
  std::vector<ll> spf_queries(Q), factor_queries(FACT_Q);
  for(auto& x:spf_queries) x=std::uniform_int_distribution<ll>(2,n)(rng);
  for(auto& x:factor_queries) x=std::uniform_int_distribution<ll>(1,n)(rng);
  report("get_spf",Q,[&]{
    return measure([&]{
      U sum=0;
      for(ll x:spf_queries) sum+=s.get_spf(x);
      return sum;
    });
  });
  report("factorize + checksum",FACT_Q,[&]{
    return measure([&]{
      U sum=0;
      for(ll x:factor_queries){
        auto fs=s.factorize(x);
        for(auto [p,e]:fs) sum+=(U)p*e;
      }
      return sum;
    });
  });
  U enumerated=0;
  report("divisors + checksum",FACT_Q,[&]{
    auto start=Clock::now();
    U sum=0, num=0;
    for(ll x:factor_queries){
      auto ds=s.divisors(x);
      num+=ds.size();
      for(ll d:ds) sum+=d;
    }
    double ms=elapsed(start);
    enumerated=num;
    return Result{ms,sum};
  });
  auto counted=report("count_divisors",FACT_Q,[&]{
    return measure([&]{
      U sum=0;
      for(ll x:factor_queries) sum+=s.count_divisors(x);
      return sum;
    });
  });
  if(counted!=enumerated) fail("divisor count mismatch");
  std::cout << "Factor/divisor queries=" << FACT_Q << "  enumerated divisors=" << enumerated << '\n';
}
} // namespace sieve_benchmark

int main(){
  using namespace sieve_benchmark;
  std::cout << std::fixed << std::setprecision(3);
  std::cout << "LinearSieve benchmark: repeats=" << REPEATS << ", seed=" << SEED << '\n';
  run(1'000'000,78'498,37'550'402'023ULL);
  run(10'000'000,664'579,3'203'324'994'356ULL);
  std::cout << "\n[SUCCESS] All checksum checks passed.\n";
}
