#include <Library/data_structure/indexed_set.hpp>
#include <Library/data_structure/fenwick_tree.hpp>
#include <Library/data_structure/potential_union_find.hpp>
#include <Library/io/debug.hpp>
#include <Library/data_structure/dual_segment_tree_preset.hpp>
#include <Library/data_structure/interval_set.hpp>
#include <atcoder/modint>

namespace debug_test {

struct Counted {
  int value;
  static inline int copies=0;

  explicit Counted(int x):value(x){}
  Counted(const Counted& other):value(other.value){ ++copies; }

  friend ostream& operator<<(ostream& out,const Counted& x){
    return out << x.value;
  }
};

} // namespace debug_test

#ifdef LOCAL

namespace unrelated {
struct Value {
  friend ostream& operator<<(ostream& out,const Value&){ return out << "value"; }
};

inline void write(const Value&){ cerr << "unrelated"; }

template<bool>
void write(const vector<Value>&){ cerr << "unrelated row"; }
} // namespace unrelated

namespace debug_test {

class Capture {
  ostringstream output;
  streambuf* original=cerr.rdbuf(output.rdbuf());

public:
  ~Capture(){ cerr.rdbuf(original); }
  string result() const { return output.str(); }
};

template<class F>
void expect(const char* label, string_view expected, F emit){
  string actual;
  {
    Capture capture;
    emit();
    actual=capture.result();
  }
  if(actual!=expected){
    cerr << label << "\nexpected: " << quoted(string(expected))
         << "\nactual:   " << quoted(actual) << '\n';
    exit(1);
  }
}

void check(bool ok,const char* label){
  if(!ok){
    cerr << label << '\n';
    exit(1);
  }
}

template<class T>
void expect_object(const char* label,const T& object,string_view plain,string_view compact){
  const string expected=string(plain)+"\n[object] = "+string(compact)+"\n";
  expect(label,expected,[&]{
    debug(object);
    dbg(object);
  });
}

struct MoveOnly {
  int value;
  explicit MoveOnly(int x):value(x){}
  MoveOnly(const MoveOnly&)=delete;
  MoveOnly(MoveOnly&&)=default;

