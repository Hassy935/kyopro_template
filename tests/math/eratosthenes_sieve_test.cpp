#include <Library/math/eratosthenes_sieve.hpp>
#include <cstdlib>
#include <iostream>
#include <random>
#include <type_traits>

namespace eratosthenes_sieve_test {
constexpr unsigned long long SEED=0xE2A705ULL;
void check(bool ok,const char* what,ll n,ll x=0){
  if(ok) return;
  std::cerr << "[FAIL] " << what << " N=" << n << " x=" << x
            << " seed=" << SEED << '\n';
  std::exit(1);
}
bool trial(ll x){
  if(x<2) return false;
  for(ll p=2;p<=x/p;p++) if(x%p==0) return false;
  return true;
}
void verify(ll n){
  const e_sieve s(n);
  check(s.limit()==n,"limit",n);
  std::vector<ll> ref;
  for(ll x=0;x<=n;x++){
    bool p=trial(x);
    check(s.is_prime(x)==p,"is_prime",n,x);
    if(p) ref.push_back(x);
  }
  check(s.get_primes()==ref,"get_primes",n);
  check(s.count_primes()==std::ssize(ref),"count_primes",n);
}
void run(){
  static_assert(std::is_same_v<e_sieve,EratosthenesSieve>);
  for(ll n=0;n<=128;n++) verify(n);
  for(ll n:{255LL,256LL,257LL,1023LL,1024LL,1025LL,4096LL,65536LL}) verify(n);
  std::mt19937_64 rng(SEED);
  for(ll j=0;j<40;j++) verify(rng()%4097);
  const e_sieve s(1000000);
  const auto primes=s.get_primes();
  check(s.count_primes()==78498,"pi(10^6)",s.limit());
  check(std::ssize(primes)==78498 && primes.front()==2 && primes.back()==999983,
        "prime list endpoints",s.limit());
  ll total=0;
  for(ll p:primes) total+=p;
  check(total==37550402023LL,"prime sum",s.limit());
  for(ll j=0;j<10000;j++){
    ll x=rng()%1000001;
    check(s.is_prime(x)==trial(x),"random is_prime",s.limit(),x);
  }
  auto copy=s.get_primes();
  copy.clear();
  check(s.get_primes()==primes,"list independence",s.limit());
}
}
int main(){
  eratosthenes_sieve_test::run();
  std::cout << "eratosthenes_sieve: all tests passed (seed="
            << eratosthenes_sieve_test::SEED << ")\n";
}
