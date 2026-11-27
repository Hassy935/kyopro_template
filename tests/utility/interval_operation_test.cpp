#include <Library/utility/interval_operation.hpp>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <utility>
#include <vector>

namespace interval_operation_test {
constexpr ll LO = std::numeric_limits<ll>::min();
constexpr ll HI = std::numeric_limits<ll>::max();
constexpr ull SEED = 0xCA92026ULL;
constexpr ll RANDOM_CASES = 100000;

// NDEBUG に依存しない検証。
void check(bool ok, const char* label, ll l, ll r, ll nl, ll nr,
           const std::pair<ll,ll>& result) {
  if (ok) return;
  std::cerr << "[FAIL] " << label << " inputs=(" << l << ',' << r
            << "),(" << nl << ',' << nr << ") result=("
            << result.first << ',' << result.second << ") seed=" << SEED << '\n';
  std::exit(1);
}

void verify(ll l, ll r, ll nl, ll nr) {
  auto result = cap(l,r,nl,nr);
  auto [left,right] = result;

  // 端点の大小関係と、入力端点のいずれかであることを検証。
  // 空集合でも端点を正規化しない現在の仕様を確認する。
  check(left >= l && left >= nl && (left == l || left == nl),
        "left endpoint", l,r,nl,nr,result);
  check(right <= r && right <= nr && (right == r || right == nr),
        "right endpoint", l,r,nl,nr,result);
  check(cap(nl,nr,l,r) == result, "commutativity", l,r,nl,nr,result);
  check(cap(l,r,l,r) == std::pair<ll,ll>{l,r}, "idempotence", l,r,nl,nr,result);

  // 端点での所属判定。半開区間と閉区間の両方で成立する。
  for (ll x : {l,r,nl,nr,left,right,LO,HI,0LL}) {
    bool half_open = (l <= x && x < r && nl <= x && x < nr);
    bool closed = (l <= x && x <= r && nl <= x && x <= nr);
    check((left <= x && x < right) == half_open,
          "half-open membership", l,r,nl,nr,result);
    check((left <= x && x <= right) == closed,
          "closed membership", l,r,nl,nr,result);
  }
}

void boundaries() {
  // 11 座標から、l <= r を満たす全区間の組を列挙する。
  for (ll l = -5; l <= 5; ++l) {
    for (ll r = l; r <= 5; ++r) {
      for (ll nl = -5; nl <= 5; ++nl) {
        for (ll nr = nl; nr <= 5; ++nr) verify(l,r,nl,nr);
      }
    }
  }

  const std::vector<ll> edges = {
    LO, LO+1, LO+2, -100, -2, -1, 0, 1, 2, 100, HI-2, HI-1, HI
  };
  for (ll l : edges) {
    for (ll r : edges) {
      if (r < l) continue;
      for (ll nl : edges) {
        for (ll nr : edges) {
          if (nl <= nr) verify(l,r,nl,nr);
        }
      }
    }
  }
}

void randomized() {
  std::mt19937_64 rng(SEED);
  std::uniform_int_distribution<ll> wide(LO,HI);
  std::uniform_int_distribution<ll> narrow(-32,32);

  auto sample = [&](auto& dist) {
    ll l = dist(rng), r = dist(rng);
    ll nl = dist(rng), nr = dist(rng);
    if (r < l) std::swap(l,r);
    if (nr < nl) std::swap(nl,nr);
    verify(l,r,nl,nr);
  };

  for (ll j = 0; j < RANDOM_CASES; ++j) {
    sample(wide);
    sample(narrow);
  }
}
} // namespace interval_operation_test

int main() {
  interval_operation_test::boundaries();
  interval_operation_test::randomized();
  std::cout << "interval_operation: all tests passed (seed="
            << interval_operation_test::SEED << ", random iterations="
            << interval_operation_test::RANDOM_CASES << ")\n";
}