  friend ostream& operator<<(ostream& out,const MoveOnly& x){
    return out << "item:" << x.value;
  }
};

struct StreamMod {
  int val() const { return 2; }
  static int mod(){ return 7; }
  friend ostream& operator<<(ostream& out,const StreamMod&){
    return out << "stream";
  }
};

struct ViewValue {
  auto debug_view() const { return tuple{3,optional{5}}; }
};

struct ViewRef {
  vector<MoveOnly> values;
  const auto& debug_view() const { return values; }
};

struct ViewRange {
  array<int,3> values{1,2,3};
  auto debug_view() const {
    return values | views::filter([](int x){ return x%2!=0; })
                  | views::transform([](int x){ return x*10; });
  }
};

static_assert(ranges::range<decltype(declval<const ViewRange&>().debug_view())>);
static_assert(!ranges::range<const decltype(declval<const ViewRange&>().debug_view())>);

struct ViewPriority {
  array<int,2> values{1,2};
  auto begin() const { return values.begin(); }
  auto end() const { return values.end(); }
  auto debug_view() const { return vector<string>{"view","first"}; }
};

struct Hash {
  size_t operator()(int x) const { return std::hash<int>{}(x); }
};

void legacy(){
  expect("string formats","one\nfirst\nsecond\nthree\n",[]{
    debug(string("one"));
    debug(vector<string>{"first","second"});
    debug("three");
  });
  expect("vector formats","1 2 \n65 66 \n1 2 \n\n3 \nA\nB\n",[]{
    debug(vl{1,2});
    debug(vb{65,66});
    debug(vvvl{{{1,2},{}},{{3}}});
    debug(vvstr{{"A"},{"B"}});
  });
  expect("empty containers","\n\n\n\n\n\n\n\n\n\n",[]{
    debug(vl{});
    debug(vb{});
    debug(vstr{});
    debug(vvl{});
    debug(vvl{{}});
    debug(deque<int>{});
    debug(set<int>{});
    debug(multiset<int>{});
    debug(unordered_set<int>{});
    debug(unordered_multiset<int>{});
    debug(map<int,int>{});
    debug(unordered_map<int,int>{});
  });
  expect("containers and pair",
      "3 1 \n1 2 \n1 1 \n5 \n6 6 \n(1,2) (3,4) \n(7,8) \n(2, 3) \n(4, 5)\n",[]{
    debug(deque<int>{3,1});
    debug(set<int>{2,1});
    debug(multiset<int>{1,1});
    debug(unordered_set<int>{5});
    debug(unordered_multiset<int>{6,6});
    debug(map<int,int>{{3,4},{1,2}});
    debug(unordered_map<int,int>{{7,8}});
    debug(vector<pair<int,int>>{{2,3}});
    debug(pair{4,5});
  });
  expect("dbg prefix and separators","[] = \n[x, p, text] = 12, (3, 4), abc\n8, 9\n",[]{
    int x=12;
    pair p{3,4};
    string text="abc";
    dbg();
    dbg(x, p, text);
    debug_out(8,9);
  });
}

void recursive(){
  const auto complex=pair{vector{tuple{1,optional{pair{2,3}}}},optional<int>{}};
  expect("nested","[complex] = ([(1, (2, 3))], nullopt)\n", [&]{ dbg(complex); });
  expect("compact","[a, b, t, x] = [1, 2, 3], [[1, 2], [3, 4]], (1, 2, 3), nullopt\n",[]{
    vector a{1,2,3};
    vector b{vector{1,2},vector{3,4}};
    tuple t{1,2,3};
    optional<int> x;
    dbg(a, b, t, x);
  });
  expect("array and string formats","[1, 2]\n[3, 4]\na\nbc\nxy\n()\n[]\n",[]{
    const int a[]={3,4};
    const char text[]={'x','y'};
    debug(array{1,2});
    debug(a);
    debug(string_view("a"));
    debug("bc");
    debug(text);
    debug(tuple{});
    debug(array<int,0>{});
  });
  expect("custom comparison and hashing","3 2 1 \n(3,[1, 2]) \n(1,4) \n[8, 7]\n",[]{
    debug(set<int,greater<int>>{1,3,2});
    debug(map<int,vector<int>,greater<int>>{{3,{1,2}}});
    debug(unordered_map<int,int,Hash>{{1,4}});
    debug(list<int>{8,7});
  });
  expect("numeric limits",
      "-170141183460469231731687303715884105728\n"
      "170141183460469231731687303715884105727\n"
      "340282366920938463463374607431768211455\n0\n",[]{
    debug(numeric_limits<i128>::min());
    debug(numeric_limits<i128>::max());
    debug(numeric_limits<__uint128_t>::max());
    debug(__uint128_t(0));
  });
  expect("modints","998244352 \n3\nstream\n",[]{
    using Mint=atcoder::modint998244353;
    using Dynamic=atcoder::dynamic_modint<7>;
    Dynamic::set_mod(11);
    debug(vector{Mint(-1)});
    debug(Dynamic(14));
    debug(StreamMod{});
  });
}

void views_and_structures(){
  expect("debug_view results","(3, 5)\nitem:9 \n[10, 30]\nview\nfirst\n"
      "[nested] = ([(3, 5)], [10, 30])\n",[]{
    ViewRef ref;
    ref.values.emplace_back(9);
    const ViewRange range;
    debug(ViewValue{});
    debug(as_const(ref));
    debug(range);
    debug(ViewPriority{});
    auto nested=pair{vector<ViewValue>(1),range};
    dbg(nested);
  });
  expect("noncopyable","item:7\n[x] = item:7\n",[]{
    const MoveOnly x(7);
    debug(x);
    dbg(x);
  });
  expect("existing iterators",
      "[3, 2, 1]\n[(1, 2), (1, 2), (2, 1)]\n"
      "[(-9223372036854775808, 9223372036854775807)]\n[]\n",[]{
    const IndexedSet<int,greater<int>> st(vector{1,3,2});
    const IndexedMultiset<pair<int,int>> ms(vector{pair{1,2},pair{2,1},pair{1,2}});
    IntervalSet intervals;
    intervals.add(numeric_limits<ll>::min(),numeric_limits<ll>::max());
    debug(st);
    debug(ms);
    debug(as_const(intervals));
    debug(IndexedSet<int>{});
  });
}


void adl_regression(){
  expect("unrelated write","value \nvalue \nvalue \n(value, 2)\n[value]\n",[]{
    const vector<unrelated::Value> items(1);
    debug(items);
    debug(vector{items,items});
    debug(pair{unrelated::Value{},2});
    debug(array{unrelated::Value{}});
  });
}

void ordered_ranges(){
  IndexedSet<int,greater<int>> st(vector{1,3,2});
  expect_object("indexed set initial",st,"[3, 2, 1]","[3, 2, 1]");
  check(st.size()==3 && st.min()==3 && st.max()==1,"indexed set initial state");
  st.erase(2);
  st.insert(4);
  expect_object("indexed set updated",st,"[4, 3, 1]","[4, 3, 1]");
  check(vector(st.begin(),st.end())==vector{4,3,1},"indexed set updated state");

  const IndexedMultiset<bool> bits(vector{true,false,true});
  expect_object("indexed multiset bool",bits,"[0, 1, 1]","[0, 1, 1]");
  check(bits.size()==3 && bits.count(true)==2,"indexed multiset bool state");
  const IndexedMultiset<tuple<int,int,int>> tuples(vector{tuple{2,1,0},tuple{1,2,3},tuple{1,2,3}});
  expect_object("indexed multiset tuples",tuples,
      "[(1, 2, 3), (1, 2, 3), (2, 1, 0)]","[(1, 2, 3), (1, 2, 3), (2, 1, 0)]");
  check(tuples.size()==3 && tuples.count({1,2,3})==2,"indexed multiset tuple state");

  IntervalSet intervals;
  expect_object("empty interval set",intervals,"[]","[]");
  check(intervals.empty(),"empty interval set state");
  constexpr ll lo=numeric_limits<ll>::min(),hi=numeric_limits<ll>::max();
  intervals.add(lo,hi);
  const auto snapshot=vector(intervals.begin(),intervals.end());
  expect_object("wide interval set",intervals,
      "[(-9223372036854775808, 9223372036854775807)]",
      "[(-9223372036854775808, 9223372036854775807)]");
  check(snapshot==vector(intervals.begin(),intervals.end()) && intervals.contains(lo,hi),
        "wide interval set state");
}

void fenwick_output(){
  const FenwickTree<> empty(0);
  expect_object("empty fenwick",empty,"[]","[]");
  check(empty.size()==0 && empty.all_sum()==0,"empty fenwick state");

  FenwickTree<> fw(vl{4,-2,1});
  expect_object("initial fenwick",fw,"[4, -2, 1]","[4, -2, 1]");
  check(fw.get(0)==4 && fw.get(1)==-2 && fw.get(2)==1,"initial fenwick state");
  fw.add(0,-5);
  fw.set(2,7);
  const pair nested{cref(fw).get(),4};
  expect_object("updated fenwick",fw,"[-1, -2, 7]","[-1, -2, 7]");
  expect("nested fenwick","([-1, -2, 7], 4)\n",[&]{ debug(nested); });
  check(fw.get(0)==-1 && fw.get(1)==-2 && fw.get(2)==7 && fw.all_sum()==4,"updated fenwick state");

  constexpr i128 wide=i128(1)<<80;
  const FenwickTree<i128> large(vc<i128>{wide,-wide});
  expect_object("wide fenwick",large,
      "[1208925819614629174706176, -1208925819614629174706176]",
      "[1208925819614629174706176, -1208925819614629174706176]");
  check(large.get(0)==wide && large.get(1)==-wide,"wide fenwick state");

  using Mint=atcoder::modint998244353;
  const FenwickTree<Mint> modular(vc<Mint>{Mint(-1),Mint(2)});
  expect_object("modular fenwick",modular,"[998244352, 2]","[998244352, 2]");
  check(modular.get(0)==Mint(-1) && modular.all_sum()==Mint(1),"modular fenwick state");
}

using Affine=pair<ll,ll>;
ll affine_map(Affine f,ll x){ return f.first*x+f.second; }
Affine affine_compose(Affine f,Affine g){
  return {f.first*g.first,f.first*g.second+f.second};
}
Affine affine_identity(){ return {1,0}; }

template<class Seg>
void check_dual(const char* label,const Seg& seg,const vl& expected,string_view rendered){
  auto check_points=[&]{
    check(seg.size()==ssize(expected),label);
    rep(p,seg.size()) check(seg.get(p)==expected[p],label);
  };
  check_points();
  expect_object(label,seg,rendered,rendered);
  check_points();
}

void dual_output(){
  using Seg=DualSegmentTree<ll,Affine,affine_map,affine_compose,affine_identity>;
  check_dual("empty dual",Seg{},{},"[]");
  Seg seg(vl{1,2,3,4});
  check_dual("initial dual",seg,{1,2,3,4},"[1, 2, 3, 4]");
  seg.apply(0,2,{2,1});
  seg.apply(0,4,{3,4});
  check_dual("noncommutative dual",seg,{13,19,13,16},"[13, 19, 13, 16]");
  seg.apply(1,{1,5});
  check_dual("noncommutative dual after point update",seg,{13,24,13,16},"[13, 24, 13, 16]");

  dual_segtree_add<> add(vl{1,2,3});
  DualSegmentTreeUpdate<> assign(vl{1,2,3});
  dual_segtree_chmin<> lower(vl{1,2,3});
  DualSegmentTreeChmax<> upper(vl{1,2,3});
  add.apply(0,2,4);
  assign.apply(1,3,4);
  lower.apply(1,3,0);
  upper.apply(0,2,4);
  check_dual("add preset",add,{5,6,3},"[5, 6, 3]");
  check_dual("assign preset",assign,{1,4,4},"[1, 4, 4]");
  check_dual("chmin preset",lower,{1,0,0},"[1, 0, 0]");
  check_dual("chmax preset",upper,{4,4,3},"[4, 4, 3]");
}

void potential_output(){
  const PotentialUnionFind<> empty(0);
  expect_object("empty potential DSU",empty,"","[]");
  check(empty.comp_cnt()==0 && empty.consistent(),"empty potential DSU state");

  PotentialUnionFind<> uf(8);
  expect_object("initial potential DSU",as_const(uf),
      "(0, 0, [(0, 0)]) (1, 0, [(1, 0)]) (2, 0, [(2, 0)]) (3, 0, [(3, 0)]) "
      "(4, 0, [(4, 0)]) (5, 0, [(5, 0)]) (6, 0, [(6, 0)]) (7, 0, [(7, 0)]) ",
      "[(0, 0, [(0, 0)]), (1, 0, [(1, 0)]), (2, 0, [(2, 0)]), (3, 0, [(3, 0)]), "
      "(4, 0, [(4, 0)]), (5, 0, [(5, 0)]), (6, 0, [(6, 0)]), (7, 0, [(7, 0)])]");
  check(uf.comp_cnt()==8 && uf.consistent(),"initial potential DSU state");
  uf.merge(3,4,7);
  uf.merge(2,4,9);
  uf.merge(0,1,4);
  uf.merge(1,2,10);
  uf.merge(5,6,1);
  uf.merge(5,6,2);
  auto check_state=[](PotentialUnionFind<> probe){
    check(probe.comp_cnt()==3 && probe.bad_comp_cnt()==1 && probe.leader(1)==3 &&
          probe.diff(3,0)==-16 && probe.diff(3,1)==-12 && probe.diff(3,2)==-2 &&
          probe.diff(3,4)==7 && !probe.consistent(6) && !probe.try_diff(5,6) &&
          probe.groups()==vvl{{0,1,2,3,4},{5,6},{7}},"updated potential DSU state");
  };
  check_state(uf);
  expect_object("updated potential DSU",uf,
      "(3, 0, [(0, -16), (1, -12), (2, -2), (3, 0), (4, 7)]) "
      "(5, 1, [(5, nullopt), (6, nullopt)]) (7, 0, [(7, 0)]) ",
      "[(3, 0, [(0, -16), (1, -12), (2, -2), (3, 0), (4, 7)]), "
      "(5, 1, [(5, nullopt), (6, nullopt)]), (7, 0, [(7, 0)])]");
  check_state(uf);

  using Mint=atcoder::modint998244353;
  PotentialUnionFind<Mint> modular(2);
  modular.merge(1,0,Mint(-1));
  expect_object("modular potential DSU",modular,
      "(1, 0, [(0, 998244352), (1, 0)]) ","[(1, 0, [(0, 998244352), (1, 0)])]");
  check(modular.diff(1,0)==Mint(-1),"modular potential DSU state");
}

template<class Adapter>
void expect_adapter(const char* label,const Adapter& object,string_view plain,string_view compact){
  auto snapshot=[&]{
    Adapter copy(object);
    vector<typename Adapter::value_type> values;
    while(!copy.empty()){
      if constexpr(requires{ copy.front(); }) values.eb(copy.front());
      else values.eb(copy.top());
      copy.pop();
    }
    return values;
  };
  const auto before=snapshot();
  expect_object(label,object,plain,compact);
  check(snapshot()==before,label);
}

struct NearestFirst {
  int target;

