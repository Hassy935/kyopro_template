#include <Library/math/matrix.hpp>
#include <Library/io/debug.hpp>
#include <Library/math/modint.hpp>

namespace {

constexpr ull seed=0x4d4154524958;
mt19937_64 rng(seed);
int trial=0;

void check(bool ok,const char* message){
  if(ok) return;
  cerr<<"FAIL "<<message<<" seed="<<seed<<" trial="<<trial<<'\n';
  exit(1);
}

static_assert(is_same_v<decltype(declval<Matrix<ll>&>()[0]),span<ll>>);
static_assert(is_same_v<decltype(declval<const Matrix<ll>&>()[0]),span<const ll>>);

void shapes(){
  using M=Matrix<ll>;
  const M empty;
  check(empty==M(0,0) && empty==M(vvl{}) && M({})==empty,"default/empty input");
  check(M(0,4)!=empty && M(4,0)!=empty && M(0,4)!=M(4,0),"empty shapes");
  check(M(4,0)[3].empty() && static_cast<const M&>(M(4,0))[0].empty(),"zero-width row");
  check(M({{}, {}})==M(2,0) && M(vvl(4))==M(4,0),"zero-width lists");
  check(M(0,4).transpose()==M(4,0) && M(4,0).transpose()==M(0,4),"empty transpose");
  check(M(4,0)*M(0,3)==M(4,3) && M(0,4)*M(4,5)==M(0,5) &&
        M(3,4)*M(4,0)==M(3,0),"empty products");
  check(M(4,0)*vl{}==vl(4) && (M(0,4)*vl(4)).empty(),"empty column products");
  check(empty.pow(0)==empty && empty.pow(1'000'000)==empty,"empty power");
  check(M(0,numeric_limits<ll>::max()).transpose().height()==numeric_limits<ll>::max(),
        "large dimension with no elements");
  M a(3,4),b(3,4,5);
  check(a.height()==3 && a.width()==4 && a[2][3]==0 && b[2][3]==5,"size and fill");
  for(auto& x:a[2]) ++x;
  check(M(vvl{{0,0,0,0},{0,0,0,0},{1,1,1,1}})==a,"row iteration");
  const M id={{1,0},{0,1}};
  check(id==M::identity(2) && M::identity(0)==empty && M::identity(1)==M(1,1,1),"identity");
  M copy=b;
  copy=static_cast<const M&>(copy);
  check(copy==b,"self copy");
  auto& reference=copy;
  copy=std::move(reference);
  check(copy==b,"self move");
  M moved=std::move(copy);
  check(moved==b && copy==empty,"move construction");
  copy=id;
  check(copy==id,"copy assignment changes shape");
  copy=std::move(moved);
  check(copy==b && moved==empty,"move assignment changes shape");
  M zero_width(5,0);
  moved=std::move(zero_width);
  check(moved==M(5,0) && zero_width==empty,"move empty shape");
}

template<class T>
void arithmetic(){
  using M=Matrix<T>;
  M a={{1,2,3},{4,5,6}},b={{-1,-2,-3},{-4,-5,-6}};
  const M zero(2,3);
  check(-a==b && a+b==zero && a-zero==a,"add/subtract/negate");
  M change=a;
  check(&(change+=b)==&change && change==zero && &(change-=b)==&change && change==a,"compound +-");
  check(a*2==2*a && a*2==M({{2,4,6},{8,10,12}}),"mixed scalars");
  change*=change[0][1];
  check(change==2*a,"scalar aliases element");
  M r={{2,1},{1,2},{1,1}}, product={{7,8},{19,20}};
  check(a*r==product && a*vc<T>{2,1,1}==vc<T>{7,19},"rectangle/matrix-column");
  change=a;
  check(&(change*=r)==&change && change==product,"shape-changing product assignment");
  change*=change;
  check(change==product*product,"self product");
  check(a.transpose()==M({{1,4},{2,5},{3,6}}) &&
        (a*r).transpose()==r.transpose()*a.transpose(),"transpose product");
  check(a*M::identity(3)==a && M::identity(2)*a==a && a*M(3,2)==M(2,2),"identity and zero products");
  check(product.pow(0)==M::identity(2) && product.pow(1)==product &&
        product.pow(2)==product*product,"power and source intact");
  check(a!=r && a!=zero && M({{1}})*M({{2}})==M(1,1,2),"comparison / scalar shape");
  check(M(1,1)*M(1,4)==M(1,4) && M(4,1)*M(1,1)==M(4,1),"one row / column");
}

struct OrderedProduct {
  int value=0;
  friend OrderedProduct operator*(OrderedProduct a,OrderedProduct b){
    return {10*a.value+b.value};
  }
  bool operator==(const OrderedProduct&) const=default;
};

void special_arithmetic(){
  const Matrix<OrderedProduct> a(1,2,{3});
  check((a*OrderedProduct{2})[0][0].value==32 &&
        (OrderedProduct{2}*a)[0][0].value==23,"scalar operand order");
  Matrix<ll> aliased{{2,3},{4,5}};
  const ll& scalar=aliased[0][0];
  auto scaled=scalar*std::move(aliased);
  check(scaled==Matrix<ll>{{4,6},{8,10}},"left scalar aliases moved matrix");
  const Matrix<ll> big(1,1,numeric_limits<ll>::max());
  check(big.pow(1)==big,"no unnecessary squaring");
  auto x=Matrix<ld>{{0.1L,0.2L}}*Matrix<ld>{{0.2L},{0.3L}};
  check(abs(x[0][0]-0.08L)<1e-15L,"floating point tolerance");
  check(Matrix<ld>(1,1,1)!=Matrix<ld>(1,1,nextafter(1.0L,2.0L)),"exact equality");
  check(isnan((Matrix<ld>(1,1,0)*Matrix<ld>(1,1,numeric_limits<ld>::infinity()))[0][0]),
        "zero times infinity is evaluated");
}

template<class T>
Matrix<T> reference(const Matrix<T>& a,const Matrix<T>& b){
  vector<vector<T>> out(a.height(),vector<T>(b.width()));
  for(ll i=0;i<a.height();++i)
    for(ll j=0;j<b.width();++j){
      T value{};
      for(ll k=0;k<a.width();++k) value=value+a[i][k]*b[k][j];
      out[i][j]=value;
    }
  if(out.empty()) return Matrix<T>(0,b.width());
  return Matrix<T>(out);
}

void exhaustive(){
  vector<Matrix<ll>> all;
  for(int mask=0;mask<81;++mask){
    Matrix<ll> a(2,2);
    int digits=mask;
    for(int i=0;i<4;++i){
      a[i/2][i%2]=digits%3-1;
      digits/=3;
    }
    all.push_back(a);
  }
  for(const auto& a:all)
    for(const auto& b:all) check(a*b==reference(a,b),"2x2 exhaustive");
}

template<class T>
void randomized(){
  auto make=[](ll h,ll w){
    Matrix<T> a(h,w);
    for(ll i=0;i<h;++i)
      for(auto& x:a[i]) x=T(ll(rng()%11)-5);
    return a;
  };
  for(trial=0;trial<300;++trial){
    ll h=rng()%9,k=rng()%9,w=rng()%9,l=rng()%9;
    auto a=make(h,k),b=make(k,w),c=make(w,l);
    check(a*b==reference(a,b),"random rectangle reference");
    check((a*b)*c==a*(b*c),"rectangle associativity");
    check(a.transpose().transpose()==a,"double transpose");
  }
}

template<class T>
pair<T,T> fib(ll n){
  if(n==0) return {0,1};
  auto [a,b]=fib<T>(n/2);
  T c=a*(2*b-a),d=a*a+b*b;
  return (n&1) ? pair<T,T>{d,c+d} : pair<T,T>{c,d};
}

template<class T>
void powers(){
  const Matrix<T> a={{1,1},{1,0}};
  for(ll n:{0LL,1LL,2LL,20LL,1'000'000'000'000'000'000LL}){
    auto p=a.pow(n);
    check(p[0][1]==fib<T>(n).first && p[0][0]==fib<T>(n).second,"Fibonacci powers");
  }
}

struct Counted {
  inline static int copies=0;
  int n=7;
  Counted()=default;
  Counted(const Counted& other): n(other.n){ ++copies; }
  friend ostream& operator<<(ostream& os,const Counted& x){ return os<<x.n; }
};

void streams(){
  Matrix<ll> a(2,2,9);
  istringstream in("1 2 3 4 7");
  auto* previous_in=cin.rdbuf(in.rdbuf());
  ll tail=0;
  const bool read=bool(cin>>a>>tail);
  cin.rdbuf(previous_in);
  check(read && a==Matrix<ll>({{1,2},{3,4}}) && tail==7,"cin row-major and chained");
  Matrix<ll> bad(2,2,9);
  istringstream broken("3 z 4");
  check(!(broken>>bad) && bad.height()==2 && bad.width()==2 &&
        bad[0][0]==3 && bad[1][0]==9,"failed input");
  ll value=0;
  Matrix<ll> empty(3,0);
  istringstream token("8");
  check(bool(token>>empty>>value) && value==8,"empty input consumes nothing");
  ostringstream out;
  auto* previous_out=cout.rdbuf(out.rdbuf());
  cout<<a<<'\n';
  show(a);
  cout<<Matrix<ll>(0,3)<<empty<<Matrix<ll>{};
  cout.rdbuf(previous_out);
  check(out.str()=="1 2\n3 4\n1 2\n3 4\n","cout/show/empty output");
  Matrix<modint> d(1,2);
  istringstream modular("-1 12");
  ostringstream result;
  modular>>d;
  result<<d;
  check(result.str()=="16 12","modint stream");
}

void debugging(){
  const Matrix<ll> a={{1,2},{3,4}},row={{5,6}},col={{7},{8}};
  [[maybe_unused]] const Matrix<modint998244353> s={{-1}};
  [[maybe_unused]] const Matrix<modint> d={{-1}};
  const Matrix<Counted> counted(2,2);
  Counted::copies=0;
  auto view=counted.debug_view();
  check(view.size()==2 && view[0].data()==counted[0].data() && Counted::copies==0,"view aliases elements");
  ostringstream out;
  auto* old=cerr.rdbuf(out.rdbuf());
  int calls=0;
  debug(a);
  debug(row);
  debug(col);
  debug(Matrix<ll>(0,4));
  debug(Matrix<ll>(2,0));
  dbg(a,row);
  dbg(vector{a},pair{row,tuple{col}});
  dbg(s,d);
  debug(counted);
  dbg((++calls,a));
  cerr.rdbuf(old);
#ifdef LOCAL
  check(calls==1 && out.str()==
      "1 2 \n3 4 \n5 6 \n7 \n8 \n\n\n"
      "[a,row] = [[1, 2], [3, 4]], [[5, 6]]\n"
      "[vector{a},pair{row,tuple{col}}] = [[[1, 2], [3, 4]]], ([[5, 6]], ([[7], [8]]))\n"
      "[s,d] = [[998244352]], [[16]]\n7 7 \n7 7 \n"
      "[(++calls,a)] = [[1, 2], [3, 4]]\n","LOCAL formatting");
#else
  check(calls==0 && out.str().empty(),"non-LOCAL inert");
#endif
  check(Counted::copies==0,"debug copies no elements");
}

bool invalid(string_view which){
  using M=Matrix<ll>;
  if(which=="negative") (void)M(-1,0);
  else if(which=="overflow") (void)M(1LL<<40,1LL<<40);
  else if(which=="max-size") (void)M(numeric_limits<ll>::max(),1);
  else if(which=="list") (void)M({{1},{1,2}});
  else if(which=="vector") (void)M(vvl{{1},{1,2}});
  else if(which=="row") (void)M(1,0)[1];
  else if(which=="add") (void)(M(2,0)+M(3,0));
  else if(which=="sub") (void)(M(2,0)-M(3,0));
  else if(which=="mul") (void)(M(2,1)*M(2,1));
  else if(which=="column") (void)(M(2,2)*vl(3));
  else if(which=="rectangular") (void)M(2,3).pow(0);
  else if(which=="exponent") (void)M(2,2).pow(-1);
  else return false;
  return true;
}

} // namespace

int main(int argc,char** argv){
  if(argc==2) return invalid(argv[1]) ? 0 : 2;
  modint::set_mod(17);
  shapes();
  special_arithmetic();
  exhaustive();
  arithmetic<ll>(); randomized<ll>();
  arithmetic<ld>(); randomized<ld>();
  arithmetic<modint998244353>(); randomized<modint998244353>(); powers<modint998244353>();
  arithmetic<modint1000000007>(); powers<modint1000000007>();
  arithmetic<modint>(); randomized<modint>(); powers<modint>();
  streams();
  debugging();
  cout<<"matrix tests passed\n";
}
