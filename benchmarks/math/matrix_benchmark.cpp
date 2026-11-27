#include <Library/math/matrix.hpp>
#include <Library/math/modint.hpp>

namespace {
using Clock=chrono::steady_clock;
constexpr ull seed=0x4d4154524958;
constexpr int repeats=5, pool=8;

template<class T>
ull residue(T x){
  if constexpr(is_same_v<T,ll>) return ull(x);
  else return ull(x.val());
}

template<class T>
ull sample(const Matrix<T>& result,int i){
  const auto row=result[i%result.height()];
  asm volatile("" : : "g"(row.data()) : "memory");
  return residue(row[i%row.size()]);
}

template<class T>
ull sample(const vector<T>& result,int i){
  asm volatile("" : : "g"(result.data()) : "memory");
  return residue(result[i%result.size()]);
}

template<class F>
void measure(string_view type,string_view op,int h,int n,int w,int iterations,F work){
  array<double,repeats> samples{};
  optional<ull> expected;
  for(int rep_no=0;rep_no<repeats;++rep_no){
    ull checksum=0;
    auto start=Clock::now();
    for(int i=0;i<iterations;++i) checksum+=sample(work(i%pool),i);
    auto end=Clock::now();
    if(!expected) expected=checksum;
    if(checksum!=*expected){
      cerr<<"benchmark checksum mismatch "<<type<<' '<<op<<'\n';
      exit(1);
    }
    samples[rep_no]=chrono::duration<double,nano>(end-start).count()/iterations;
  }
  ranges::sort(samples);
  cout<<type<<' '<<op<<' '<<h<<'x'<<n<<'x'<<w<<' '<<iterations<<' '<<samples[2]<<' '<<*expected<<'\n';
}

template<class T>
Matrix<T> make(int h,int w,mt19937_64& rng,bool sparse){
  Matrix<T> m(h,w);
  for(int i=0;i<h;++i)
    for(auto& v:m[i]){
      bool zero=sparse && rng()%100<90;
      v=zero ? T{} : T(1+rng()%100);
    }
  return m;
}

template<class T>
void multiply(string_view type,int h,int n,int w,bool sparse){
  mt19937_64 rng(seed);
  array<Matrix<T>,pool> a,b;
  for(int i=0;i<pool;++i){
    a[i]=make<T>(h,n,rng,sparse);
    b[i]=make<T>(n,w,rng,sparse);
  }
  measure(type,sparse?"multiply-sparse":"multiply",h,n,w,max(1,8'388'608/(h*n*w)),
      [&](int i){return a[i]*b[i];});
}

template<class T>
void run(string_view type){
  for(int n:{2,4,16,32,64,128,256}) multiply<T>(type,n,n,n,false);
  multiply<T>(type,64,96,48,false);
  multiply<T>(type,128,128,128,true);
  for(int n:{2,16,64,256}){
    mt19937_64 rng(seed);
    array<Matrix<T>,pool> a;
    array<vc<T>,pool> v;
    for(int i=0;i<pool;++i){
      a[i]=make<T>(n,n,rng,false);
      v[i].resize(n);
      for(auto& x:v[i]) x=T(1+rng()%100);
    }
    int iterations=max(1,2'097'152/(n*n));
    measure(type,"column",n,n,1,iterations,[&](int i){return a[i]*v[i];});
    measure(type,"transpose",n,n,n,iterations,[&](int i){return a[i].transpose();});
  }
  for(int n:{2,4,16,32,64}){
    mt19937_64 rng(seed);
    array<Matrix<T>,pool> a;
    for(auto& x:a){
      x=make<T>(n,n,rng,false);
      if constexpr(is_same_v<T,ll>){
        // 置換行列なら高い累乗でも整数があふれない．
        x=Matrix<T>(n,n);
        vc<int> indices(n);
        iota(indices.begin(),indices.end(),0);
        shuffle(indices.begin(),indices.end(),rng);
        for(int i=0;i<n;++i) x[i][indices[i]]=1;
      }
    }
    constexpr ll exponent=65'537;
    measure(type,"pow",n,n,n,max(1,4'194'304/(17*n*n*n)),
        [&](int i){return a[i].pow(exponent);});
  }
}
} // namespace

int main(int argc,char** argv){
  modint::set_mod(stoll(argc>1 ? argv[1] : "1000000007"));
  cout<<"seed "<<seed<<" repeats "<<repeats<<" dynamic-mod "<<modint::mod()<<" pow-exponent 65537\n";
  cout<<"type operation dimensions iterations ns/op checksum\n"<<fixed<<setprecision(2);
  run<ll>("ll");
  run<modint998244353>("static");
  run<modint>("dynamic");
}
