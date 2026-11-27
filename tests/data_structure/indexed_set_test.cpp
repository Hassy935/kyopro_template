#include <Library/data_structure/indexed_set.hpp>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <optional>
#include <random>
#include <set>
#include <tuple>
#include <type_traits>
#include <vector>

namespace indexed_set_test {
using I = long long;
constexpr unsigned seed = 0x17A7BEEF;
std::mt19937 rng(seed);
const char* phase = "initial";
I step = -1;
void check(bool ok, const char* msg) {
  if (!ok) {
    std::cerr << "FAIL: " << phase << " step=" << step
              << " seed=" << seed << ": " << msg << '\n';
    std::exit(1);
  }
}
#define IDX_CHECK(...) check((__VA_ARGS__), #__VA_ARGS__)

// std::set / std::multiset の反復順序を正解とする。
template<class C, class R>
void verify(const C& st, const R& ref, const typename R::value_type& x, bool full) {
  using T = typename R::value_type;
  IDX_CHECK(st.size() == std::ssize(ref));
  IDX_CHECK(st.empty() == ref.empty());
  auto lo = ref.lower_bound(x), hi = ref.upper_bound(x);
  I l = std::distance(ref.begin(), lo), r = std::distance(ref.begin(), hi);
  IDX_CHECK(st.contains(x) == (l != r));
  IDX_CHECK(st.count(x) == r-l);
  IDX_CHECK(st.lower_bound(x) == l);
  IDX_CHECK(st.upper_bound(x) == r);
  IDX_CHECK(st.equal_range(x) == std::pair<I,I>{l,r});
  auto next = [&](auto it) -> std::optional<T> {
    if (it == ref.end()) return std::nullopt;
    return *it;
  };
  auto prev = [&](auto it) -> std::optional<T> {
    if (it == ref.begin()) return std::nullopt;
    return *std::prev(it);
  };
  IDX_CHECK(st.lt(x) == prev(lo));
  IDX_CHECK(st.le(x) == prev(hi));
  IDX_CHECK(st.gt(x) == next(hi));
  IDX_CHECK(st.ge(x) == next(lo));
  if (l != r) IDX_CHECK(st(x) == l);
  if (!ref.empty()) {
    IDX_CHECK(st.min() == *ref.begin());
    IDX_CHECK(st.max() == *ref.rbegin());
    I k = rng() % ref.size();
    IDX_CHECK(st[k] == *std::next(ref.begin(), k));
    auto it = st.end();
    IDX_CHECK(*(--it) == *ref.rbegin());
  }
  if (full) {
    IDX_CHECK(std::equal(st.begin(),st.end(),ref.begin(),ref.end()));
    IDX_CHECK(std::equal(st.rbegin(),st.rend(),ref.rbegin(),ref.rend()));
    I k = 0;
    for (auto it = st.begin(); it != st.end();) {
      auto old = it++;
      IDX_CHECK(*old == st[k]);
      ++k;
    }
    IDX_CHECK(k == st.size());
    auto it = st.end();
    for (auto rt = ref.rbegin(); rt != ref.rend(); ++rt) {
      auto old = it--;
      IDX_CHECK(*it == *rt);
      auto forward = it;
      IDX_CHECK(++forward == old);
    }
    IDX_CHECK(it == st.begin());
  }
}

template<bool Multi, class Compare>
void random_test(Compare cmp) {
  using C = IndexedSetBase<I,Compare,Multi>;
  using R = std::conditional_t<Multi,std::multiset<I,Compare>,std::set<I,Compare>>;
  std::vector<I> v;
  for (I k=0;k<257;++k) v.push_back(I(rng()%101)-50);
  C st(v,cmp);
  R ref(v.begin(),v.end(),cmp);
  verify(st,ref,0,true);
  for (step=0;step<30000;++step) {
    I x=I(rng()%301)-150;
    if (step%2000 == 1999) {
      st.clear(); ref.clear();
    } else switch (rng()%12) {
      case 0: case 1: case 2:
        if constexpr(Multi) { IDX_CHECK(st.insert(x)); ref.insert(x); }
        else { IDX_CHECK(st.insert(x)==ref.insert(x).second); }
        break;
      case 3: {
        auto it=ref.find(x);
        bool found=it!=ref.end();
        if constexpr(Multi) { IDX_CHECK(st.erase_one(x)==found); }
        else { IDX_CHECK(st.erase(x)==found); }
        if(found) ref.erase(it);
        break;
      }
      case 4:
        if constexpr(Multi) {
          I k=rng()%40;
          IDX_CHECK(st.insert(x,k)==k);
          for(I j=0;j<k;++j) ref.insert(x);
        } else { IDX_CHECK(st.insert(x)==ref.insert(x).second); }
        break;
      case 5:
        if constexpr(Multi) {
          I k=rng()%50, cnt=0;
          auto it=ref.lower_bound(x);
          while(cnt<k && it!=ref.end() && !cmp(x,*it)) {
            it=ref.erase(it); ++cnt;
          }
          IDX_CHECK(st.erase_many(x,k)==cnt);
        } else { IDX_CHECK(st.erase(x)==(ref.erase(x)!=0)); }
        break;
      case 6:
        if constexpr(Multi) { IDX_CHECK(st.erase_all(x)==I(ref.erase(x))); }
        else { IDX_CHECK(st.erase(x)==(ref.erase(x)!=0)); }
        break;
      case 7: case 8:
        if(!ref.empty()) {
          I k=rng()%ref.size();
          auto it=std::next(ref.begin(),k);
          if(step%2) { IDX_CHECK(st.pop(k)==*it); }
          else st.erase_at(k);
          ref.erase(it);
        }
        break;
      case 9:
        if(!ref.empty()) { IDX_CHECK(st.pop_min()==*ref.begin()); ref.erase(ref.begin()); }
        break;
      case 10:
        if(!ref.empty()) { IDX_CHECK(st.pop_max()==*ref.rbegin()); ref.erase(std::prev(ref.end())); }
        break;
      case 11: break;
    }
    I y=I(rng()%401)-200, z=x;
    if(cmp(y,z)) std::swap(y,z);
    IDX_CHECK(st.count(z,y)==std::distance(ref.lower_bound(z),ref.lower_bound(y)));
    verify(st,ref,x,step%97==0);
  }
  verify(st,ref,0,true);
}

void bulk_and_rebuild() {
  IndexedMultiset<I> st;
  std::multiset<I> ref;
  verify(st,ref,0,true);
  IDX_CHECK(st.insert(0,0)==0);
  IDX_CHECK(st.erase_many(0,0)==0);
  IDX_CHECK(st.erase_all(0)==0);
  // 同値要素を多数のバケットに分散させる。
  for (I x : {-1,0,1}) {
    IDX_CHECK(st.insert(x,2048)==2048);
    for(I j=0;j<2048;++j) ref.insert(x);
  }
  for (I x : {-2,-1,0,1,2}) verify(st,ref,x,true);
  for (I k : {1,63,64,65,511,1024,10000}) {
    I cnt=std::min<I>(k,ref.count(0));
    auto it=ref.lower_bound(0);
    ref.erase(it,std::next(it,cnt));
    IDX_CHECK(st.erase_many(0,k)==cnt);
    verify(st,ref,0,true);
  }
  IDX_CHECK(st.erase_all(-1)==I(ref.erase(-1)));
  IDX_CHECK(st.erase_all(1)==I(ref.erase(1)));
  verify(st,ref,0,true);
  // 単一要素の連続挿入、縮小、再利用。昇順/降順で端の分割を誘発。
  IndexedSet<I> unique;
  std::set<I> oracle;
  for(I round=0;round<2;++round) {
    for(I k=0;k<4096;++k) {
      I x=round ? -k : k;
      IDX_CHECK(unique.insert(x)); oracle.insert(x);
      if(k%127==0) verify(unique,oracle,x,false);
    }
    verify(unique,oracle,0,true);
    while(!oracle.empty()) {
      I x=round ? *oracle.rbegin() : *oracle.begin();
      IDX_CHECK((round ? unique.pop_max() : unique.pop_min())==x);
      oracle.erase(x);
      if(oracle.size()%127==0) verify(unique,oracle,x,false);
    }
    verify(unique,oracle,0,true);
  }
}

// 比較上は同値でも実際の値が異なる場合の安定順序。
struct KeyCompare {
  bool descending;
  KeyCompare()=delete;
  explicit KeyCompare(bool d):descending(d) {}
  bool operator()(const std::pair<I,I>& a,const std::pair<I,I>& b) const {
    return descending ? a.first>b.first : a.first<b.first;
  }
};
void equivalent_keys() {
  using P=std::pair<I,I>;
  KeyCompare cmp(true);
  std::vector<P> v;
  for(I j=0;j<512;++j) v.emplace_back(j%3,j);
  IndexedMultiset<P,KeyCompare> st(v,cmp);
  std::multiset<P,KeyCompare> ref(v.begin(),v.end(),cmp);
  verify(st,ref,P{1,-1},true);
  for(I j=512;j<1024;++j) { P x{1,j}; IDX_CHECK(st.insert(x)); ref.insert(x); }
  IDX_CHECK(st.insert(P{1,2048},128)==128);
  for(I j=0;j<128;++j) ref.insert(P{1,2048});
  verify(st,ref,P{1,-1},true);
  IDX_CHECK(st.erase_one(P{1,-1})); ref.erase(ref.lower_bound(P{1,-1}));
  IDX_CHECK(st.erase_many(P{1,-1},300)==300);
  auto it=ref.lower_bound(P{1,-1}); ref.erase(it,std::next(it,300));
  verify(st,ref,P{1,-1},true);
  auto sit=st.begin(); IDX_CHECK(sit->first==ref.begin()->first);
  for(auto [x,y]:st) { (void)x; (void)y; }
  IDX_CHECK(st.erase_all(P{1,-1})==I(ref.erase(P{1,-1})));
  verify(st,ref,P{1,-1},true);
  IndexedSet<P,KeyCompare> unique(cmp);
  IDX_CHECK(unique.insert(P{1,10}));
  IDX_CHECK(!unique.insert(P{1,20}));
  IDX_CHECK(unique.contains(P{1,99}));
  IDX_CHECK(unique[0]==P(1,10));
  IDX_CHECK(unique.erase(P{1,99}));
  IDX_CHECK(unique.empty());
  // set の一括構築でどの同値要素が残るかは規定しない。
  IndexedSet<P,KeyCompare> built(v,cmp);
  IDX_CHECK(built.size()==3);
  IDX_CHECK(built[0].first==2 && built[1].first==1 && built[2].first==0);
}

void types_and_iterators() {
  static_assert(std::bidirectional_iterator<IndexedSet<I>::iterator>);
  static_assert(std::bidirectional_iterator<IndexedMultiset<bool>::iterator>);
  static_assert(std::is_same_v<idx_set<I>,IndexedSet<I>>);
  static_assert(std::is_same_v<idx_multiset<I>,IndexedMultiset<I>>);
  IndexedMultiset<bool> st(std::vector<bool>{true,false,true});
  std::multiset<bool> ref{true,false,true};
  verify(st,ref,false,true);
  IDX_CHECK(st.insert(false,64)==64);
  for(I j=0;j<64;++j) ref.insert(false);
  verify(st,ref,true,true);
  IDX_CHECK(st.erase_all(false)==I(ref.erase(false)));
  verify(st,ref,true,true);
  IndexedSet<std::tuple<I,I,I>> tuples;
  IDX_CHECK(tuples.insert({1,2,3}));
  IDX_CHECK(tuples.insert({0,9,8}));
  auto [x,y,z]=*tuples.begin();
  IDX_CHECK(x==0 && y==9 && z==8);
  auto copy=tuples;
  IDX_CHECK(copy.pop_min()==std::tuple<I,I,I>{0,9,8});
  IDX_CHECK(tuples.size()==2 && copy.size()==1);
}
#undef IDX_CHECK
} // namespace indexed_set_test

int main() {
  using namespace indexed_set_test;
  phase="bulk and rebuild"; bulk_and_rebuild();
  phase="set ascending"; random_test<false>(std::less<I>{});
  phase="set descending"; random_test<false>(std::greater<I>{});
  phase="multiset ascending"; random_test<true>(std::less<I>{});
  phase="multiset descending"; random_test<true>(std::greater<I>{});
  step=-1;
  phase="equivalent keys"; equivalent_keys();
  phase="types and iterators"; types_and_iterators();
  std::cout << "IndexedSet / IndexedMultiset: all tests passed (seed=" << seed << ")\n";
}
