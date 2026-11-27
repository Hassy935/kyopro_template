#include <Library/math/convolution.hpp>
#include <Library/io/debug.hpp>
#include <atcoder/convolution>

namespace {
constexpr ull seed=0x434f4e56;
mt19937_64 rng(seed);
size_t n,m;
ll modulus;
void check(bool condition,const char* message){
  if(condition) return;
  cerr<<"FAIL "<<message<<" mod="<<modulus<<" N="<<n<<" M="<<m<<" seed="<<seed<<'\n';
  exit(1);
}

template<ll M>
vc<static_modint<M>> reference(const vc<static_modint<M>>& a,const vc<static_modint<M>>& b){
  if(a.empty() || b.empty()) return {};
  vc<static_modint<M>> out(a.size()+b.size()-1);
  for(size_t k=0;k<out.size();++k){
    ll lo=max(0LL,ll(k)-ll(b.size())+1),hi=min(ll(k),ll(a.size())-1);
    uint64_t sum=0;
    for(ll i=lo;i<=hi;++i)
      sum=(sum+uint64_t(a[i].val())*b[k-i].val())%M;
    out[k]=sum;
  }
  return out;
}

template<ll M>
void test(const vc<static_modint<M>>& a,const vc<static_modint<M>>& b){
  n=a.size(); m=b.size(); modulus=M;
  auto saved=a,other=b;
  auto expected=reference(a,b);
  check(::convolution(a,b)==expected && ::convolution(b,a)==expected,"reference and symmetry");
  check(a==saved && b==other,"lvalue preservation");
  check(::convolution(std::move(saved),std::move(other))==expected,"rvalue");
}

template<ll M>
void random_case(size_t h,size_t w){
  vc<static_modint<M>> a(h),b(w);
  for(auto& x:a) x=rng();
  for(auto& x:b) x=rng();
  test(a,b);
}

template<ll M>
void acl_case(size_t h,size_t w){
  using A=atcoder::static_modint<M>;
  vc<static_modint<M>> a(h),b(w);
  vc<A> x(h),y(w);
  for(size_t i=0;i<h;++i){ a[i]=rng(); x[i]=a[i].val(); }
  for(size_t i=0;i<w;++i){ b[i]=rng(); y[i]=b[i].val(); }
  n=h; m=w; modulus=M;
  auto c=::convolution(std::move(a),std::move(b));
  auto d=atcoder::convolution(std::move(x),std::move(y));
  check(c.size()==d.size() && c.size()==h+w-1,"ACL result length");
  for(size_t i=0;i<c.size();++i) check(c[i].val()==d[i].val(),"ACL values");
}

void integration(){
  vc<modint998244353> a(3);
  istringstream tokens("1 -1 2");
  auto* input=cin.rdbuf(tokens.rdbuf());
  bool good=bool(cin>>a);
  cin.rdbuf(input);
  check(good && a==vc<modint998244353>{1,-1,2},"cin vector");
  const auto c=::convolution(a,a);
  check(c==reference(a,a),"self convolution");
  ostringstream out,err;
  auto* cout_buffer=cout.rdbuf(out.rdbuf());
  auto* cerr_buffer=cerr.rdbuf(err.rdbuf());
  cout<<c[4]<<'\n';
  show(c[0],c[1],c[2]);
  int evaluated=0;
  debug(c);
  dbg(++evaluated,c);
  cout.rdbuf(cout_buffer);
  cerr.rdbuf(cerr_buffer);
  check(out.str()=="4\n1 998244351 5\n","cout and show");
#ifdef LOCAL
  check(evaluated==1 && err.str()=="1 998244351 5 998244349 4 \n[++evaluated,c] = 1, [1, 998244351, 5, 998244349, 4]\n","debug and dbg");
#else
  check(evaluated==0 && err.str().empty(),"non-LOCAL debug");
#endif
}
}

int main(int argc,char** argv){
  if(argc==2){
    if(string_view(argv[1])!="oversize") return 2;
    vc<static_modint<257>> a(129),b(129);
    (void)::convolution(std::move(a),std::move(b));
    return 0;
  }
  using Mint=modint998244353;
  test(vc<Mint>{},vc<Mint>{});
  test(vc<Mint>{},vc<Mint>{1,2});
  test(vc<Mint>{-1},vc<Mint>{-1});
  random_case<2>(1,300);
  random_case<17>(5,200);
  for(int x=0;x<8;++x) for(int y=0;y<8;++y){
    vc<static_modint<2>> a(3),b(3);
    for(int i=0;i<3;++i){ a[i]=x>>i&1; b[i]=y>>i&1; }
    test(a,b);
  }
  constexpr auto limit=convolution_detail::naive_limit;
  for(size_t k:{limit-1,limit,limit+1}){
    random_case<998244353>(k,k+3);
    random_case<998244353>(k,1051);
  }
  for(size_t k:{127,128,129}) random_case<998244353>(k,130);
  for(size_t k:{127,128,129}) random_case<257>(k,257-k);
  random_case<998244353>(2,30'000);
  random_case<998244353>(100,1024);
  random_case<2013265921>(101,170);
  test(vc<Mint>(130),vc<Mint>(125,-1));
  test(vc<Mint>(130,-1),vc<Mint>(125,-1));
  vc<Mint> p(100),q(200); p[92]=-1; q[175]=8;
  test(p,q);
  for(int i=0;i<100;++i) random_case<998244353>(rng()%250,rng()%250);
  acl_case<998244353>(65,65);
  acl_case<998244353>(1<<16,(1<<16)+2);
  acl_case<167772161>(6000,137);
  acl_case<469762049>(1003,800);
  acl_case<754974721>(1500,1550);
  acl_case<1224736769>(350,508);
  acl_case<257>(128,129);
  vc<Mint> reusable(88,3),shorter(72,4);
  reusable.reserve(256);
  const auto* storage=reusable.data();
  auto expected=reference(reusable,shorter);
  auto result=::convolution(std::move(reusable),std::move(shorter));
  check(result==expected && result.data()==storage,"preallocated rvalue storage reuse");
  integration();
  cout<<"convolution tests passed\n";
}
