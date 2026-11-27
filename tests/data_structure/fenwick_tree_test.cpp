#include <Library/data_structure/fenwick_tree.hpp>
#include <atcoder/modint>

namespace {
constexpr ull seed=0xF311CULL;
constexpr ll sizes[]={0,1,2,3,4,7,8,9,16,17,32,33};
using Mint=atcoder::modint998244353;

static_assert(is_same_v<fenwick_tree<>,FenwickTree<ll>>);
static_assert(is_same_v<fenwick_tree<i128>,FenwickTree<i128>>);
static_assert(is_same_v<fenwick_tree<Mint>,FenwickTree<Mint>>);

void check(bool ok,const char* api,ll n,ll op=-1,ll l=-1,ll r=-1){
  if(ok) return;
  cerr << api << " failed: seed=" << seed << " n=" << n << " op=" << op
       << " l=" << l << " r=" << r << '\n';
  exit(1);
}

template<class T>
void verify(const FenwickTree<T>& tree,const vector<T>& a,ll op=-1){
  const ll n=ssize(a);
  check(tree.size()==n,"size",n,op);
  T prefix{};
  for(ll r=0;r<=n;r++){
    check(tree.sum(r)==prefix,"sum(r)",n,op,r);
    if(r<n){
      check(tree.get(r)==a[r],"get",n,op,r);
      prefix+=a[r];
    }
  }
  check(tree.all_sum()==prefix,"all_sum",n,op);
  for(ll l=0;l<=n;l++){
    T range{};
    for(ll r=l;r<=n;r++){
      check(tree.sum(l,r)==range,"sum(l,r)",n,op,l,r);
      if(r<n) range+=a[r];
    }
  }
}

template<class T>
void arithmetic(T scale){
  mt19937_64 rng(seed);
  for(ll n:sizes){
    vector<T> a(n);
    verify(FenwickTree<T>(n),a);
    for(ll p=0;p<n;p++) a[p]=T(p%9-4)*scale;
    FenwickTree<T> tree(a);
    verify(tree,a);
    for(ll op=0;n>0 && op<200;op++){
      const ll p=rng()%n;
      const T x=T(ll(rng()%41)-20)*scale;
      if(rng()%2){
        tree.add(p,x);
        a[p]+=x;
      }else{
        tree.set(p,x);
        a[p]=x;
      }
      verify(tree,a,op);
    }
  }
}

template<class T>
void verify_search(const FenwickTree<T>& tree,const vector<T>& a,T x,ll op=-1){
  const ll n=ssize(a);
  ll lower=n,upper=n;
  T prefix{};
  for(ll p=0;p<n;p++){
    prefix+=a[p];
    if(lower==n && prefix>=x) lower=p;
    if(upper==n && prefix>x) upper=p;
  }
  check(tree.lower_bound(x)==lower,"lower_bound",n,op);
  check(tree.upper_bound(x)==upper,"upper_bound",n,op);
}

template<class T>
void search(T scale){
  mt19937_64 rng(seed);
  for(ll n:sizes){
    vector<T> a(n);
    FenwickTree<T> tree(n);
    for(ll x:{-1,0,1}) verify_search(tree,a,T(x)*scale);
    for(ll op=0;n>0 && op<200;op++){
      const ll p=rng()%n;
      const T x=T(rng()%4)*scale;
      if(rng()%2) tree.add(p,x-a[p]);
      else tree.set(p,x);
      a[p]=x;

      T prefix{};
      verify_search(tree,a,-scale,op);
      verify_search(tree,a,prefix,op);
      for(const T& value:a){
        prefix+=value;
        verify_search(tree,a,prefix,op);
        verify_search(tree,a,prefix+1,op);
      }
    }
  }
  const T largest=numeric_limits<T>::max();
  const vector<T> a={0,0,largest,0,0};
  const FenwickTree<T> tree(a);
  verify_search(tree,a,T{});
  verify_search(tree,a,largest);
  verify_search(FenwickTree<T>(3),vector<T>(3),largest);
}

struct Counted {
  ll value=0;
  static inline ll additions=0;
  Counted& operator+=(const Counted& other){
    ++additions;
    value+=other.value;
    return *this;
  }
};

void linear_build(){
  for(ll n:{1024,1025}){
    Counted::additions=0;
    const FenwickTree<Counted> tree(vector<Counted>(n,Counted{1}));
    check(Counted::additions<=n,"linear build",n);
    check(tree.all_sum().value==n,"linear build sum",n);
  }
}
}

int main(){
  linear_build();
  arithmetic(1LL);
  arithmetic(i128(1)<<75);
  arithmetic(Mint(998244351));
  search(1LL);
  search(i128(1)<<75);
  cout << "FenwickTree tests passed.\n";
}
