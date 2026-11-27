#include <Library/math/integer.hpp>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

namespace integer_test {
constexpr ll LO = std::numeric_limits<ll>::min();
constexpr ll HI = std::numeric_limits<ll>::max();
constexpr ll ROOT = 3037000499LL;
constexpr ll COMB = 4294967296LL;
constexpr unsigned long long SEED = 0x1A7E6E2ULL;
constexpr ll RANDOM_CASES = 100000;
static_assert(std::numeric_limits<ll>::digits == 63);

// NDEBUG に依存しない検証。失敗時は入力と乱数シードを表示する。
void check(bool ok, const char* label, ll x, ll y = 0) {
  if (ok) return;
  std::cerr << "[FAIL] " << label << " x=" << x << " y=" << y
            << " seed=" << SEED << '\n';
  std::exit(1);
}

void test_sq(ll x) {
  check((i128)sq(x) == (i128)x*x, "sq", x);
}

void test_div(ll x, ll y) {
  // 結果が ll に収まらない組合せは仕様外。
  if (y == 0 || (x == LO && y == -1)) return;
  // 分母を正に正規化して、不等式で丸め方向を検証する。
  i128 a = x, b = y;
  if (b < 0) { a = -a; b = -b; }
  i128 f = divf(x,y), c = divc(x,y);
  check(f*b <= a && a < (f+1)*b, "divf", x,y);
  check((c-1)*b < a && a <= c*b, "divc", x,y);
  check(c-f == (a%b != 0), "div difference", x,y);
  if (y > 0) {
    i128 r = mod_pos(x,y);
    check(0 <= r && r < b && (a-r)%b == 0, "mod_pos", x,y);
    check(f*b+r == a, "division identity", x,y);
  }
}

// 浮動小数点演算を使わない独立した参照実装。
ll sqrt_ref(ll x) {
  ll low = 0, high = ROOT+1;
  while (high-low > 1) {
    ll mid = low+(high-low)/2;
    if ((i128)mid*mid <= x) low = mid;
    else high = mid;
  }
  return low;
}

void test_sqrt(ll x) {
  ll f = sqrt_ref(x);
  bool perfect = (i128)f*f == x;
  check(floor_sqrt(x) == f, "floor_sqrt", x);
  check(ceil_sqrt(x) == f+!perfect, "ceil_sqrt", x);
  check(is_sq(x) == perfect, "is_sq", x);
}

void test_near_square(ll k) {
  i128 v = (i128)k*k;
  for (ll delta = -2; delta <= 2; ++delta) {
    i128 x = v+delta;
    if (0 <= x && x <= HI) test_sqrt((ll)x);
  }
}

void test_comb(ll x) {
  check((i128)nC2(x) == (i128)x*(x-1)/2, "nC2", x);
}

void boundaries() {
  const std::vector<ll> edges = {
    LO, LO+1, LO+2, -COMB, -ROOT-1, -ROOT,
    -100, -7, -3, -2, -1, 0, 1, 2, 3, 7, 100,
    ROOT, ROOT+1, COMB, HI-2, HI-1, HI
  };
  for (ll x : edges) {
    for (ll y : edges) test_div(x,y);
    if (-ROOT <= x && x <= ROOT) test_sq(x);
    if (x >= 0) test_sqrt(x);
    if (0 <= x && x <= COMB) test_comb(x);
  }
  for (ll x = -128; x <= 128; ++x) {
    for (ll y = -128; y <= 128; ++y) test_div(x,y);
  }
  for (ll x = 0; x <= 10000; ++x) {
    test_sq(x); test_sq(-x); test_sqrt(x); test_comb(x);
  }
  for (ll j = 0; j <= 1024; ++j) {
    test_near_square(j); test_near_square(ROOT-j);
    test_sq(ROOT-j); test_sq(-ROOT+j);
    test_comb(COMB-j);
    test_sqrt(HI-j);
  }
  for (ll j = 0; j <= 62; ++j) {
    ll v = 1LL << j;
    test_sqrt(v-1); test_sqrt(v); test_sqrt(v+1);
    test_div(LO,v); test_div(HI,-v);
  }
}

void randomized() {
  std::mt19937_64 rng(SEED);
  std::uniform_int_distribution<ll> signed_dist(LO,HI);
  std::uniform_int_distribution<ll> nonnegative(0,HI);
  std::uniform_int_distribution<ll> square_input(-ROOT,ROOT);
  std::uniform_int_distribution<ll> root_dist(0,ROOT);
  std::uniform_int_distribution<ll> comb_dist(0,COMB);
  std::uniform_int_distribution<ll> small_div(-1000,1000);
  for (ll j = 0; j < RANDOM_CASES; ++j) {
    ll x = signed_dist(rng);
    ll y = signed_dist(rng);
    test_div(x,y);
    test_div(x,small_div(rng));
    test_div(small_div(rng),y);
    test_sq(square_input(rng));
    test_sqrt(nonnegative(rng));
    test_near_square(root_dist(rng));
    test_comb(comb_dist(rng));
  }
}
} // namespace integer_test

int main() {
  integer_test::boundaries();
  integer_test::randomized();
  std::cout << "integer: all tests passed (seed=" << integer_test::SEED
            << ", random iterations=" << integer_test::RANDOM_CASES << ")\n";
}
