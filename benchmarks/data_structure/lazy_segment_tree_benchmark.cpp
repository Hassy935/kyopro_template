#include <Library/data_structure/lazy_segment_tree.hpp>
#include <Library/math/modint.hpp>
#include <atcoder/lazysegtree>

namespace {
using Clock=chrono::steady_clock;
constexpr ull seed=0x4c415a5942454e43;
constexpr int repetitions=5,queries=100000;
template<class T> struct Sum {T value; ll length;};
template<class T> Sum<T> combine(Sum<T> a,Sum<T> b){return {a.value+b.value,a.length+b.length};}
template<class T> Sum<T> empty_sum(){return {0,0};}
ll add(ll f,ll g){return f+g;}
ll zero(){return 0;}
Sum<ll> add_sum(ll f,Sum<ll> x){return {x.value+f*x.length,x.length};}
ll low(ll a,ll b){return min(a,b);}
ll infinity(){return 1LL<<60;}
ll add_min(ll f,ll x){return x==infinity()?x:f+x;}
using Assign=optional<ll>;
Assign assign(Assign f,Assign g){return f?f:g;}
Assign no_assign(){return nullopt;}
Sum<ll> assign_sum(Assign f,Sum<ll> x){return {f ? *f*x.length : x.value,x.length};}
using Mint=modint998244353;
struct Affine {Mint a,b;};
Affine compose(Affine f,Affine g){return {f.a*g.a,f.a*g.b+f.b};}
Affine identity(){return {1,0};}
Sum<Mint> affine_sum(Affine f,Sum<Mint> x){return {f.a*x.value+f.b*x.length,x.length};}
ull code(ll x){return ull(x);}
ull code(Sum<ll> x){return ull(x.value);}
ull code(Sum<Mint> x){return ull(x.value.val());}

struct Query {int l,r; ll value; bool update;};

template<class A,class B>
void measure(string_view label,int n,int count,A own,B acl){
  optional<ull> expected;
  array<double,repetitions> a{},b{};
  for(int i=0;i<repetitions;++i){
    pair<double,ull> x,y;
    if(i%2==0){x=own();y=acl();}
    else {y=acl();x=own();}
    if(!expected) expected=x.second;
    if(x.second!=*expected || y.second!=*expected) throw runtime_error("lazy benchmark checksum");
    a[i]=x.first/count; b[i]=y.first/count;
  }
  ranges::sort(a); ranges::sort(b);
  cout<<n<<' '<<label<<' '<<count<<' '<<a[2]<<' '<<b[2]<<' '<<*expected<<'\n';
}

template<class T>
void observe(const T& x){ asm volatile("" : : "g"(&x) : "memory"); }

template<class S,auto op,auto e,class F,auto mapping,auto composition,auto id,class Execute>
void compare(string_view label,const vector<S>& values,const vector<Query>& trace,Execute execute){
  using Own=LazySegmentTree<S,op,e,F,mapping,composition,id>;
  using ACL=atcoder::lazy_segtree<S,op,e,F,mapping,composition,id>;
  auto timed=[&]<class Tree>(){
    Tree t(values);
    ull checksum=0;
    auto start=Clock::now();
    for(Query q:trace) checksum+=execute(t,q);
    observe(t);
    auto stop=Clock::now();
    return pair{chrono::duration<double,nano>(stop-start).count(),checksum+code(t.all_prod())};
  };
  measure(label,int(values.size()),int(trace.size()),[&]{return timed.template operator()<Own>();},
          [&]{return timed.template operator()<ACL>();});
}

using Own=LazySegmentTree<Sum<ll>,combine<ll>,empty_sum<ll>,ll,add_sum,add,zero>;
using ACL=atcoder::lazy_segtree<Sum<ll>,combine<ll>,empty_sum<ll>,ll,add_sum,add,zero>;

void special(const vector<Sum<ll>>& values,const vector<Query>& trace){
  int n=int(values.size()),builds=max(3,1'000'000/n);
  auto build=[&]<class Tree>(){
    ull checksum=0;
    double elapsed=0;
    for(int i=0;i<builds;++i){
      auto start=Clock::now();
      Tree tree(values);
      observe(tree);
      elapsed+=chrono::duration<double,nano>(Clock::now()-start).count();
      checksum+=code(tree.all_prod());
    }
    return pair{elapsed,checksum};
  };
  measure("build",n,builds,[&]{return build.template operator()<Own>();},[&]{return build.template operator()<ACL>();});
  auto materialize=[&]<class Tree>(){
    Tree tree(values);
    for(Query q:trace) if(q.update) tree.apply(q.l,q.r,q.value);
    auto start=Clock::now();
    auto out=[&]{
      if constexpr(is_same_v<Tree,Own>) return as_const(tree).to_vector();
      else{
        vector<Sum<ll>> result;
        result.reserve(n);
        for(int i=0;i<n;++i) result.push_back(tree.get(i));
        return result;
      }
    }();
    auto stop=Clock::now();
    ull checksum=0;
    for(int i=0;i<n;++i) checksum+=ull(i+1)*code(out[i]);
    return pair{chrono::duration<double,nano>(stop-start).count(),checksum};
  };
  measure("to-vector-vs-get",n,n,[&]{return materialize.template operator()<Own>();},
          [&]{return materialize.template operator()<ACL>();});
}

vector<Query> sequence(int n,string_view pattern,int update_rate){
  mt19937_64 rng(seed);
  vector<Query> trace(queries);
  for(auto& q:trace){
    q.l=int(rng()%n);q.r=int(rng()%n);
    if(q.l>q.r) swap(q.l,q.r);
    ++q.r;
    if(pattern=="short") q.r=min(n,q.l+1+int(rng()%16));
    if(pattern=="long"){q.l/=4;q.r=n-(n-q.r)/4;}
    if(pattern=="full"){q.l=0;q.r=n;}
    q.value=1+ll(rng()%16);
    q.update=int(rng()%100)<update_rate;
  }
  return trace;
}

void run(int n){
  vector<Sum<ll>> values(n);
  vector<Sum<Mint>> modular(n);
  vl raw(n);
  for(int i=0;i<n;++i){
    raw[i]=1+i%71;
    values[i]={raw[i],1}; modular[i]={raw[i],1};
  }
  auto trace=sequence(n,"random",60);
  auto range_sum=[](auto& t,Query q){
    if(q.update){t.apply(q.l,q.r,q.value);return 0ULL;}
    return code(t.prod(q.l,q.r));
  };
  special(values,trace);
  compare<Sum<ll>,combine<ll>,empty_sum<ll>,ll,add_sum,add,zero>("point-set-get",values,trace,
      [](auto& t,Query q){if(q.update) t.set(q.l,{q.value,1});return code(t.get(q.l));});
  compare<Sum<ll>,combine<ll>,empty_sum<ll>,ll,add_sum,add,zero>("add-sum-random60",values,trace,range_sum);
  compare<Sum<ll>,combine<ll>,empty_sum<ll>,ll,add_sum,add,zero>(
      "add-sum-short10",values,sequence(n,"short",10),range_sum);
  compare<Sum<ll>,combine<ll>,empty_sum<ll>,ll,add_sum,add,zero>(
      "add-sum-full90",values,sequence(n,"full",90),range_sum);
  compare<ll,low,infinity,ll,add_min,add,zero>("add-min-long90",raw,sequence(n,"long",90),range_sum);
  compare<Sum<ll>,combine<ll>,empty_sum<ll>,Assign,assign_sum,assign,no_assign>("assign-sum",values,trace,
      [](auto& t,Query q){if(q.update){t.apply(q.l,q.r,Assign(q.value));return 0ULL;}return code(t.prod(q.l,q.r));});
  compare<Sum<Mint>,combine<Mint>,empty_sum<Mint>,Affine,affine_sum,compose,identity>(
      "affine-sum",modular,trace,[](auto& t,Query q){
        if(q.update){t.apply(q.l,q.r,Affine{q.value+1,q.value});return 0ULL;}
        return code(t.prod(q.l,q.r));});
  compare<Sum<ll>,combine<ll>,empty_sum<ll>,ll,add_sum,add,zero>("add-search",values,trace,
      [](auto& t,Query q){
        if(q.update){t.apply(q.l,q.r,q.value);return 0ULL;}
        auto pred=[&](Sum<ll> x){return x.value<=q.value*1024;};
        return ull(t.max_right(q.l,pred))+ull(t.min_left(q.r,pred));
      });
}
}

int main(){
  cout<<fixed<<setprecision(3)<<"seed "<<seed<<" repeats "<<repetitions<<'\n';
  cout<<"sizeof "<<sizeof(Own)<<' '<<sizeof(ACL)<<'\n';
  cout<<"N operation repetitions own_ns ACL_ns checksum\n";
  for(int n:{1024,65536,262144,262145,1000000}) run(n);
}
