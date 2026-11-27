#include <Library/math/combination.hpp>
#include <atcoder/modint>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <utility>
#include <vector>

namespace combination_benchmark {
using Clock = std::chrono::steady_clock;
constexpr ll N = 1000000;
constexpr ll Q = 2000000;
constexpr int REPEATS = 5;
constexpr ull SEED = 0xC0B1A710ULL;
constexpr ll MOD = 998244353;
using Query = std::pair<ll,ll>;

struct NaiveWorkload {
  ll k;
  std::vector<Query> queries;
};

struct Result {
  double ms;
  ull checksum;
  ll limit = -1;
};

// 測定中の出力を避け、計算結果はチェックサムとして使用する。
template<class F>
Result measure(F&& f) {
  auto start = Clock::now();
  ull checksum = f();
  auto end = Clock::now();
  return {std::chrono::duration<double,std::milli>(end-start).count(), checksum};
}

void fail(const char* message) {
  std::cerr << "[FAIL] " << message << '\n';
  std::exit(1);
}

// 同じ入力を複数回測定し、中央値を表示する。
template<class F>
Result report(const char* label, ll operations, F&& f) {
  std::vector<double> times;
  Result first{};
  for (int rep_id=0; rep_id<REPEATS; ++rep_id) {
    Result result = f();
    if (rep_id==0) first = result;
    else if (result.checksum!=first.checksum || result.limit!=first.limit)
      fail("inconsistent results across repetitions");
    times.push_back(result.ms);
  }
  std::sort(times.begin(),times.end());
  double median = times[times.size()/2];
  std::cout << std::left << std::setw(28) << label << std::right
            << " median=" << std::setw(10) << median << " ms"
            << "  min=" << std::setw(10) << times.front() << " ms"
            << "  max=" << std::setw(10) << times.back() << " ms";
  if (operations>0)
    std::cout << "  " << std::setw(10) << median*1e6/operations << " ns/op";
  if (first.limit>=0) std::cout << "  limit=" << first.limit;
  std::cout << "  checksum=" << first.checksum << '\n';
  first.ms = median;
  return first;
}

// 構築と逐次拡張では、破棄の時間を測定から除く。
template<class Mint>
std::vector<ull> run(const char* label, const std::vector<Query>& queries,
                     const std::vector<NaiveWorkload>& naive) {
  std::cout << "\n=== " << label << " (mod=" << Mint::mod() << ") ===\n";
  std::vector<ull> checksums;

  auto fixed = report("Bulk precompute", 0, [] {
    std::optional<Combination<Mint>> comb;
    auto start = Clock::now();
    comb.emplace(N);
    auto end = Clock::now();
    // 計算結果を測定後にも利用する。
    ull checksum = (ull)comb->fact(N).val() + comb->inv_fact(N).val();
    return Result{std::chrono::duration<double,std::milli>(end-start).count(),
                  checksum, comb->limit()};
  });
  checksums.push_back(fixed.checksum);

  auto growth = report("Sequential fact / growth", N, [] {
    std::optional<Combination<Mint>> comb;
    ull sum = 0;
    auto start = Clock::now();
    comb.emplace();
    for (ll n=1; n<=N; ++n) sum += comb->fact(n).val();
    auto end = Clock::now();
    return Result{std::chrono::duration<double,std::milli>(end-start).count(),
                  sum, comb->limit()};
  });
  checksums.push_back(growth.checksum);

  // 以降の測定には事前計算を含めない。
  Combination<Mint> comb(N);
  ull expected_growth = 0;
  for (ll n=1; n<=N; ++n) expected_growth += comb.fact(n).val();
  if (growth.checksum!=expected_growth) fail("growth checksum mismatch");
  if (fixed.limit!=N || growth.limit<N) fail("unexpected precomputation limit");

  auto choose = report("C (precomputed)", Q, [&] {
    return measure([&] {
      ull sum = 0;
      for (auto [n,k]:queries) sum += comb.C(n,k).val();
      return sum;
    });
  });
  checksums.push_back(choose.checksum);

  auto perm = report("P (precomputed)", Q, [&] {
    return measure([&] {
      ull sum = 0;
      for (auto [n,k]:queries) sum += comb.P(n,k).val();
      return sum;
    });
  });
  checksums.push_back(perm.checksum);

  for (const auto& work:naive) {
    std::cout << "C_naive: k=" << work.k << ", Q=" << work.queries.size() << '\n';
    auto result = report("C_naive (precomputed)", (ll)work.queries.size(), [&] {
      return measure([&] {
        ull sum = 0;
        for (auto [n,k]:work.queries) sum += comb.C_naive(n,k).val();
        return sum;
      });
    });
    checksums.push_back(result.checksum);
  }
  if (comb.limit()!=N) fail("unexpected growth during precomputed queries");
  return checksums;
}
} // namespace combination_benchmark

int main() {
  using namespace combination_benchmark;
  std::mt19937_64 rng(SEED);
  std::vector<Query> queries;
  queries.reserve(Q);
  for (ll j=0; j<Q; ++j) {
    ll n = std::uniform_int_distribution<ll>(0,N)(rng);
    ll k = std::uniform_int_distribution<ll>(0,n)(rng);
    queries.emplace_back(n,k);
  }

  // 各ケースで分子の積を合計200万回計算する。
  std::vector<NaiveWorkload> naive;
  for (ll k:{10LL,1000LL,100000LL}) {
    NaiveWorkload work{k,{}};
    ll count = 2000000/k;
    work.queries.reserve(count);
    for (ll j=0; j<count; ++j) {
      ll n = std::uniform_int_distribution<ll>(1000000000000000LL,
                                               1000000000000000000LL)(rng);
      work.queries.emplace_back(n,k);
    }
    naive.push_back(std::move(work));
  }

  std::cout << std::fixed << std::setprecision(3);
  std::cout << "Combination benchmark: N=" << N << ", Q=" << Q
            << ", repeats=" << REPEATS << ", seed=" << SEED << '\n';
  using StaticMint = atcoder::modint998244353;
  using DynamicMint = atcoder::dynamic_modint<0>;
  DynamicMint::set_mod(MOD);
  auto a = run<StaticMint>("static_modint",queries,naive);
  auto b = run<DynamicMint>("dynamic_modint",queries,naive);
  if (a!=b) fail("static / dynamic checksum mismatch");
  std::cout << "\n[SUCCESS] All checksums matched.\n";
}
