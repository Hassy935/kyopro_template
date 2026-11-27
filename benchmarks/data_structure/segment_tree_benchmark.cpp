#include <Library/data_structure/segment_tree.hpp>
#include <atcoder/segtree>

namespace {
using Clock=chrono::steady_clock;
constexpr int repetitions=5, queries=100000;
constexpr ull seed=0x53454742454e4348;
ll sum(ll a,ll b){return a+b;}
ll zero(){return 0;}
ll low(ll a,ll b){return min(a,b);}
ll infinity(){return numeric_limits<ll>::max();}
struct Affine {uint32_t a,b;};
Affine compose(Affine a,Affine b){
  return {uint32_t(b.a*a.a),uint32_t(b.a*a.b+b.b)};
}
Affine identity(){return {1,0};}
ull code(ll x){return ull(x);}
ull code(Affine x){return (ull(x.a)<<32)|x.b;}

struct Query {int l,r,p; ll x;};

template<class A,class B>
void measure(string_view name,int n,int count,A own,B acl){
  array<double,repetitions> a{},b{};
  optional<ull> expected;
  for(int i=0;i<repetitions;++i){
    pair<double,ull> u,v;
    if(i%2==0){u=own();v=acl();}
    else {v=acl();u=own();}
    if(!expected) expected=u.second;
    if(u.second!=*expected || v.second!=*expected) throw runtime_error("benchmark checksum");
    a[i]=u.first/count; b[i]=v.first/count;
  }
  ranges::sort(a); ranges::sort(b);
  cout<<n<<' '<<name<<' '<<count<<' '<<a[2]<<' '<<b[2]<<' '<<*expected<<'\n';
}

template<class T>
void observe(const T& object){ asm volatile("" : : "g"(&object) : "memory"); }

template<class S,auto op,auto e,class F>
void compare(string_view name,const vector<S>& values,const vector<Query>& trace,F execute){
  using Own=SegmentTree<S,op,e>;
  using ACL=atcoder::segtree<S,op,e>;
  auto timed=[&]<class Tree>(){
    Tree tree(values);
    ull checksum=0;
    auto start=Clock::now();
    for(const Query& q:trace) checksum+=execute(tree,q);
    observe(tree);
    auto stop=Clock::now();
    return pair{chrono::duration<double,nano>(stop-start).count(),checksum+code(tree.all_prod())};
  };
  measure(name,int(values.size()),int(trace.size()),[&]{return timed.template operator()<Own>();},
          [&]{return timed.template operator()<ACL>();});
}

void run(int n){
  using Own=SegmentTree<ll,sum,zero>;
  using ACL=atcoder::segtree<ll,sum,zero>;
  mt19937_64 rng(seed);
  vl values(n);
  for(auto& x:values) x=1+ll(rng()%100);
  vector<Affine> functions(n);
  for(auto& x:functions) x={uint32_t(rng())|1U,uint32_t(rng())};
  vector<Query> trace(queries);
  for(auto& q:trace){
    q.l=int(rng()%n); q.r=1+int(rng()%n);
    if(q.l>q.r) swap(q.l,q.r);
    q.p=int(rng()%n); q.x=1+ll(rng()%200);
  }
  int builds=max(4,2'000'000/n);
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
  compare<ll,sum,zero>("get",values,trace,[](auto& tree,const Query& q){return code(tree.get(q.p));});
  compare<ll,sum,zero>("set",values,trace,[](auto& tree,const Query& q){tree.set(q.p,q.x);return 0ULL;});
  compare<ll,sum,zero>("sum",values,trace,[](auto& tree,const Query& q){return code(tree.prod(q.l,q.r));});
  compare<ll,low,infinity>("min",values,trace,[](auto& tree,const Query& q){return code(tree.prod(q.l,q.r));});
  compare<Affine,compose,identity>("affine-prod",functions,trace,
      [](auto& tree,const Query& q){return code(tree.prod(q.l,q.r));});
  compare<ll,sum,zero>("max-right",values,trace,[](auto& tree,const Query& q){
    return ull(tree.max_right(q.l,[&](ll x){return x<=q.x*16;}));});
  compare<ll,sum,zero>("min-left",values,trace,[](auto& tree,const Query& q){
    return ull(tree.min_left(q.r,[&](ll x){return x<=q.x*16;}));});
  compare<ll,sum,zero>("set-get-mixed",values,trace,[](auto& tree,const Query& q){
    if(q.x%4==0) tree.set(q.p,q.x);
    return code(tree.get(q.p));});
}
}

int main(){
  cout<<fixed<<setprecision(3)<<"seed "<<seed<<" repeats "<<repetitions<<'\n';
  cout<<"sizeof "<<sizeof(SegmentTree<ll,sum,zero>)<<' '<<sizeof(atcoder::segtree<ll,sum,zero>)<<'\n';
  cout<<"N operation repetitions own_ns ACL_ns checksum\n";
  for(int n:{1024,65536,200000,262144,262145,1000000}) run(n);
}
