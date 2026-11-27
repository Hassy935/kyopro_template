#include <Library/utility/compress.hpp>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace compress_test {
constexpr unsigned long long SEED = 0xC09E5526ULL;
constexpr ll RANDOM_CASES = 2000;
constexpr ll LO = std::numeric_limits<ll>::min();
constexpr ll HI = std::numeric_limits<ll>::max();
ll case_id = 0;
const char* phase = "initial";

// NDEBUG に依存しないチェック。
void check(bool ok, const char* label) {
  if (ok) return;
  std::cerr << "[FAIL] " << label << " phase=" << phase
            << " case=" << case_id << " seed=" << SEED << '\n';
  std::exit(1);
}

// std::set によりソート済みの異なる値を独立に構築する。
// 検索の参照値は二分探索を使わず、線形走査で求める。
template<class T>
void verify(const compress<T>& cp, const std::vector<T>& input,
            const std::vector<T>& queries) {
  const std::set<T> ref(input.begin(), input.end());
  const std::vector<T> expected(ref.begin(), ref.end());
  check(cp.xs == expected, "xs");
  check(cp.size() == std::ssize(expected), "size");
  check(cp.empty() == expected.empty(), "empty");

  for (ll j = 0; j < std::ssize(expected); ++j) {
    const T value = expected[j];
    check(cp[j] == value, "operator[]");
    check(cp.get(value) == j, "get");
    check(cp(value) == j, "operator()");
    check(cp[cp(value)] == value, "round trip");
    check(cp.contains(value), "contains registered");
    check(cp.lower_bound_index(value) == j, "lower registered");
    check(cp.upper_bound_index(value) == j+1, "upper registered");
  }

  for (const T& value : queries) {
    ll low = 0, high = 0;
    bool found = false;
    for (const T& elem : ref) {
      low += elem < value;
      high += !(value < elem);
      found = found || elem == value;
    }
    check(cp.contains(value) == found, "contains query");
    check(cp.lower_bound_index(value) == low, "lower query");
    check(cp.upper_bound_index(value) == high, "upper query");
    if (found) {
      check(cp.get(value) == low, "get query");
      check(cp(value) == low, "operator() query");
    }
  }
}

template<class T>
void construction(const std::vector<T>& input, const std::vector<T>& queries) {
  compress<T> cp(input);
  verify(cp, input, queries);
  cp.build(); // 再構築は冪等。
  verify(cp, input, queries);
  compress<T> incremental;
  for (const T& value : input) incremental.add(value);
  incremental.build();
  verify(incremental, input, queries);
}

void boundaries() {
  phase = "boundaries";
  const std::vector<ll> queries = {LO, LO+1, -100, -2, -1, 0, 1, 2,
                                   5, 10, 20, 30, 40, HI-1, HI};
  construction<ll>({}, queries);
  construction<ll>({0}, queries);
  construction<ll>({HI}, queries);
  construction<ll>({LO}, queries);
  construction<ll>(std::vector<ll>(1000, -2), queries);
  construction<ll>({LO, HI, LO, 0, -1, 1, HI, HI-1, LO+1}, queries);
  construction<ll>({-2,-1,0,1,2}, queries);
  construction<ll>({2,1,0,-1,-2}, queries);

  compress<ll> cp;
  verify(cp, {}, queries);
  cp.add(10, 30, 20, 10);
  cp.build();
  verify(cp, {10,30,20,10}, queries);
  cp.add(5, 40, 20);
  cp.build();
  verify(cp, {10,30,20,10,5,40,20}, queries);
  cp.xs.push_back(LO);
  cp.xs.push_back(HI);
  cp.build();
  verify(cp, {10,30,20,5,40,LO,HI}, queries);
  cp.xs.clear();
  cp.build();
  verify(cp, {}, queries);
  cp.add(0);
  cp.build();
  verify(cp, {0}, queries);
}

void types() {
  phase = "types";
  construction<std::string>({"abc", "", "a", "aa", "abc", "z"},
                            {"", "a", "ab", "abc", "b", "zz"});
  using P = std::pair<ll,ll>;
  construction<P>({{1,2},{0,3},{1,1},{1,2},{LO,HI}},
                  {{0,0},{1,1},{1,3},{HI,HI},{LO,HI}});
  using U = std::tuple<ll,ll,ll>;
  construction<U>({{1,2,3},{1,2,2},{0,5,3},{1,2,3}},
                  {{0,0,0},{1,2,2},{1,2,4},{2,0,0}});
  construction<double>({-1.5,0.0,1.25,-1.5,3.75}, {-2.,0.,.5,1.25,4.});

  // vector<bool> の要素参照に対応していることを確認する。
  static_assert(std::is_same_v<decltype(std::declval<const compress<ll>&>()[0]), const ll&>);
  static_assert(std::is_same_v<decltype(std::declval<const compress<bool>&>()[0]), bool>,
                "compress<bool>::operator[] must return bool by value");
  construction<bool>({}, {false,true});
  construction<bool>({true,true}, {false,true});
  construction<bool>({false,false}, {false,true});
  construction<bool>({true,false,true,false}, {false,true});
  compress<bool> flags;
  flags.add(true, false, true);
  flags.build();
  verify(flags, {true,false,true}, {false,true});
}

void randomized() {
  phase = "randomized";
  std::mt19937_64 rng(SEED);
  std::uniform_int_distribution<ll> small(-30,30), wide(LO,HI), len(0,200);
  for (case_id = 0; case_id < RANDOM_CASES; ++case_id) {
    std::vector<ll> input, queries = {LO,HI,-1,0,1};
    const ll length = len(rng);
    for (ll j = 0; j < length; ++j)
      input.push_back(case_id%2 ? small(rng) : wide(rng));
    for (ll j = 0; j < 20; ++j) {
      queries.push_back(small(rng));
      queries.push_back(wide(rng));
    }
    compress<ll> cp(input);
    verify(cp, input, queries);
    for (ll j = 0; j < 3; ++j) {
      const ll x = small(rng), y = wide(rng), z = small(rng);
      cp.add(x,y,z);
      input.insert(input.end(), {x,y,z});
      cp.build();
      verify(cp, input, queries);
    }
  }
}
} // namespace compress_test

int main() {
  compress_test::boundaries();
  compress_test::types();
  compress_test::randomized();
  std::cout << "compress: all tests passed (seed=" << compress_test::SEED
            << ", random iterations=" << compress_test::RANDOM_CASES << ")\n";
}
