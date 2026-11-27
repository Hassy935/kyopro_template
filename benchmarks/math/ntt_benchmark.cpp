#include <Library/math/ntt.hpp>

namespace {
using Clock=chrono::steady_clock;
constexpr ull seed=0x4e545442454e4348;
constexpr int repeats=5;

template<class T>
ull checksum(const vc<T>& a){
  ull sum=0;
  for(size_t i=0;i<a.size();++i) sum+=ull(i+1)*a[i].val();
  return sum;
}

template<ll M>
void bench(int exponent){
  using Mint=static_modint<M>;
  mt19937_64 rng(seed);
  const size_t n=size_t(1)<<exponent;
  vc<Mint> a(n);
  for(auto& x:a) x=rng();
  auto f=a;
  ::ntt(f);
  const size_t iterations=clamp(size_t(262144)/n,size_t(1),size_t(4096));
  array<array<double,repeats>,2> times{};
  for(int r=0;r<repeats;++r) for(int off=0;off<2;++off){
    const int k=(r+off)%2;
    vector<vc<Mint>> work(iterations,k==0 ? a : f);
    auto start=Clock::now();
    if(k==0){ for(auto& v:work) ::ntt(v); }
    else { for(auto& v:work) ::intt(v); }
    auto end=Clock::now();
    for(const auto& v:work)
      if(v!=(k==0 ? f : a)) throw runtime_error("public NTT benchmark mismatch");
    times[k][r]=chrono::duration<double,micro>(end-start).count()/iterations;
  }
  cout<<M<<' '<<n<<' '<<iterations;
  for(auto& t:times){ ranges::sort(t); cout<<' '<<t[2]; }
  cout<<' '<<checksum(f)<<' '<<checksum(a)<<'\n';
}
}

int main(){
  cout<<"seed "<<seed<<" repeats "<<repeats<<'\n'<<fixed<<setprecision(3);
  cout<<"mod length iterations forward_us inverse_us forward_checksum inverse_checksum\n";
  for(int rank:{4,6,8,10,12,14,16,17,18,20,21}) bench<998244353>(rank);
  for(int rank:{8,14,18}) bench<167772161>(rank);
}
