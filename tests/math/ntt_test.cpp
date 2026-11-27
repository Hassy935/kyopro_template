#include <Library/math/ntt.hpp>
#include <Library/io/debug.hpp>

namespace {
constexpr ull seed=0x4e545446;
mt19937_64 rng(seed);
ll modulus;
size_t length;

void check(bool condition,const char* message){
  if(condition) return;
  cerr<<"FAIL "<<message<<" mod="<<modulus<<" length="<<length<<" seed="<<seed<<'\n';
  exit(1);
}

static_assert(!ntt_detail::is_prime(1) && !ntt_detail::is_prime(49) &&
              !ntt_detail::is_prime(2147483646) && !ntt_detail::is_prime(561));
static_assert(ntt_detail::is_prime(2) && ntt_detail::is_prime(2147483647));
static_assert(ntt_detail::Plan<2>::generator==1);

template<ll M>
void tables(size_t expected){
  using P=ntt_detail::Plan<M>;
  modulus=M;
  check(P::max_length==expected,"maximum length");
  auto g=P::generator;
  check(g.pow(M-1)==1,"Fermat");
  ll x=M-1;
  for(ll p=2;p<=x/p;++p){
    if(x%p!=0) continue;
    check(g.pow((M-1)/p)!=1,"generator order");
    do {x/=p;} while(x%p==0);
  }
  if(x>1) check(g.pow((M-1)/x)!=1,"generator largest factor");
  for(int i=0;i<=P::rank;++i){
    ll n=1LL<<i;
    auto w=P::table.root[i];
    check(w.pow(n)==1 && (i==0 || w.pow(n/2)!=1),"root order");
    check(w*P::table.inverse_root[i]==1 && P::table.scale[i]*n==1,"inverse root and scaling");
  }
}

template<ll M>
vc<static_modint<M>> direct(const vc<static_modint<M>>& a,bool inverse){
  using Mint=static_modint<M>;
  const ll n=ssize(a);
  auto w=ntt_detail::Plan<M>::generator.pow((M-1)/n);
  if(inverse) w=w.inv();
  Mint inverse_n=Mint(n).inv();
  vc<Mint> result(n);
  for(ll k=0;k<n;++k){
    Mint step=w.pow(k),power=1;
    for(Mint x:a){ result[k]+=x*power; power*=step; }
    if(inverse) result[k]*=inverse_n;
  }
  return result;
}

template<ll M>
void transforms(const vc<static_modint<M>>& input){
  using Mint=static_modint<M>;
  modulus=M;
  length=input.size();
  auto f=input, inv=input, internal=input;
  ::ntt(f);
  ::intt(inv);
  check(f==direct(input,false),"positive DFT normal order");
  check(inv==direct(input,true),"negative DFT normal order");
  ntt_detail::forward(internal);
  const int rank=countr_zero(length);
  for(size_t i=0;i<length;++i){
    size_t reversed=0;
    for(int j=0;j<rank;++j) reversed=2*reversed+(i>>j&1);
    check(internal[i]==f[reversed],"internal bit order");
  }
  ntt_detail::inverse(internal);
  for(size_t i=0;i<length;++i) check(internal[i]==input[i]*Mint(length),"unscaled inverse");
  ::intt(f);
  ::ntt(inv);
  check(f==input && inv==input,"both round trips");
}

template<ll M>
void small(){
  using Mint=static_modint<M>;
  for(size_t n=1;n<=min(size_t(128),ntt_detail::Plan<M>::max_length);n*=2){
    transforms(vc<Mint>(n));
    transforms(vc<Mint>(n,1));
    transforms(vc<Mint>(n,-1));
    vc<Mint> x(n);
    x[n/2]=1;
    transforms(x);
    for(int rep=0;rep<3;++rep){
      for(auto& z:x) z=rng();
      transforms(x);
    }
  }
}

template<ll M>
void properties(size_t n){
  using Mint=static_modint<M>;
  modulus=M; length=n;
  vc<Mint> a(n),b(n),sum(n),cyclic(n);
  for(size_t i=0;i<n;++i){ a[i]=rng(); b[i]=rng(); sum[i]=a[i]+b[i]; }
  for(size_t i=0;i<n;++i)
    for(size_t j=0;j<n;++j) cyclic[(i+j)%n]+=a[i]*b[j];
  ::ntt(a); ::ntt(b); ::ntt(sum);
  for(size_t i=0;i<n;++i){
    check(sum[i]==a[i]+b[i],"linearity");
    a[i]*=b[i];
  }
  ::intt(a);
  check(a==cyclic,"circular convolution theorem");
}

template<ll M>
void large_roundtrip(size_t n){
  modulus=M; length=n;
  vc<static_modint<M>> a(n);
  for(auto& x:a) x=rng();
  auto original=a;
  ::ntt(a);
  ::intt(a);
  check(a==original,"large roundtrip");
}

bool invalid(string_view mode){
  if(mode=="empty"){ vc<modint998244353> a; ::ntt(a); }
  else if(mode=="non-power"){ vc<modint998244353> a(3); ::intt(a); }
  else if(mode=="oversize"){ vc<static_modint<17>> a(32); ::ntt(a); }
  else return false;
  return true;
}
}

int main(int argc,char** argv){
  if(argc==2) return invalid(argv[1]) ? 0 : 2;
  tables<2>(1); tables<3>(2); tables<5>(4); tables<17>(16); tables<97>(32);
  tables<257>(256); tables<65537>(65536); tables<167772161>(1<<25);
  tables<469762049>(1<<26); tables<754974721>(1<<24); tables<998244353>(1<<23);
  tables<1000000007>(2); tables<1224736769>(1<<24); tables<2013265921>(1<<27);
  tables<2147483647>(2);
  small<2>(); small<3>(); small<5>(); small<17>(); small<97>(); small<257>(); small<65537>();
  small<167772161>(); small<469762049>(); small<754974721>(); small<998244353>();
  small<1000000007>(); small<1224736769>(); small<2013265921>(); small<2147483647>();
  for(int mask=0;mask<625;++mask){
    vc<static_modint<5>> a(4);
    int t=mask;
    for(auto& x:a){ x=t%5; t/=5; }
    transforms(a);
  }
  properties<998244353>(64); properties<2013265921>(32);
  large_roundtrip<257>(256);
  large_roundtrip<65537>(1<<15); large_roundtrip<65537>(1<<16);
  large_roundtrip<998244353>(1<<22);
  large_roundtrip<998244353>(1<<23);
  cout<<"ntt tests passed\n";
}
