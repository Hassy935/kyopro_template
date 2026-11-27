#include <Library/math/combination.hpp>
#include <atcoder/modint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <type_traits>

namespace combination_test {
constexpr ull SEED=0xC0B1A710ULL;
void check(bool ok,const char* label,ll n=0,ll k=0){
  if(ok) return;
  std::cerr << "[FAIL] " << label << " n=" << n << " k=" << k
            << " seed=" << SEED << '\n';
  std::exit(1);
}

// Pascal の漸化式（逆元を使わない参照実装）。
vector<vector<ll>> pascal(ll N,ll p){
  vector<vector<ll>> a(N+1,vector<ll>(N+1));
  a[0][0]=1;
  for(ll n=1;n<=N;n++){
    a[n][0]=1;
    for(ll k=1;k<=n;k++) a[n][k]=(a[n-1][k-1]+a[n-1][k])%p;
  }
  return a;
}

template<class Mint>
void basic(ll N){
  ll p=Mint::mod();
  auto ref=pascal(N,p);
  Combination<Mint> c;
  check(c.limit()==0,"initial limit");
  check(c.fact(0).val()==1,"0!");
  check(c.inv_fact(0).val()==1,"inverse 0!");
  check(c.C(0,0).val()==1,"C(0,0)");
  check(c.P(0,0).val()==1,"P(0,0)");
  check(c.H(0,0).val()==1,"H(0,0)");
  check(c.H(0,1).val()==0,"H(0,1)");
  check(c.multinomial({}).val()==1,"empty multinomial");
  check(c.catalan(0).val()==1,"Catalan(0)");
  check(c.C(-1,0).val()==0 && c.C(3,-1).val()==0 && c.C(1,2).val()==0,"invalid C");
  check(c.P(-1,0).val()==0 && c.P(3,-1).val()==0 && c.P(1,2).val()==0,"invalid P");
  check(c.H(-1,0).val()==0 && c.H(1,-1).val()==0,"invalid H");
  check(c.C_naive(-1,0).val()==0 && c.C_naive(1,2).val()==0 && c.C_naive(2,-1).val()==0,"invalid C_naive");
  check(c.multinomial({1,-1}).val()==0 && c.catalan(-1).val()==0,"invalid other");
  check(c.limit()==0,"invalid queries do not grow");
  ll f=1;
  for(ll n=0;n<=N;n++){
    ll old=c.limit();
    if(n) f=f*n%p;
    check(c.fact(n).val()==f,"fact",n);
    check(c.limit()>=n && c.limit()>=old && c.limit()<p,"growth",n,c.limit());
    check((c.fact(n)*c.inv_fact(n)).val()==1,"inverse factorial",n);
    if(n) check(c.inv_fact(n-1)==c.inv_fact(n)*Mint(n),"inverse recurrence",n);
    ll perm=1;
    for(ll k=0;k<=n;k++){
      check(c.C(n,k).val()==ref[n][k],"C",n,k);
      check(c.C_naive(n,k).val()==ref[n][k],"C_naive small",n,k);
      check(c.P(n,k).val()==perm,"P",n,k);
      perm=perm*(n-k)%p;
      if(n>0 && n+k-1<=N) check(c.H(n,k).val()==ref[n+k-1][k],"H",n,k);
    }
    if(2*n<=N){
      ll cat=(ref[2*n][n]-(n?ref[2*n][n-1]:0)+p)%p;
      check(c.catalan(n).val()==cat,"Catalan",n);
    }
  }
  for(ll a=0;a<=N && a<=12;a++) for(ll b=0;a+b<=N && b<=12;b++)
    for(ll d=0;a+b+d<=N && d<=12;d++){
      ll expected=ref[a+b+d][a]*ref[b+d][b]%p;
      check(c.multinomial({a,b,d}).val()==expected,"multinomial",a,b);
    }
  ll old=c.limit(); c.extend(0); c.extend(N);
  check(c.limit()==old,"extend never shrinks");
  combination<Mint> fixed(N);
  check(fixed.limit()==N,"explicit initial limit",N,fixed.limit());
  for(ll n=0;n<=N;n++){
    check(fixed.fact(n)==c.fact(n),"initial vs incremental fact",n);
    check(fixed.inv_fact(n)==c.inv_fact(n),"initial vs incremental inverse",n);
  }
}

// 小さい素数について Lucas の定理で巨大な n を独立に照合する。
ll lucas(ll n,ll k,ll p,const vector<vector<ll>>& ref){
  ll res=1;
  while(n || k){
    ll a=n%p,b=k%p;
    if(b>a) return 0;
    res=res*ref[a][b]%p;
    n/=p; k/=p;
  }
  return res;
}

template<int P>
void large(){
  using Mint=atcoder::static_modint<P>;
  Combination<Mint> c;
  auto ref=pascal(P-1,P);
  std::mt19937_64 rng(SEED+P);
  constexpr ll HI=std::numeric_limits<ll>::max();
  for(ll n : {0LL,1LL,(ll)P-1,(ll)P,(ll)P+1,2LL*P,HI-1,HI}){
    for(ll k=0;k<P && k<=n;k++){
      check(c.C_naive(n,k).val()==lucas(n,k,P,ref),"large boundary",n,k);
      check(c.C_naive(n,n-k).val()==lucas(n,n-k,P,ref),"large symmetry",n,n-k);
    }
  }
  std::uniform_int_distribution<ll> dist(0,HI);
  for(ll i=0;i<10000;i++){
    ll n=dist(rng),k=std::min<ll>(n,rng()%P);
    if(i%2) k=n-k;
    check(c.C_naive(n,k).val()==lucas(n,k,P,ref),"large random",n,k);
  }
  check(c.limit()<P,"large only precomputes k");
}

void growth(){
  using Mint=atcoder::modint998244353;
  Combination<Mint> c;
  ll f=1;
  for(ll n=1;n<=200000;n++){
    f=f*n%Mint::mod();
    check(c.fact(n).val()==f,"large growth fact",n);
    check((c.fact(n)*c.inv_fact(n)).val()==1,"large growth inverse",n);
    check(c.limit()>=n && c.limit()<2*n+1,"geometric growth",n,c.limit());
  }
  constexpr ll HI=std::numeric_limits<ll>::max();
  check(c.C_naive(HI,1)==Mint(HI),"LL max choose 1");
  i128 v=(i128)HI*(HI-1)/2;
  check(c.C_naive(HI,2).val()==(ll)(v%Mint::mod()),"LL max choose 2");
  check(c.C_naive(HI,HI).val()==1,"LL max choose itself");
}
} // namespace combination_test

int main(){
  using namespace combination_test;
  basic<atcoder::modint998244353>(200);
  basic<atcoder::modint1000000007>(200);
  basic<atcoder::static_modint<2>>(1);
  basic<atcoder::static_modint<7>>(6);
  basic<atcoder::static_modint<101>>(100);
  using DM=atcoder::dynamic_modint<42>;
  DM::set_mod(17); basic<DM>(16);
  DM::set_mod(101); basic<DM>(100);
  large<2>(); large<7>(); large<101>();
  growth();
  std::cout << "combination: all tests passed (seed=" << SEED << ")\n";
}
