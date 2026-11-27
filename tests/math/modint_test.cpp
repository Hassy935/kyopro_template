#include <Library/io/debug.hpp>
#include <Library/math/modint.hpp>
#include <Library/math/combination.hpp>
#include <Library/data_structure/fenwick_tree.hpp>
#include <Library/data_structure/potential_union_find.hpp>
#include <Library/data_structure/dual_segment_tree_preset.hpp>
#include <atcoder/modint>

namespace {

using S=static_modint<13>;
using D=dynamic_modint<12>;
using Wide=__uint128_t;
constexpr ull seed=0x7319F621;
mt19937_64 rng(seed);
int modulus=0;
ull trial=0;

void check(bool ok,const char* what){
  if(ok) return;
  cerr<<"FAIL "<<what<<" seed="<<seed<<" modulus="<<modulus<<" trial="<<trial<<'\n';
  exit(1);
}

template<class T> concept Ordered=requires(T a){ a<a; };
template<class T> concept FloatRaw=requires{ T::raw(1.0); };
template<class T> concept FloatSet=requires{ T::set_mod(1.0); };

static_assert(sizeof(S)==4 && sizeof(D)==4);
static_assert(is_same_v<modint998244353,static_modint<998244353>>);
static_assert(is_same_v<modint1000000007,static_modint<1000000007>>);
static_assert(is_same_v<modint,dynamic_modint<0>>);
static_assert(is_same_v<decltype(S::mod()),int> && is_same_v<decltype(D{}.val()),int>);
static_assert(is_convertible_v<ll,S> && is_convertible_v<ull,D> && is_convertible_v<bool,S>);
static_assert(!is_constructible_v<S,double> && !is_constructible_v<D,long double>);
static_assert(!is_convertible_v<S,int> && !is_convertible_v<S,D> && !is_convertible_v<D,S>);
static_assert(!is_convertible_v<S,static_modint<17>> && !is_convertible_v<D,modint>);
static_assert(!Ordered<S> && !Ordered<D> && !FloatRaw<S> && !FloatSet<D>);

constexpr bool constexpr_api(){
  using Z=static_modint<1>;
  constexpr Z z=123;
  static_assert(z.val()==0 && z.pow(0).val()==0 && z.inv().val()==0 && (z/z).val()==0);
  S a;
  if(a!=0 || S::mod()!=13) return false;
  a=S::raw(12ULL);
  if(a++!=12 || a!=0 || --a!=12 || a--!=12 || ++a!=12) return false;
  if(+a!=12 || -a!=1 || (a+=4)!=3 || (a-=5)!=11 || (a*=2)!=9 || (a/=3)!=3) return false;
  return 2+a==5 && a+2==5 && 2-a==12 && a-2==1 && 2*a==6 && a*2==6 &&
         6/a==2 && a/3==1 && a==3 && 3==a && 2!=a && a!=2 &&
         a.pow(0)==1 && a.pow(100)*a.inv()==a.pow(99);
}
static_assert(constexpr_api());

template<class M,class T>
void endpoints(){
  for(T x:{numeric_limits<T>::min(),numeric_limits<T>::max()}){
    int m=M::mod();
    if constexpr(is_signed_v<T> || is_same_v<T,i128>){
      i128 r=i128(x)%m;
      check(M(x).val()==(r+m)%m,"signed endpoint");
    }else{
      check(M(x).val()==int(Wide(x)%uint32_t(m)),"unsigned endpoint");
    }
  }
}

template<class M>
void normalize(){
  endpoints<M,bool>();
  endpoints<M,char>();
  endpoints<M,wchar_t>();
  endpoints<M,char8_t>();
  endpoints<M,char16_t>();
  endpoints<M,char32_t>();
  endpoints<M,signed char>();
  endpoints<M,unsigned char>();
  endpoints<M,short>();
  endpoints<M,unsigned short>();
  endpoints<M,int>();
  endpoints<M,unsigned>();
  endpoints<M,long>();
  endpoints<M,unsigned long>();
  endpoints<M,ll>();
  endpoints<M,ull>();
  endpoints<M,i128>();
  endpoints<M,Wide>();
  for(ll k=-3;k<=3;++k){
    for(ll d=-1;d<=1;++d){
      ll x=k*M::mod()+d;
      check(M(x).val()==(x%M::mod()+M::mod())%M::mod(),"multiple neighbors");
    }
  }
  M original=-1,copy=original,moved=std::move(copy);
  copy=moved;
  moved=std::move(copy);
  check(moved==original && M{}.val()==0,"copy/move/default");
}

ll ref_pow(ll a,ull n,ll m){
  if(n==0) return 1%m;
  ll half=ref_pow(a,n/2,m);
  ll square=ll(i128(half)*half%m);
  return n%2 ? ll(i128(square)*a%m) : square;
}

template<class M>
void check_pair(ll a,ll b){
  const ll m=M::mod();
  M x=M::raw(a),y=M::raw(b);
  check((x+y).val()==(a+b)%m && (x-y).val()==(a-b+m)%m &&
        (x*y).val()==i128(a)*b%m,"independent arithmetic");
  if(gcd(b,m)==1){
    M inv=y.inv();
    check((i128(inv.val())*b)%m==1%m &&
          (x/y).val()==i128(a)*inv.val()%m,"inverse and division");
    if(m<=25){
      ll expected=0;
      while(expected*b%m!=1%m) ++expected;
      check(inv.val()==expected,"exhaustive inverse reference");
    }
  }
}

template<class M>
void operations(){
  modulus=M::mod();
  normalize<M>();
  check_pair<M>(modulus-1,modulus-1);
  if(modulus<=25){
    for(int a=0;a<modulus;++a)
      for(int b=0;b<modulus;++b){
        trial=ull(a)*modulus+b;
        check_pair<M>(a,b);
      }
  }
  for(trial=0;trial<3000;++trial){
    ll a=ll(rng()%modulus),b=ll(rng()%modulus);
    check_pair<M>(a,b);
    if(trial%17==0){
      ll n=ll(rng()>>1);
      check(M(a).pow(n).val()==ref_pow(a,n,modulus) && M(a).pow(0).val()==1%modulus,"power");
    }
  }
  M x=0;
  check(x--==0 && x==modulus-1 && x++==modulus-1 && x==0,"postfix wrap");
  check(--x==modulus-1 && ++x==0,"prefix wrap");
  check((x+=2)==2 && (x-=2)==0 && (-M(1)+1)==0 && +x==0,"remaining operators");
}

template<class M,class A>
void acl(){
  for(trial=0;trial<3000;++trial){
    ull a=rng();
    i128 b=-i128(rng()>>1)*rng();
    // 負の広い値を構築するときも符号付きの範囲に収める．
    M x=a,y=b;
    A u=a,v=b;
    check(x.val()==u.val() && y.val()==v.val() &&
          (x+y).val()==(u+v).val() && (x-y).val()==(u-v).val() &&
          (x*y).val()==(u*v).val(),"ACL arithmetic");
    if(gcd(y.val(),M::mod())==1)
      check((x/y).val()==(u/v).val() && y.inv().val()==v.inv().val(),"ACL division");
  }
}

void streams(){
  istringstream input("-1 2 3 4 5 6 7 8 9 0 -13 a -b");
  vc<S> vector_values(2),zipped(2);
  deque<S> deque_values(2),zipped_deque(2);
  pair<S,S> p;
  input>>vector_values>>deque_values>>p;
  check(vector_values==vc<S>{12,2} && deque_values==deque<S>{3,4} && p==pair<S,S>{5,6},"nested input");
  auto* old_in=cin.rdbuf(input.rdbuf());
  cin_zip(zipped,zipped_deque);
  S direct=5;
  const bool read=bool(cin>>direct);
  cin.rdbuf(old_in);
  check(read && direct==0,"cin");
  check(zipped==vc<S>{7,9} && zipped_deque==deque<S>{8,0},"cin_zip");
  S x=5;
  input.clear();
  input>>hex>>x;
  check(x==10,"hex input");
  input>>x;
  check(x==2,"negative hex input");
  for(const char* bad:{"xyz","9223372036854775808","-9223372036854775809",""}){
    istringstream failed(bad);
    check(!(failed>>x) && x==2,"failed extraction preserves value");
  }
  istringstream limits("-9223372036854775808 9223372036854775807");
  pair<S,S> limit_pair;
  check(bool(limits>>limit_pair) &&
        limit_pair==pair<S,S>{numeric_limits<ll>::min(),numeric_limits<ll>::max()},"stream ll limits");
  ostringstream out;
  out<<hex<<showbase<<S(12)<<' '<<p;
  check(out.str()=="0xc (0x5, 0x6)","stream output flags and pair");
  ostringstream actual;
  auto* old_out=cout.rdbuf(actual.rdbuf());
  cout<<S(-1)<<'\n';
  show(S(-1),p);
  cout.rdbuf(old_out);
  check(actual.str()=="12\n12 (5, 6)\n","show");
  ostringstream dbg_output;
  auto* old_err=cerr.rdbuf(dbg_output.rdbuf());
  int calls=0;
  debug(vector_values);
  debug(tuple{p,deque_values});
  dbg(++calls,vector_values);
  cerr.rdbuf(old_err);
#ifdef LOCAL
  check(calls==1 && dbg_output.str()=="12 2 \n((5, 6), [3, 4])\n[++calls,vector_values] = 1, [12, 2]\n","LOCAL debug");
#else
  check(calls==0 && dbg_output.str().empty(),"non-LOCAL debug");
#endif
}

template<class M>
void libraries(){
  Combination<M> c(3);
  c.extend(8);
  check(c.C(8,3)==56 && c.P(5,2)==20 && c.fact(5)*c.inv_fact(5)==1,"Combination");
  FenwickTree<M> tree(vc<M>{2,5,7});
  tree.add(0,5);
  tree.set(1,9);
  check(tree.sum(0,2)==16 && tree.get(1)==9 && tree.all_sum()==23,"FenwickTree");
  PotentialUnionFind<M> uf(3);
  uf.merge(0,1,-1);
  uf.merge(1,2,4);
  check(uf.diff(0,2)==3 && uf.try_diff(2,0)==M(-3),"PotentialUnionFind");
  DualSegmentTreeAdd<M> dual(vc<M>{2,5,7});
  dual.apply(0,2,8);
  dual.apply(1,3,-4);
  check(dual.get(0)==10 && dual.get(1)==9 && dual.get(2)==3,"DualSegmentTree");
}

bool invalid(string_view mode){
  if(mode=="raw-negative") (void)S::raw(-1LL);
  else if(mode=="raw-bound") (void)S::raw(13);
  else if(mode=="raw-wrap") (void)D::raw((1ULL<<32)+1);
  else if(mode=="raw-wide") (void)S::raw((Wide(1)<<100)+1);
  else if(mode=="mod-zero") D::set_mod(0);
  else if(mode=="mod-negative") D::set_mod(-1LL);
  else if(mode=="mod-wrap") D::set_mod((1ULL<<32)+13);
  else if(mode=="mod-wide") D::set_mod((Wide(1)<<100)+13);
  else if(mode=="pow-negative") (void)S(1).pow(-1);
  else if(mode=="inverse") (void)static_modint<8>(4).inv();
  else if(mode=="division") (void)(static_modint<8>(3)/4);
  else return false;
  return true;
}

} // namespace

int main(int argc,char** argv){
  if(argc==2) return invalid(argv[1]) ? 0 : 2;
  check(D::mod()==998244353 && modint::mod()==998244353,"initial modulus");
  operations<static_modint<1>>();
  operations<static_modint<2>>();
  operations<static_modint<12>>();
  operations<modint998244353>();
  operations<modint1000000007>();
  operations<static_modint<2147483647>>();
  acl<modint998244353,atcoder::modint998244353>();
  acl<modint1000000007,atcoder::modint1000000007>();
  for(int m:{1,2,8,17,25,998244353,1000000007,2000001000,2147483646,2147483647}){
    D::set_mod(m);
    check(modint::mod()==998244353 && D::mod()==m,"ID independence");
    operations<D>();
    if(m<=2000001000){
      using A=atcoder::dynamic_modint<12>;
      A::set_mod(m);
      acl<D,A>();
    }
  }
  streams();
  libraries<modint998244353>();
  D::set_mod(998244353);
  libraries<D>();
  cout<<"modint tests passed\n";
}