  bool operator()(int x,int y) const {
    return pair{abs(x-target),x}>pair{abs(y-target),y};
  }
};

void adapter_output(){
  deque<int> deq;
  expect_object("empty deque",deq,"","[]");
  check(deq.empty(),"empty deque state");
  deq.pb(2);
  expect_object("one element deque",deq,"2 ","[2]");
  check(deq==deque{2},"one element deque state");
  deq.pf(3);
  deq.pb(3);
  expect_object("multi-element deque",deq,"3 2 3 ","[3, 2, 3]");
  check(deq==deque{3,2,3},"multi-element deque state");
  deq.pop_front();
  expect_object("updated deque",deq,"2 3 ","[2, 3]");
  check(deq==deque{2,3},"updated deque state");

  stack<int,vector<int>> st;
  expect_adapter("empty stack",st,"","[]");
  st.push(1);
  expect_adapter("one element stack",st,"1 ","[1]");
  st.push(2);
  st.push(2);
  expect_adapter("multi-element stack",st,"2 2 1 ","[2, 2, 1]");
  st.pop();
  st.push(3);
  expect_adapter("updated stack",st,"3 2 1 ","[3, 2, 1]");

  queue<int,list<int>> fifo;
  expect_adapter("empty queue",fifo,"","[]");
  fifo.push(1);
  expect_adapter("one element queue",fifo,"1 ","[1]");
  fifo.push(1);
  fifo.push(2);
  expect_adapter("multi-element queue",fifo,"1 1 2 ","[1, 1, 2]");
  fifo.pop();
  fifo.push(3);
  expect_adapter("updated queue",fifo,"1 2 3 ","[1, 2, 3]");

  priority_queue<int> heap;
  expect_adapter("empty priority queue",heap,"","[]");
  heap.push(3);
  expect_adapter("one element priority queue",heap,"3 ","[3]");
  heap.push(1);
  heap.push(3);
  expect_adapter("multi-element priority queue",heap,"3 3 1 ","[3, 3, 1]");
  heap.pop();
  heap.push(2);
  expect_adapter("updated priority queue",heap,"3 2 1 ","[3, 2, 1]");

  const priority_queue_g<int> min_heap(greater<int>{},vector{4,2,2});
  expect_adapter("priority_queue_g",min_heap,"2 2 4 ","[2, 2, 4]");
  const priority_queue<int,deque<int>,NearestFirst> custom(NearestFirst{5},deque{1,2,6,9,6});
  expect_adapter("stateful comparator and deque storage",custom,"6 6 2 1 9 ","[6, 6, 2, 1, 9]");

  stack<Counted> counted;
  counted.emplace(1);
  counted.emplace(2);
  Counted::copies=0;
  expect_object("one adapter copy per display",counted,"2 1 ","[2, 1]");
  check(Counted::copies==4 && counted.size()==2 && counted.top().value==2,
        "stack copy count or state");
}

void nested_adapters(){
  using Item=tuple<pair<int,int>,optional<int>>;
  const queue<Item> tuples(deque<Item>{{{1,2},nullopt},{{3,4},5}});
  expect_adapter("tuple, pair and optional elements",tuples,
      "((1, 2), nullopt) ((3, 4), 5) ","[((1, 2), nullopt), ((3, 4), 5)]");
  const queue<stack<int>> nested(deque{stack<int>(deque{1,2}),stack<int>{}});
  expect_adapter("nested adapters",nested,"[2, 1] [] ","[[2, 1], []]");
  const auto combined=pair{vector{nested},IndexedSet<int>(vector{3,1,2})};
  expect_object("adapters nested with IndexedSet",combined,
      "([[[2, 1], []]], [1, 2, 3])","([[[2, 1], []]], [1, 2, 3])");
  check(combined.first.front().size()==2 && combined.first.front().front().top()==2 &&
        combined.second.size()==3,"nested object state");

  constexpr i128 wide=i128(1)<<80;
  const queue<i128> large(deque{wide,-wide});
  expect_adapter("i128 elements",large,
      "1208925819614629174706176 -1208925819614629174706176 ",
      "[1208925819614629174706176, -1208925819614629174706176]");
  using Mint=atcoder::modint998244353;
  const stack<Mint> modular(deque{Mint(2),Mint(-1)});
  expect_adapter("modint elements",modular,"998244352 2 ","[998244352, 2]");
}

} // namespace debug_test

