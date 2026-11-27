#include <Library/math/modint.hpp>
#include <Library/math/combination.hpp>
#include <atcoder/modint>

namespace {
using Clock=chrono::steady_clock;
constexpr int N=1<<20, slow=1<<17, repeats=5, fac_n=1<<20;
constexpr int fixed_mod=998244353;
constexpr ull seed=0x4D4F44494E54;

struct Input {
  vc<ll> narrow;
  vc<i128> wide;
  vc<int> a,b,exponent;
  vc<pair<int,int>> queries;
  explicit Input(int m){
    mt19937_64 rng(seed);
    for(int i=0;i<N;++i){
      narrow.push_back(bit_cast<ll>(rng()));
      wide.push_back(i128(narrow.back())*(i128(1)<<61)+rng());
      a.push_back(int(rng()%m));
      b.push_back(1+int(rng()%(m-1)));
      exponent.push_back(int(rng()%4096));
      int n=int(rng()%(fac_n+1));
      queries.emplace_back(n,int(rng()%(n+1)));
    }
  }
};

template<class M>
ull checksum(const vc<M>& v,int count){
  ull result=0;
  for(int i=0;i<count;++i) result+=uint32_t(v[i].val())*ull(i+1);
  return result;
}

template<class F>
auto timed(F&& f){
  auto start=Clock::now();
  f();
  return chrono::duration<double,nano>(Clock::now()-start).count();
}

template<class A,class B>
void measure(string_view group,string_view operation,int count,A first,B second){
  array<double,repeats> a{},b{};
  ull expected=0;
  for(int i=0;i<repeats;++i){
    pair<double,ull> x,y;
    if(i%2==0){ x=first(); y=second(); }
    else { y=second(); x=first(); }
    if(i==0) expected=x.second;
    if(x.second!=expected || y.second!=expected){
      cerr<<"checksum mismatch "<<group<<' '<<operation<<'\n';
      exit(1);
    }
    a[i]=x.first/count;
    b[i]=y.first/count;
  }
  ranges::sort(a);
  ranges::sort(b);
  cout<<group<<' '<<operation<<' '<<count<<' '<<a[2]<<' '<<b[2]<<' '<<expected<<'\n';
}

template<class M,class A>
void suite(string_view group,const Input& in){
  vc<M> x(N),y(N),out(N);
  vc<A> ax(N),ay(N),aout(N);
  for(int i=0;i<N;++i){
    x[i]=M::raw(in.a[i]); y[i]=M::raw(in.b[i]);
    ax[i]=A::raw(in.a[i]); ay[i]=A::raw(in.b[i]);
  }
  auto loops=[&](string_view name,int count,auto op){
    auto run=[&](auto& dest,const auto& left,const auto& right){
      double ns=timed([&]{ for(int i=0;i<count;++i) dest[i]=op(left[i],right[i],i); });
      return pair{ns,checksum(dest,count)};
    };
    measure(group,name,count,[&]{return run(out,x,y);},[&]{return run(aout,ax,ay);});
  };
  loops("construct64",N,[&](auto l,auto,int i){ return decltype(l)(in.narrow[i]); });
  loops("construct128",N,[&](auto l,auto,int i){ return decltype(l)(in.wide[i]); });
  loops("array-add",N,[](auto l,auto r,int){ return l+r; });
  loops("array-sub",N,[](auto l,auto r,int){ return l-r; });
  loops("array-mul",N,[](auto l,auto r,int){ return l*r; });
  loops("array-div",slow,[](auto l,auto r,int){ return l/r; });
  loops("array-pow",slow,[&](auto l,auto,int i){ return l.pow(in.exponent[i]); });
  loops("array-inv",slow,[](auto,auto r,int){ return r.inv(); });
  auto chains=[&](string_view name,int count,auto op){
    auto run=[&](const auto& rhs){
      using T=typename remove_cvref_t<decltype(rhs)>::value_type;
      T value=rhs.front();
      double ns=timed([&]{ for(int i=0;i<count;++i) value=op(value,rhs[i],i); });
      return pair{ns,ull(value.val())};
    };
    measure(group,name,count,[&]{return run(y);},[&]{return run(ay);});
  };
  chains("chain-add",N,[](auto l,auto r,int){return l+r;});
  chains("chain-sub",N,[](auto l,auto r,int){return l-r;});
  chains("chain-mul",N,[](auto l,auto r,int){return l*r;});
  chains("chain-div",slow,[](auto l,auto r,int){return l/r;});

  chains("chain-pow-mul",slow,[&](auto l,auto r,int i){return l.pow(in.exponent[i])*r;});
  chains("chain-inv-mul",slow,[](auto l,auto r,int){return (l*r).inv();});

  auto build=[]<class T>(type_identity<T>){
    optional<Combination<T>> c;
    double ns=timed([&]{c.emplace(fac_n);});
    return pair{ns,ull(c->fact(fac_n).val())};
  };
  auto extend=[]<class T>(type_identity<T>){
    optional<Combination<T>> c;
    double ns=timed([&]{
      c.emplace();
      for(int n=1;n<=fac_n;n*=2) c->extend(n);
    });
    return pair{ns,ull(c->fact(fac_n).val())};
  };
  measure(group,"comb-build",1,[&]{return build(type_identity<M>{});},[&]{return build(type_identity<A>{});});
  measure(group,"comb-extend",1,[&]{return extend(type_identity<M>{});},[&]{return extend(type_identity<A>{});});
  Combination<M> comb(fac_n);
  Combination<A> acl_comb(fac_n);
  auto query=[&](auto& c,auto& dest){
    double ns=timed([&]{
      for(int i=0;i<N;++i) dest[i]=c.C(in.queries[i].first,in.queries[i].second);
    });
    return pair{ns,checksum(dest,N)};
  };
  measure(group,"comb-query",N,[&]{return query(comb,out);},[&]{return query(acl_comb,aout);});
}
} // namespace

int main(int argc,char** argv){
  // 実行時に読み取り，動的法を定数畳み込みさせない．
  int m=stoi(argc>1?argv[1]:"1000000007");
  if(m!=1000000007 && m!=998244353) return 2;
  using A=atcoder::dynamic_modint<0>;
  modint::set_mod(m);
  A::set_mod(m);
  cout<<fixed<<setprecision(3);
  cout<<"seed "<<seed<<" repeats "<<repeats<<" static_mod "<<fixed_mod
      <<" dynamic_mod "<<m<<" fac_n "<<fac_n<<'\n';
  cout<<"sizeof "<<sizeof(modint998244353)<<' '<<sizeof(atcoder::modint998244353)
      <<' '<<sizeof(modint)<<' '<<sizeof(A)<<'\n';
  cout<<"type operation count own_ns/op ACL_ns/op checksum\n";
  suite<modint998244353,atcoder::modint998244353>("static",Input(fixed_mod));
  suite<modint,A>("dynamic",Input(m));
}
