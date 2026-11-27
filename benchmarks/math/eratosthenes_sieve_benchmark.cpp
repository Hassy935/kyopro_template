#include <Library/math/eratosthenes_sieve.hpp>
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
    std::optional<EratosthenesSieve> s;
    auto start=Clock::now();
    s.emplace(n);
    double ms=elapsed(start);
    // 構築結果の検証と破棄は測定区間外。
    if(s->limit()!=n || s->is_prime(0) || s->is_prime(1) || !s->is_prime(2))
      fail("build boundary");
    return Result{ms,(U)s->count_primes()};
  });
  if(built!=(U)expected_count) fail("prime count after build");

  const EratosthenesSieve s(n);
  report("is_prime",Q,[&]{
    return measure([&]{
      U sum=0;
      for(ll x:xs) sum+=s.is_prime(x);
      return sum;
    });
  });

  auto listed=report("get_primes",0,[&]{
    auto start=Clock::now();
    auto ps=s.get_primes();
    double ms=elapsed(start);
    // 生成結果の走査・破棄は測定区間外。
    if((ll)ps.size()!=expected_count) fail("get_primes size");
    return Result{ms,std::accumulate(ps.begin(),ps.end(),U{0})};
  });
  if(listed!=expected_sum) fail("prime sum");
  auto count=report("count_primes",0,[&]{
    return measure([&]{return (U)s.count_primes();});
  });
  if(count!=(U)expected_count) fail("count_primes");
}
} // namespace sieve_benchmark

int main(){
  using namespace sieve_benchmark;
  std::cout << std::fixed << std::setprecision(3);
  std::cout << "EratosthenesSieve benchmark: repeats=" << REPEATS << ", seed=" << SEED << '\n';
  run(1'000'000,78'498,37'550'402'023ULL);
  run(10'000'000,664'579,3'203'324'994'356ULL);
  std::cout << "\n[SUCCESS] All checksum checks passed.\n";
}