int main(){
  debug_test::legacy();
  debug_test::recursive();
  debug_test::views_and_structures();
  debug_test::adl_regression();
  debug_test::ordered_ranges();
  debug_test::fenwick_output();
  debug_test::dual_output();
  debug_test::potential_output();
  debug_test::adapter_output();
  debug_test::nested_adapters();
  cout << "debug (LOCAL): all checks passed\n";
}

#else

int main(){
  ostringstream captured;
  auto* original=cerr.rdbuf(captured.rdbuf());
  int calls=0;
  struct View {
    int& calls;
    int debug_view() const { return ++calls; }
  };
  [[maybe_unused]] View view{calls};
  debug(++calls);
  dbg(++calls, view, local_only_variable);
  dbg(FenwickTree<>(++calls).debug_view());
  stack<debug_test::Counted> counted;
  counted.emplace(7);
  debug_test::Counted::copies=0;
  debug(counted);
  dbg(counted, (++calls, counted));
  debug(stack<debug_test::Counted>(counted));
  debug(view);
  debug(local_only_variable);
  dbg();
  cerr.rdbuf(original);
  if(calls!=0 || debug_test::Counted::copies!=0 || !captured.str().empty()){
    cerr << "non-LOCAL debug evaluated an argument or wrote to stderr\n";
    return 1;
  }
  cout << "debug (non-LOCAL): all checks passed\n";
}

#endif
