#include <Library/math/convolution.hpp>
#include <atcoder/convolution>

namespace {
using Clock=chrono::steady_clock;
constexpr ull seed=0x434f4e5642454e43;
constexpr int repeats=5;

template<bool moved,class T,class F>
pair<double,ull> measure(const vc<T>& a,const vc<T>& b,size_t count,F operation){
  vector<pair<vc<T>,vc<T>>> work;
  if constexpr(moved) work.assign(count,{a,b});
  ull sum=0;
  auto start=Clock::now();
  for(size_t i=0;i<count;++i){
    auto c=[&]{
      if constexpr(moved) return operation(std::move(work[i].first),std::move(work[i].second));
      else return operation(a,b);
    }();
    asm volatile("" : : "g"(c.data()) : "memory");
    sum+=c[(i*997)%c.size()].val();
  }
  auto end=Clock::now();
  return {chrono::duration<double,micro>(end-start).count()/count,sum};
}

template<ll M>
void bench(size_t n,size_t m){
  using Mint=static_modint<M>;
  using A=atcoder::static_modint<M>;
  vc<Mint> a(n),b(m);
  vc<A> x(n),y(m);
  mt19937_64 rng(seed);
  for(size_t i=0;i<n;++i){ a[i]=rng(); x[i]=a[i].val(); }
  for(size_t i=0;i<m;++i){ b[i]=rng(); y[i]=b[i].val(); }
  auto c=::convolution(a,b);
  auto d=atcoder::convolution(x,y);
  if(c.size()!=d.size()) throw runtime_error("convolution benchmark size");
  for(size_t i=0;i<c.size();++i) if(c[i].val()!=d[i].val())
    throw runtime_error("convolution benchmark result");
  const size_t padded=bit_ceil(c.size());
  const size_t count=clamp(size_t(2'000'000)/max(n+m,min(n*m,padded*bit_width(padded))),
                           size_t(1),size_t(4000));
  auto own=[](auto&& u,auto&& v){
    return ::convolution(std::forward<decltype(u)>(u),std::forward<decltype(v)>(v));
  };
  auto acl=[](auto&& u,auto&& v){
    return atcoder::convolution(std::forward<decltype(u)>(u),std::forward<decltype(v)>(v));
  };
  array<array<double,repeats>,4> times{};
  optional<ull> expected;
  for(int r=0;r<repeats;++r) for(int off=0;off<4;++off){
    int k=(r+off)%4;
    pair<double,ull> sample;
    if(k==0) sample=measure<false>(a,b,count,own);
    if(k==1) sample=measure<false>(x,y,count,acl);
    if(k==2) sample=measure<true>(a,b,count,own);
    if(k==3) sample=measure<true>(x,y,count,acl);
    if(!expected) expected=sample.second;
    if(sample.second!=*expected) throw runtime_error("convolution benchmark checksum");
    times[k][r]=sample.first;
  }
  cout<<M<<' '<<n<<' '<<m<<' '<<padded<<' '<<count;
  for(auto& t:times){ ranges::sort(t); cout<<' '<<t[2]; }
  cout<<' '<<*expected<<'\n';
}
}

int main(){
  cout<<"seed "<<seed<<" repeats "<<repeats<<'\n'<<fixed<<setprecision(3);
  cout<<"mod N M padded iterations own_lvalue_us ACL_lvalue_us own_rvalue_us ACL_rvalue_us checksum\n";
  for(auto [n,m]:array<pair<size_t,size_t>,14>{{
      {2,2},{16,16},{60,61},{65,65},{128,129},{128,130},{1024,1024},
      {4096,8193},{4096,8194},{65536,65536},{524288,524288},
      {1,262144},{8,65536},{64,65536}}}) bench<998244353>(n,m);
  bench<167772161>(1024,1024);
  bench<167772161>(65536,65536);
}
