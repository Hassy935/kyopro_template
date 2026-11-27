#include <Library/math/bit_operation.hpp>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>

namespace bit_operation_test {
constexpr ll HI = std::numeric_limits<ll>::max();
constexpr ll DIGITS = std::numeric_limits<ll>::digits;
constexpr ull SEED = 0xB1702026ULL;
constexpr ll RANDOM_CASES = 100000;
static_assert(DIGITS == 63);

// NDEBUG に依存しない検証。
void check(bool ok, const char* label, ll x, ll actual, ll expected) {
  if (ok) return;
  std::cerr << "[FAIL] " << label << " x=" << x
            << " actual=" << actual << " expected=" << expected
            << " seed=" << SEED << '\n';
  std::exit(1);
}

// 標準ビット演算関数に依存しない参照実装。
ll popcount_ref(ll x) {
  ll res = 0;
  while (x > 0) {
    res += x%2;
    x /= 2;
  }
  return res;
}

void verify(ll x) {
  ll actual_pop = popcountll(x);
  ll expected_pop = popcount_ref(x);
  check(actual_pop == expected_pop, "popcountll", x, actual_pop, expected_pop);

  if (x == 0) return; // log2 の事前条件は x > 0。

  // 2 の冪を順に生成する。2^63 も ull なら表現可能。
  ull p = 1;
  ll f = 0;
  while (p*2 <= static_cast<ull>(x)) {
    p *= 2;
    ++f;
  }
  ll c = f + (p != static_cast<ull>(x));
  ll actual_f = floor_log2(x);
  ll actual_c = ceil_log2(x);
  check(actual_f == f, "floor_log2", x, actual_f, f);
  check(actual_c == c, "ceil_log2", x, actual_c, c);
}

void boundaries() {
  // 小さい整数を全探索。
  for (ll j = 0; j <= 65535; ++j) verify(j);

  // 全ビット位置、2 の冪の前後、全ビットが 1 の値。
  for (ll j = 0; j < DIGITS; ++j) {
    ll p = static_cast<ll>(1ULL << j);
    for (ll delta = -2; delta <= 2; ++delta) {
      if (p+delta >= 0) verify(p+delta);
    }
    verify(HI ^ p);
    verify(HI ^ (p-1));

    // 2 ビットのみ 1、または 2 ビットのみ 0。
    for (ll k = j+1; k < DIGITS; ++k) {
      ll mask = p | static_cast<ll>(1ULL << k);
      verify(mask);
      verify(HI ^ mask);
    }
  }

  for (ll j = 0; j <= 1024; ++j) verify(HI-j);
  verify(static_cast<ll>(0x5555555555555555ULL));
  verify(static_cast<ll>(0x2AAAAAAAAAAAAAAAULL));
}

void randomized() {
  std::mt19937_64 rng(SEED);
  std::uniform_int_distribution<ll> value_dist(0, HI);
  std::uniform_int_distribution<ll> bit_dist(0, DIGITS-1);

  for (ll j = 0; j < RANDOM_CASES; ++j) {
    ll x = value_dist(rng);
    verify(x);
    // 有効ビット幅を変え、小さい値にも十分な入力を割り当てる。
    verify(x >> bit_dist(rng));
  }
}
} // namespace bit_operation_test

int main() {
  bit_operation_test::boundaries();
  bit_operation_test::randomized();
  std::cout << "bit_operation: all tests passed (seed="
            << bit_operation_test::SEED << ", random iterations="
            << bit_operation_test::RANDOM_CASES << ")\n";
}
