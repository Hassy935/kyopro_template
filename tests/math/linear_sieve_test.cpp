#include <Library/math/linear_sieve.hpp>
#include <cstdlib>
#include <iostream>
#include <random>
#include <type_traits>

namespace linear_sieve_test {
constexpr unsigned long long SEED=0x11EA25ULL;
void check(bool ok,const char* what,ll n,ll x=0){
  if(ok) return;
  std::cerr << "[FAIL] " << what << " N=" << n << " x=" << x
            << " seed=" << SEED << '\n';
  std::exit(1);
}
std::vector<std::pair<ll,ll>> trial_factor(ll x){
  std::vector<std::pair<ll,ll>> res;
  for(ll p=2;p<=x/p;p++){
    ll e=0;
    while(x%p==0){ x/=p; e++; }
    if(e) res.emplace_back(p,e);
  }
  if(x>1) res.emplace_back(x,1);
  return res;
}
std::vector<ll> trial_divisors(ll x){
  std::vector<ll> res;
  for(ll d=1;d<=x/d;d++) if(x%d==0){
    res.push_back(d);
    if(d!=x/d) res.push_back(x/d);
  }
  std::sort(res.begin(),res.end());
  return res;
}
void verify_value(const l_sieve& s,ll x){
  ll n=s.limit();
  if(x==0){ check(!s.is_prime(x),"is_prime(0)",n,x); return; }
  auto ref=trial_factor(x);
  bool p=x>=2 && ref.size()==1 && ref[0].first==x;
  check(s.is_prime(x)==p,"is_prime",n,x);
  if(x>=2) check(s.get_spf(x)==ref.front().first,"get_spf",n,x);
  check(s.factorize(x)==ref,"factorize",n,x);
  auto div=s.divisors(x);
  std::sort(div.begin(),div.end());
  auto expected=trial_divisors(x);
  check(div==expected,"divisors",n,x);
  check(s.count_divisors(x)==std::ssize(expected),"count_divisors",n,x);
}
void verify(ll n){
  const l_sieve s(n);
  check(s.limit()==n,"limit",n);
  std::vector<ll> ref;
  for(ll x=0;x<=n;x++){
    verify_value(s,x);
    if(x>=2){
      auto factors=trial_factor(x);
      if(factors.front().first==x) ref.push_back(x);
    }
  }
  check(s.get_primes()==ref,"get_primes",n);
  check(s.count_primes()==std::ssize(ref),"count_primes",n);
}
void run(){
  static_assert(std::is_same_v<l_sieve,LinearSieve>);
  static_assert(std::is_same_v<decltype(std::declval<const l_sieve&>().get_primes()),const std::vector<ll>&>);
  for(ll n=0;n<=128;n++) verify(n);
  for(ll n:{255LL,256LL,257LL,1024LL,4096LL}) verify(n);
  const l_sieve s(1000000);
  const auto& primes=s.get_primes();
  check(s.count_primes()==78498 && std::ssize(primes)==78498,"pi(10^6)",s.limit());
  check(primes.front()==2 && primes.back()==999983,"prime list endpoints",s.limit());
  ll total=0;
  for(ll p:primes) total+=p;
  check(total==37550402023LL,"prime sum",s.limit());
  for(ll x:{0LL,1LL,2LL,4LL,12LL,360LL,720720LL,524288LL,531441LL,999983LL,1000000LL}) verify_value(s,x);
  std::mt19937_64 rng(SEED);
  for(ll j=0;j<10000;j++) verify_value(s,rng()%1000001);
}
}
int main(){
  linear_sieve_test::run();
  std::cout << "linear_sieve: all tests passed (seed="
            << linear_sieve_test::SEED << ")\n";
}
