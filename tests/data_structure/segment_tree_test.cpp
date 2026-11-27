#include <Library/data_structure/segment_tree.hpp>
#include <Library/io/debug.hpp>
#include <Library/math/modint.hpp>
#include <atcoder/segtree>

namespace {
constexpr ull seed=0x53454754524545;
mt19937_64 rng(seed);
int step=0,n=0;
void check(bool ok,const char* message){
  if(!ok){ cerr<<message<<" seed="<<seed<<" n="<<n<<" step="<<step<<'\n'; exit(1); }
}
ll sum(ll a,ll b){return a+b;}
ll zero(){return 0;}
ll minimum(ll a,ll b){return min(a,b);}
ll maximum(ll a,ll b){return max(a,b);}
ll top(){return numeric_limits<ll>::max();}
ll bottom(){return numeric_limits<ll>::min();}
ll common(ll a,ll b){return gcd(a,b);}
ll xor_op(ll a,ll b){return a^b;}
bool both(bool a,bool b){return a&&b;}
bool yes_value(){return true;}
bool identity_bool(bool x){return x;}
bool at_most_five(ll x){return x<=5;}
[[maybe_unused]] bool false_pred(ll){return false;}
using Tree=SegmentTree<ll,sum,zero>;
static_assert(is_same_v<decltype(declval<const Tree&>()[0]),ll>);
static_assert(is_same_v<decltype(declval<const Tree&>().debug_view()),span<const ll>>);

template<class S,auto op,auto e>
void aggregates(vector<S> a){
  SegmentTree<S,op,e> own(a);
  atcoder::segtree<S,op,e> acl(a);
  n=int(a.size());
  for(step=0;step<500;++step){
    if(n!=0 && step%3==0){
      int p=int(rng()%n),q=int(rng()%n);
      own.set(p,S(a[q])); acl.set(p,S(a[q])); a[p]=a[q];
      check(own.get(p)==S(a[p]) && own[p]==acl.get(p),"get/set");
    }
    int l=int(rng()%(n+1)),r=int(rng()%(n+1));
    if(l>r) swap(l,r);
    S expected=e(),full=e();
    for(int i=l;i<r;++i) expected=op(expected,S(a[i]));
    for(auto x:a) full=op(full,S(x));
    check(own.prod(l,r)==expected && acl.prod(l,r)==expected &&
          own.all_prod()==full && own.to_vector()==a,"aggregate");
  }
}

void search(){
  for(n=0;n<=6;++n) for(int mask=0;mask<(1<<n);++mask){
    vl a(n);
    for(int i=0;i<n;++i) a[i]=mask>>i&1;
    const Tree t(a);
    atcoder::segtree<ll,sum,zero> acl(a);
    for(int l=0;l<=n;++l) for(int limit=0;limit<=n;++limit){
      auto pred=[limit](ll x){return x<=limit;};
      int right=l,left=l;
      ll x=0;
      while(right<n && x+a[right]<=limit) x+=a[right++];
      x=0;
      while(left>0 && x+a[left-1]<=limit) x+=a[--left];
      check(t.max_right(l,pred)==right && t.min_left(l,pred)==left &&
            acl.max_right(l,pred)==right && acl.min_left(l,pred)==left,"exhaustive search");
      check(t.prod(l,l)==0,"empty interval");
    }
  }
  Tree t(vl{3,2,1});
  check(t.max_right<at_most_five>(0)==2 && t.min_left<at_most_five>(3)==1,"NTTP predicate");
}

string concat(const string& a,const string& b){return a+b;}
string no_chars(){return {};}
using Text=SegmentTree<string,concat,no_chars>;
struct Affine {
  ll a,b;
  bool operator==(const Affine&) const=default;
};
Affine compose(Affine a,Affine b){return {b.a*a.a%97,(b.a*a.b+b.b)%97};}
Affine identity(){return {1,0};}

void noncommutative(){
  aggregates<Affine,compose,identity>({{2,1},{3,4},{5,6},{2,7},{3,8}});
  const Text t(vector<string>{"a","b","c","d","e","f","g"});
  check(t.prod(1,6)=="bcdef","ordered concatenation");
  check(t.max_right(1,[](const string& x){return string_view("bcdx").starts_with(x);})==4,
        "noncommutative right search");
  check(t.min_left(6,[](const string& x){return string_view("xdef").ends_with(x);})==3,
        "noncommutative left search");
}

template<class T> T mint_sum(T a,T b){return a+b;}
template<class T> T mint_zero(){return 0;}

void state(){
  Tree empty,zero_size(0), initialized(5), original(vl{2,4,8});
  check(empty.empty() && zero_size.all_prod()==0 && initialized.to_vector()==vl(5),"constructors");
  auto saved=original;
  Tree moved(std::move(original));
  check(original.empty() && original.all_prod()==0 && original.to_vector().empty() &&
        original.debug_view().empty() && original.prod(0,0)==0,"move construction state");
  original=moved;
  check(original.to_vector()==vl{2,4,8},"copy assignment");
  original.set(0,5);
  check(moved[0]==2 && saved[0]==2,"copy independence");
  moved=std::move(original);
  check(moved[0]==5 && original.empty(),"move assignment");
  auto& alias=moved;
  moved=std::move(alias);
  check(moved[0]==5 && moved.size()==3,"self move");
  SegmentTree<bool,both,yes_value> bits(vector<bool>{true,false,true});
  check(bits.max_right<identity_bool>(0)==1 && bits.min_left<identity_bool>(3)==2 &&
        bits.debug_view()==vector<bool>{true,false,true},"bool search and view");
}

void debugging(){
  using namespace atcoder;
  using Short=::segtree<ll,sum,zero>;
  using ACL=atcoder::segtree<ll,sum,zero>;
  static_assert(is_same_v<Short,Tree>);
  static_assert(!is_same_v<Short,ACL>);
  using Mint=modint998244353;
  const SegmentTree<Mint,mint_sum<Mint>,mint_zero<Mint>> modular(vector<Mint>{-1,2});
  Short tree(vl{1,3});
  ACL acl(vl{1,3});
  tree.set(1,2);
  acl.set(1,2);
  const Short& a=tree;
  const Short b;
  check(a.get(1)==acl.get(1) && a.prod(0,2)==3 && a.prod(0,2)==acl.prod(0,2),
        "qualified own and ACL segtree aliases");
  const Text text(vector<string>{"x","y"});
  check(ranges::equal(a.debug_view(),vl{1,2}),"leaf view");
  ostringstream out;
  auto* old=cerr.rdbuf(out.rdbuf());
  int hits=0;
  debug(a);
  debug(b);
  dbg(vector{a},pair{b,tuple{a}},modular,text,++hits);
  cerr.rdbuf(old);
#ifdef LOCAL
  check(hits==1 && out.str()=="1 2 \n\n[vector{a},pair{b,tuple{a}},modular,text,++hits] = [[1, 2]], ([], ([1, 2])), [998244352, 2], [x, y], 1\n","debug display");
#else
  check(hits==0 && out.str().empty(),"non-LOCAL debug");
#endif
}

bool invalid(string_view mode){
  if(mode=="negative") (void)Tree(-1);
  else if(mode=="overflow") (void)Tree(numeric_limits<ll>::max());
  else {
#ifndef NDEBUG
    Tree t(2);
    if(mode=="index") t.set(2,1);
    else if(mode=="range") (void)t.prod(2,1);
    else if(mode=="start") (void)t.min_left(3,at_most_five);
    else if(mode=="predicate") (void)t.max_right<false_pred>(0);
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
  state(); search(); noncommutative(); debugging();
  vl a(67);
  for(auto& x:a) x=ll(rng()%100);
  aggregates<ll,sum,zero>(a);
  aggregates<ll,minimum,top>(a);
  aggregates<ll,maximum,bottom>(a);
  aggregates<ll,common,zero>(a);
  aggregates<ll,xor_op,zero>(a);
  aggregates<bool,both,yes_value>({true,false,true,true,false});
  aggregates<modint998244353,mint_sum<modint998244353>,mint_zero<modint998244353>>({1,2,3,4,5});
  cout<<"segment_tree tests passed\n";
}
