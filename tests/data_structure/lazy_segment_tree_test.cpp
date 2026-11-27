#include <Library/data_structure/lazy_segment_tree.hpp>
#include <Library/data_structure/dual_segment_tree.hpp>
#include <Library/io/debug.hpp>
#include <Library/math/modint.hpp>
#include <atcoder/lazysegtree>

namespace {
constexpr ull seed=0x4c415a59534547;
mt19937_64 rng(seed);
int n=0,step=0;
void check(bool ok,const char* message){
  if(!ok){ cerr<<message<<" seed="<<seed<<" n="<<n<<" step="<<step<<'\n'; exit(1); }
}

template<class T>
struct Sum {
  T value;
  ll length;
  bool operator==(const Sum&) const=default;
  friend ostream& operator<<(ostream& os,const Sum& x){return os<<x.value;}
};

template<class T> Sum<T> combine(Sum<T> a,Sum<T> b){return {a.value+b.value,a.length+b.length};}
template<class T> Sum<T> empty_sum(){return {0,0};}
template<class T> struct Affine {T a,b;};
template<class T> Affine<T> affine(Affine<T> f,Affine<T> g){return {f.a*g.a,f.a*g.b+f.b};}
template<class T> Affine<T> affine_id(){return {1,0};}
template<class T> Sum<T> map_affine(Affine<T> f,Sum<T> x){
  return {f.a*x.value+f.b*x.length,x.length};
}
template<class T> using OwnAffine=LazySegmentTree<Sum<T>,combine<T>,empty_sum<T>,
  Affine<T>,map_affine<T>,affine<T>,affine_id<T>>;
template<class T> using AclAffine=atcoder::lazy_segtree<Sum<T>,combine<T>,empty_sum<T>,
  Affine<T>,map_affine<T>,affine<T>,affine_id<T>>;
static_assert(is_same_v<decltype(declval<OwnAffine<ll>&>()[0]),Sum<ll>>);
static_assert(is_same_v<decltype(declval<const OwnAffine<ll>&>().debug_view()),vector<Sum<ll>>>);

ll min_op(ll x,ll y){return min(x,y);}
ll max_op(ll x,ll y){return max(x,y);}
ll inf(){return 1'000'000'000;}
ll neg_inf(){return -inf();}
ll add_extreme(ll f,ll x){return x==inf() || x==neg_inf() ? x : f+x;}
ll add(ll f,ll g){return f+g;}
ll zero(){return 0;}
using Assign=optional<ll>;
Assign assign(Assign f,Assign g){return f ? f : g;}
Assign no_assign(){return nullopt;}
ll replace_min(Assign f,ll x){return f.value_or(x);}
Sum<ll> add_sum(ll f,Sum<ll> x){return {x.value+f*x.length,x.length};}
Sum<ll> replace_sum(Assign f,Sum<ll> x){return {f ? *f*x.length : x.value,x.length};}
using AddSum=LazySegmentTree<Sum<ll>,combine<ll>,empty_sum<ll>,ll,add_sum,add,zero>;
using AclSum=atcoder::lazy_segtree<Sum<ll>,combine<ll>,empty_sum<ll>,ll,add_sum,add,zero>;

template<class Tree,class Acl,class S,class F,class Op,class Map,class E>
void random_operations(vector<S> a,vector<F> actions,Op op,Map map,E e){
  Tree tree(a);
  Acl acl(a);
  n=int(a.size());
  auto range=[&](int l,int r){
    S result=e();
    for(int i=l;i<r;++i) result=op(result,a[i]);
    return result;
  };
  for(step=0;step<1500;++step){
    int l=int(rng()%(n+1)),r=int(rng()%(n+1));
    if(l>r) swap(l,r);
    if(step%11==0){l=0;r=n;}
    F f=actions[rng()%actions.size()];
    if(step%5<2){
      tree.apply(l,r,f); acl.apply(l,r,f);
      for(int i=l;i<r;++i) a[i]=map(f,a[i]);
    }else if(n && step%5==2){
      int p=int(rng()%n),q=int(rng()%n);
      tree.set(p,a[q]); acl.set(p,a[q]); a[p]=a[q];
      tree.apply(p,f); acl.apply(p,f); a[p]=map(f,a[p]);
      check(tree.get(p)==a[p] && tree[p]==acl.get(p),"point operations");
    }else{
      S expected=range(l,r);
      check(tree.prod(l,r)==expected && acl.prod(l,r)==expected,"independent prod");
    }
    if(step%7==0) check(as_const(tree).to_vector()==a,"const snapshot");
    check(as_const(tree).all_prod()==range(0,n) && acl.all_prod()==range(0,n),"root aggregate");
  }
}

template<class T>
void affine_tests(){
  vector<Sum<T>> a(65);
  for(int i=0;i<65;++i) a[i]={i,1};
  random_operations<OwnAffine<T>,AclAffine<T>>(a,vector<Affine<T>>{{2,3},{3,1},{0,7},{1,2}},
      combine<T>,map_affine<T>,empty_sum<T>);
  OwnAffine<T> own(a);
  DualSegmentTree<Sum<T>,Affine<T>,map_affine<T>,affine<T>,affine_id<T>> dual(a);
  auto apply=[&](int l,int r,Affine<T> f){
    own.apply(l,r,f);
    dual.apply(l,r,f);
    for(int i=l;i<r;++i) a[i]=map_affine(f,a[i]);
  };
  apply(0,64,{2,1}); apply(8,24,{3,2}); apply(0,64,{2,3});
  apply(14,56,{5,1}); apply(0,64,{3,4}); apply(0,64,{2,5});
  auto same=own;
  check(as_const(own).to_vector()==a && as_const(own).debug_view()==a,"stacked noncommutative tags");
  for(int i=0;i<65;++i) check(dual.get(i)==a[i],"dual agreement");
  own.set(17,{100,1}); same.set(17,{100,1});
  own.apply(2,59,{3,7}); same.apply(2,59,{3,7});
  check(own.to_vector()==same.to_vector(),"snapshot preserves future updates");
}

struct Inversions {
  ll zeros,ones,inversions;
  bool operator==(const Inversions&) const=default;
};
Inversions concat(Inversions a,Inversions b){
  return {a.zeros+b.zeros,a.ones+b.ones,a.inversions+b.inversions+a.ones*b.zeros};
}
Inversions no_bits(){return {0,0,0};}
Inversions flip(bool f,Inversions x){
  if(f) return {x.ones,x.zeros,x.ones*x.zeros-x.inversions};
  return x;
}
bool toggle(bool a,bool b){return a!=b;}
bool no_flip(){return false;}
using Inv=LazySegmentTree<Inversions,concat,no_bits,bool,flip,toggle,no_flip>;
using AclInv=atcoder::lazy_segtree<Inversions,concat,no_bits,bool,flip,toggle,no_flip>;

void inversions(){
  for(n=0;n<=5;++n) for(int mask=0;mask<(1<<n);++mask)
    for(int l=0;l<=n;++l) for(int r=l;r<=n;++r){
      vector<Inversions> leaves(n);
      vl bits(n);
      for(int i=0;i<n;++i){ bits[i]=mask>>i&1; leaves[i]={1-bits[i],bits[i],0}; }
      Inv own(leaves); AclInv acl(leaves);
      own.apply(l,r,true); acl.apply(l,r,true);
      for(int i=l;i<r;++i) bits[i]^=1;
      auto brute=[&](int x,int y){
        ll count=0;
        for(int i=x;i<y;++i) for(int j=i+1;j<y;++j) count+=bits[i]>bits[j];
        return count;
      };
      check(own.all_prod().inversions==brute(0,n) && acl.all_prod()==own.all_prod(),"exhaustive inversions");
      for(int start=0;start<=n;++start){
        auto pred=[](Inversions x){return x.inversions==0;};
        int left=start,right=start;
        while(left>0 && brute(left-1,start)==0) --left;
        while(right<n && brute(start,right+1)==0) ++right;
        check(own.max_right(start,pred)==right && own.min_left(start,pred)==left,"noncommutative search");
      }
    }
}

void search_after_updates(){
  for(int size:{1,2,7,16,17,64,65}){
    n=size;
    vector<Sum<ll>> a(n,{1,1});
    AddSum tree(a);
    AclSum acl(a);
    for(step=0;step<500;++step){
      int l=int(rng()%(n+1)),r=int(rng()%(n+1)),start=int(rng()%(n+1));
      if(l>r) swap(l,r);
      ll increase=ll(rng()%5),limit=ll(rng()%2000);
      tree.apply(l,r,increase); acl.apply(l,r,increase);
      for(int j=l;j<r;++j) a[j].value+=increase;
      if(step%13==0){
        int j=int(rng()%n);
        a[j].value=0; tree.set(j,a[j]); acl.set(j,a[j]);
      }
      auto g=[limit](Sum<ll> x){return x.value<=limit;};
      int right=start,left=start;
      ll sum_right=0,sum_left=0;
      while(right<n && sum_right+a[right].value<=limit) sum_right+=a[right++].value;
      while(left>0 && sum_left+a[left-1].value<=limit) sum_left+=a[--left].value;
      check(tree.max_right(start,g)==right && tree.min_left(start,g)==left &&
            acl.max_right(start,g)==right && acl.min_left(start,g)==left,"search after mixed updates");
    }
  }
}

using Permutation=array<int,3>;
string join_text(const string& a,const string& b){return a+b;}
string empty_text(){return {};}
Permutation identity_perm(){return {0,1,2};}
Permutation compose_perm(Permutation f,Permutation g){return {f[g[0]],f[g[1]],f[g[2]]};}
string substitute(Permutation f,string x){
  for(char& c:x) c=char('a'+f[c-'a']);
  return x;
}

void ordered_actions(){
  using Tree=LazySegmentTree<string,join_text,empty_text,Permutation,substitute,compose_perm,identity_perm>;
  using ACL=atcoder::lazy_segtree<string,join_text,empty_text,Permutation,substitute,compose_perm,identity_perm>;
  const vector<Permutation> actions{{1,0,2},{0,2,1},{2,0,1},{0,1,2}};
  check(compose_perm(actions[0],actions[1])!=compose_perm(actions[1],actions[0]),
        "noncommutative permutations");
  random_operations<Tree,ACL>(vector<string>{"a","b","c","b","c","a","c","b"},
      actions,join_text,substitute,empty_text);
}

bool bound(Sum<ll> x){return x.value<=3;}
[[maybe_unused]] bool reject(Sum<ll>){return false;}
vl observed;
Sum<ll> observed_add(ll f,Sum<ll> x){observed.push_back(f);return add_sum(f,x);}
void state(){
  AddSum empty, none(0), zeros(5), t(vector<Sum<ll>>(7,{1,1}));
  check(empty.empty() && none.all_prod()==empty_sum<ll>() &&
        zeros.to_vector()==vector<Sum<ll>>(5,{0,0}),"empty and size constructors");
  empty.apply(0,0,4);
  check(empty.prod(0,0)==empty_sum<ll>() && empty.max_right<bound>(0)==0 &&
        empty.min_left<bound>(0)==0,"empty APIs");
  t.apply(0,7,2);
  check(t.max_right<bound>(0)==1 && t.min_left<bound>(7)==6,"search after range update");
  int limit=8;
  auto pred=[limit](Sum<ll> x){return x.value<=limit;};
  check(t.max_right(1,pred)==3 && t.min_left(6,pred)==4,"captured search");
  auto copy=t;
  AddSum moved(std::move(t));
  check(t.empty() && t.all_prod()==empty_sum<ll>() && t.to_vector().empty(),"moved-from empty");
  t=copy;
  check(t.to_vector()==moved.to_vector(),"copy");
  t.set(1,{10,1});
  moved=std::move(t);
  check(moved[1].value==10 && t.empty() && copy[1].value==3,"move assignment");
  auto& alias=moved;
  moved=std::move(alias);
  check(moved[1].value==10,"self move");
  using Observed=LazySegmentTree<Sum<ll>,combine<ll>,empty_sum<ll>,ll,observed_add,add,zero>;
  Observed x(vector<Sum<ll>>(32,{1,1}));
  x.apply(0,16,4); x.apply(0,32,3);
  auto y=x;
  (void)as_const(x).to_vector();
  observed.clear();
  auto first=x.get(9);
  auto calls=observed;
  observed.clear();
  check(y.get(9)==first && calls==observed,"snapshot leaves propagation state intact");
}

bool both(bool x,bool y){return x&&y;}
bool truth(){return true;}
bool saturate(bool f,bool x){return f||x;}
bool either(bool f,bool g){return f||g;}
void bool_and_debug(){
  using namespace atcoder;
  using Short=::lazy_segtree<Sum<ll>,combine<ll>,empty_sum<ll>,ll,add_sum,add,zero>;
  using ACL=atcoder::lazy_segtree<Sum<ll>,combine<ll>,empty_sum<ll>,ll,add_sum,add,zero>;
  static_assert(is_same_v<Short,AddSum>);
  static_assert(!is_same_v<Short,ACL>);
  LazySegmentTree<bool,both,truth,bool,saturate,either,no_flip> bool_tree(vector<bool>{false,true,false});
  bool_tree.apply(0,2,true);
  check(bool_tree.to_vector()==vector<bool>{true,true,false} && !bool_tree.all_prod(),"S and F bool");
  const OwnAffine<modint> a(vector<Sum<modint>>{{-1,1},{2,1}});
  const OwnAffine<ll> empty;
  const vector<Sum<ll>> values{{1,1},{2,1}};
  Short change(values);
  ACL acl(values);
  change.apply(0,2,3);
  acl.apply(0,2,3);
  const Short& b=change;
  check(b.all_prod()==Sum<ll>{9,2} && b.all_prod()==acl.all_prod(),
        "qualified own and ACL lazy_segtree aliases");
  ostringstream out;
  auto* old=cerr.rdbuf(out.rdbuf());
  int hits=0;
  debug(b); debug(empty);
  dbg(vector{b},pair{empty,tuple{a}},++hits);
  cerr.rdbuf(old);
#ifdef LOCAL
  check(hits==1 && out.str()=="4 5 \n\n[vector{b},pair{empty,tuple{a}},++hits] = [[4, 5]], ([], ([1000000006, 2])), 1\n","debug display");
#else
  check(hits==0 && out.str().empty(),"non-LOCAL debug");
#endif
}

bool invalid(string_view mode){
  if(mode=="negative") (void)AddSum(-1);
  else if(mode=="overflow") (void)AddSum(numeric_limits<ll>::max());
  else {
#ifndef NDEBUG
    AddSum t(2);
    if(mode=="index") (void)t.get(2);
    else if(mode=="range") t.apply(0,3,1);
    else if(mode=="start") (void)t.max_right(-1,bound);
    else if(mode=="predicate") (void)t.min_left<reject>(2);
    else return false;
#else
    return false;
#endif
  }
  return true;
}
}

int main(int argc,char** argv){
  if(argc==2) return invalid(argv[1]) ? 0 : 2;
  modint::set_mod(1'000'000'007);
  state(); bool_and_debug(); inversions(); ordered_actions(); search_after_updates();
  affine_tests<modint998244353>(); affine_tests<modint>();
  vector<Sum<ll>> a(73,{1,1});
  random_operations<AddSum,AclSum>(a,vl{0,2,-1,7},combine<ll>,add_sum,empty_sum<ll>);
  using Replace=LazySegmentTree<Sum<ll>,combine<ll>,empty_sum<ll>,Assign,replace_sum,assign,no_assign>;
  using AclReplace=atcoder::lazy_segtree<Sum<ll>,combine<ll>,empty_sum<ll>,Assign,replace_sum,assign,no_assign>;
  random_operations<Replace,AclReplace>(a,vector<Assign>{nullopt,0,2,-1},combine<ll>,replace_sum,empty_sum<ll>);
  using Min=LazySegmentTree<ll,min_op,inf,ll,add_extreme,add,zero>;
  using Max=LazySegmentTree<ll,max_op,neg_inf,ll,add_extreme,add,zero>;
  using AclMin=atcoder::lazy_segtree<ll,min_op,inf,ll,add_extreme,add,zero>;
  using AclMax=atcoder::lazy_segtree<ll,max_op,neg_inf,ll,add_extreme,add,zero>;
  using ReplaceMin=LazySegmentTree<ll,min_op,inf,Assign,replace_min,assign,no_assign>;
  using AclReplaceMin=atcoder::lazy_segtree<ll,min_op,inf,Assign,replace_min,assign,no_assign>;
  random_operations<Min,AclMin>(vl(65,2),vl{0,3,-1},min_op,add_extreme,inf);
  random_operations<Max,AclMax>(vl(65,2),vl{0,3,-1},max_op,add_extreme,neg_inf);
  random_operations<ReplaceMin,AclReplaceMin>(vl(65,2),vector<Assign>{nullopt,0,5,-2},min_op,replace_min,inf);
  cout<<"lazy_segment_tree tests passed\n";
}
