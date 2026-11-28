#include <Library/data_structure/indexed_set.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace {

using Clock=std::chrono::steady_clock;
using Flags=std::vector<unsigned char>;
constexpr ll Q=200'000;
constexpr int trials=5;
constexpr ull seed=0x494E445345543935ULL;
constexpr std::uint32_t key_mask=0x7fffffffU;
constexpr std::uint32_t key_multiplier=0x9e3779b9U;
constexpr std::uint32_t key_inverse=0x144cbc89U;
static_assert(key_multiplier*key_inverse==1);
constexpr std::array names{"IndexedSet", "std::set", "unordered_no_reserve", "unordered_reserve"};

enum class Action { insert, erase, contains };
enum class Kind { incremental, bulk, contains, insert, erase, mixed, abc, scan, ascending };

struct Operation {
  Action action;
  ll key;
};

struct Workload {
  std::string_view name;
  Kind kind;
  const vl& initial;
  std::vector<Operation> operations{};
};

struct Reference {
  Flags present, results;
  vl ascending;
  ll size, min_size, max_size;
  ull sum=0;
};

using Times=std::array<double,4>;

ll key(ll id){
  // 上位31ビットは要素番号に戻せる．下位32ビットを別に混ぜてバケットへの偏りを抑える．
  const auto high=(std::uint32_t(id)*key_multiplier)&key_mask;
  auto low=std::uint32_t(id)+std::uint32_t(seed);
  low=(low^(low>>16))*std::uint32_t(0x85ebca6bU);
  low=(low^(low>>13))*std::uint32_t(0xc2b2ae35U);
  low^=low>>16;
  return ll((ull(high)<<32)|low);
}

ll id_of(ll value){
  return (std::uint32_t(ull(value)>>32)*key_inverse)&key_mask;
}

Reference reference(const Workload& workload,ll n){
  Reference ref{Flags(n+2*Q),{}, {},n,n,n};
  for(ll value:workload.initial){
    ref.present[id_of(value)]=1;
    ref.sum+=ull(value);
  }
  ref.results.reserve(workload.operations.size());
  for(auto [action,value]:workload.operations){
    auto& present=ref.present[id_of(value)];
    switch(action){
      case Action::insert:
        ref.results.push_back(!present);
        if(!present){ ++ref.size; ref.sum+=ull(value); present=1; }
        break;
      case Action::erase:
        ref.results.push_back(present);
        if(present){ --ref.size; ref.sum-=ull(value); present=0; }
        break;
      case Action::contains:
        ref.results.push_back(present);
        break;
    }
    ref.min_size=std::min(ref.min_size,ref.size);
    ref.max_size=std::max(ref.max_size,ref.size);
  }
  if(workload.kind==Kind::abc || workload.kind==Kind::ascending){
    ref.ascending.reserve(ref.size);
    for(ll id=0;id<std::ssize(ref.present);++id)
      if(ref.present[id]) ref.ascending.push_back(key(id));
    std::sort(ref.ascending.begin(),ref.ascending.end());
  }
  return ref;
}

struct Trial {
  ll n;
  std::string_view workload,container;
  int repeat;

  [[noreturn]] void fail(std::string_view reason,ll step=-1) const {
    std::cerr << "[FAIL] N=" << n << " workload=" << workload << " container=" << container
              << " trial=" << repeat << " seed=" << seed << " step=" << step
              << ' ' << reason << '\n';
    std::exit(1);
  }

  template<class Set>
  void verify(const Set& set,const Reference& ref) const {
    if(ll(set.size())!=ref.size) fail("final size differs");
    Flags remaining=ref.present;
    ll visited=0;
    ull sum=0;
    for(ll value:set){
      const ll id=id_of(value);
      if(id>=std::ssize(remaining) || value!=key(id) || !remaining[id])
        fail("unexpected or duplicate final element",id);
      remaining[id]=0;
      ++visited;
      sum+=ull(value);
    }
    if(visited!=ref.size || sum!=ref.sum) fail("final elements differ");
  }

  void verify_results(const Flags& results,const Reference& ref) const {
    const auto diff=std::mismatch(results.begin(),results.end(),ref.results.begin());
    if(diff.first!=results.end()) fail("operation result differs",diff.first-results.begin());
  }
};

double ms(Clock::time_point start,Clock::time_point finish){
  return std::chrono::duration<double,std::milli>(finish-start).count();
}

template<class T>
void observe(const T& value){
  asm volatile("" : : "g"(&value) : "memory");
}

template<bool Reserve,class Set>
void prepare(Set& set,ll capacity){
  if constexpr(Reserve) set.reserve(capacity);
}

template<class Set>
bool insert(Set& set,ll value){
  if constexpr(std::is_same_v<Set,IndexedSet<ll>>) return set.insert(value);
  else return set.insert(value).second;
}

template<class Set>
ll insert_each(Set& set,const vl& values){
  ll inserted=0;
  for(ll value:values) inserted+=insert(set,value);
  return inserted;
}

template<class Set,bool Reserve>
Set build(const vl& values,ll capacity){
  if constexpr(std::is_same_v<Set,IndexedSet<ll>>) return Set(values);
  else if constexpr(Reserve){
    Set set;
    prepare<true>(set,capacity);
    set.insert(values.begin(),values.end());
    return set;
  }else return Set(values.begin(),values.end());
}

template<class Set>
vl ascending(const Set& set){
  vl values;
  values.reserve(set.size());
  for(ll value:set) values.push_back(value);
  if constexpr(std::is_same_v<Set,std::unordered_set<ll>>)
    std::sort(values.begin(),values.end());
  return values;
}

template<class Set>
void apply(Set& set,Kind kind,const std::vector<Operation>& ops,Flags& results){
  if(kind==Kind::contains){
    for(size_t i=0;i<ops.size();++i) results[i]=set.contains(ops[i].key);
  }else if(kind==Kind::insert){
    for(size_t i=0;i<ops.size();++i) results[i]=insert(set,ops[i].key);
  }else if(kind==Kind::erase){
    for(size_t i=0;i<ops.size();++i) results[i]=set.erase(ops[i].key)!=0;
  }else{
    for(size_t i=0;i<ops.size();++i){
      auto [action,value]=ops[i];
      switch(action){
        case Action::insert: results[i]=insert(set,value); break;
        case Action::erase: results[i]=set.erase(value)!=0; break;
        case Action::contains: results[i]=set.contains(value); break;
      }
    }
  }
}

template<class Set,bool Reserve>
Times abc_sample(const Workload& w,const Reference& ref,const Trial& trial){
  Flags results(w.operations.size());
  const auto start=Clock::now();
  Set set;
  prepare<Reserve>(set,ref.max_size);
  const ll inserted=insert_each(set,w.initial);
  observe(set);
  const auto built=Clock::now();

  for(size_t i=0;i<w.operations.size();i+=2){
    results[i]=set.erase(w.operations[i].key)!=0;
    results[i+1]=insert(set,w.operations[i+1].key);
  }
  observe(set);
  observe(results);
  const auto updated=Clock::now();

  auto sorted=ascending(set);
  observe(sorted);
  const auto finish=Clock::now();

  if(inserted!=trial.n) trial.fail("initial insertion count differs");
  trial.verify_results(results,ref);
  if(sorted!=ref.ascending) trial.fail("ascending result differs");
  trial.verify(set,ref);
  return {ms(start,built),ms(built,updated),ms(updated,finish),ms(start,finish)};
}

template<class Set,bool Reserve=false>
Times sample(const Workload& w,const Reference& ref,const Trial& trial){
  if(w.kind==Kind::abc) return abc_sample<Set,Reserve>(w,ref,trial);
  if(w.kind==Kind::incremental){
    const auto start=Clock::now();
    Set set;
    prepare<Reserve>(set,ref.max_size);
    const ll inserted=insert_each(set,w.initial);
    observe(set);
    const auto finish=Clock::now();
    if(inserted!=trial.n) trial.fail("initial insertion count differs");
    trial.verify(set,ref);
    return {ms(start,finish)};
  }
  if(w.kind==Kind::bulk){
    const auto start=Clock::now();
    auto set=build<Set,Reserve>(w.initial,ref.max_size);
    observe(set);
    const auto finish=Clock::now();
    trial.verify(set,ref);
    return {ms(start,finish)};
  }

  auto set=build<Set,Reserve>(w.initial,ref.max_size);
  double elapsed;
  if(w.kind==Kind::scan){
    const auto start=Clock::now();
    const ull sum=std::accumulate(set.begin(),set.end(),0ULL);
    observe(sum);
    elapsed=ms(start,Clock::now());
    if(sum!=ref.sum) trial.fail("unordered traversal checksum differs");
  }else if(w.kind==Kind::ascending){
    const auto start=Clock::now();
    auto sorted=ascending(set);
    observe(sorted);
    elapsed=ms(start,Clock::now());
    if(sorted!=ref.ascending) trial.fail("ascending result differs");
  }else{
    Flags results(w.operations.size());
    const auto start=Clock::now();
    apply(set,w.kind,w.operations,results);
    observe(set);
    observe(results);
    elapsed=ms(start,Clock::now());
    trial.verify_results(results,ref);
  }
  trial.verify(set,ref);
  return {elapsed};
}

void benchmark(ll n,const Workload& workload){
  const auto ref=reference(workload,n);
  ull result_checksum=0;
  for(unsigned char result:ref.results) result_checksum=result_checksum*1000003+result;
  std::array<ll,3> counts{};
  for(auto op:workload.operations) ++counts[size_t(op.action)];
  std::cout << "CASE " << n << ' ' << workload.name << " size=" << n << "->" << ref.size
            << " min=" << ref.min_size << " max=" << ref.max_size
            << " insert=" << counts[0] << " erase=" << counts[1] << " contains=" << counts[2]
            << " result_checksum=" << result_checksum << " final_sum=" << ref.sum << '\n';

  const bool abc=workload.kind==Kind::abc;
  const int metrics=abc ? 4 : 1;
  const ll count=workload.operations.empty() ? n : std::ssize(workload.operations);
  const std::array denominators{abc ? n : count,Q,n,0LL};
  constexpr std::array phases{"build","update","ascending","total"};
  std::array<std::array<Times,trials>,names.size()> readings{};
  for(int repeat=0;repeat<trials;++repeat){
    for(int offset=0;offset<int(names.size());++offset){
      const int index=(repeat+offset)%int(names.size());
      const Trial trial{n,workload.name,names[index],repeat+1};
      auto& result=readings[index][repeat];
      switch(index){
        case 0: result=sample<IndexedSet<ll>>(workload,ref,trial); break;
        case 1: result=sample<std::set<ll>>(workload,ref,trial); break;
        case 2: result=sample<std::unordered_set<ll>>(workload,ref,trial); break;
        case 3: result=sample<std::unordered_set<ll>,true>(workload,ref,trial); break;
      }
      for(int metric=0;metric<metrics;++metric){
        const std::string_view phase=abc ? phases[metric] : "main";
        std::cout << "TRIAL " << n << ' ' << workload.name << ' ' << phase << ' '
                  << names[index] << ' ' << trial.repeat << ' ' << result[metric] << '\n';
      }
    }
  }

  for(int metric=0;metric<metrics;++metric){
    const std::string_view phase=abc ? phases[metric] : "main";
    for(size_t index=0;index<names.size();++index){
      std::array<double,trials> times{};
      for(int repeat=0;repeat<trials;++repeat) times[repeat]=readings[index][repeat][metric];
      std::sort(times.begin(),times.end());
      const double median=times[trials/2];
      const ll operations=denominators[metric];
      std::cout << "RESULT " << n << ' ' << workload.name << ' ' << phase << ' '
                << operations << ' ' << names[index] << ' '
                << median << ' ' << times.front() << ' ' << times.back() << ' ';
      if(operations>0) std::cout << median*1'000'000/operations;
      else std::cout << '-';
      std::cout << '\n';
    }
  }
}

std::vector<Operation> queries(ll n,int hits,std::mt19937_64& rng){
  std::uniform_int_distribution<ll> existing(0,n-1),missing(0,Q-1);
  std::vector<Operation> ops;
  ops.reserve(Q);
  for(ll i=0;i<Q;++i)
    ops.push_back({Action::contains,key(i<hits ? existing(rng) : n+Q+missing(rng))});
  if(0<hits && hits<Q) std::shuffle(ops.begin(),ops.end(),rng);
  return ops;
}

std::vector<Operation> replacements(ll n,bool abc,std::mt19937_64& rng){
  vl active(n);
  std::iota(active.begin(),active.end(),0LL);
  std::uniform_int_distribution<ll> slot(0,n-1),missing(0,Q-1);
  std::vector<Operation> ops;
  ops.reserve(abc ? 2*Q : Q);
  for(ll i=0;i<(abc ? Q : Q/4);++i){
    const ll position=slot(rng);
    const ll old=key(active[position]),fresh=key(n+i);
    if(abc){
      ops.push_back({Action::erase,old});
      ops.push_back({Action::insert,fresh});
    }else{
      ops.push_back({Action::insert,fresh});
      ops.push_back({Action::contains,fresh});
      ops.push_back({Action::erase,old});
      ops.push_back({Action::contains,key(n+Q+missing(rng))});
    }
    active[position]=n+i;
  }
  return ops;
}

void run(ll n){
  std::mt19937_64 rng(seed+ull(n));
  vl shuffled(n);
  for(ll i=0;i<n;++i) shuffled[i]=key(i);
  vl sorted=shuffled;
  std::sort(sorted.begin(),sorted.end());
  const vl reversed(sorted.rbegin(),sorted.rend());
  std::shuffle(shuffled.begin(),shuffled.end(),rng);
  benchmark(n,{"insert_random",Kind::incremental,shuffled});
  benchmark(n,{"insert_ascending",Kind::incremental,sorted});
  benchmark(n,{"insert_descending",Kind::incremental,reversed});
  benchmark(n,{"bulk",Kind::bulk,shuffled});
  benchmark(n,{"contains_hit",Kind::contains,shuffled,queries(n,Q,rng)});
  benchmark(n,{"contains_miss",Kind::contains,shuffled,queries(n,0,rng)});
  benchmark(n,{"contains_mixed",Kind::contains,shuffled,queries(n,Q/2,rng)});

  std::vector<Operation> insertions,deletions;
  insertions.reserve(Q);
  for(ll i=0;i<Q;++i) insertions.push_back({Action::insert,key(n+i)});
  std::shuffle(insertions.begin(),insertions.end(),rng);
  benchmark(n,{"insert_new",Kind::insert,shuffled,std::move(insertions)});
  const ll erase_count=std::min(n,Q);
  deletions.reserve(erase_count);
  for(ll i=0;i<erase_count;++i) deletions.push_back({Action::erase,shuffled[i]});
  benchmark(n,{"erase_existing",Kind::erase,shuffled,std::move(deletions)});

  benchmark(n,{"mixed",Kind::mixed,shuffled,replacements(n,false,rng)});
  benchmark(n,{"abc486b",Kind::abc,shuffled,replacements(n,true,rng)});
  benchmark(n,{"scan",Kind::scan,shuffled});
  benchmark(n,{"ascending",Kind::ascending,shuffled});
}

} // namespace

int main(){
  std::cout << std::fixed << std::setprecision(6)
            << "IndexedSet benchmark seed=" << seed << " Q=" << Q << " trials=" << trials << '\n'
            << "TRIAL N workload phase container trial ms\n"
            << "RESULT N workload phase operations container median_ms min_ms max_ms median_ns/op\n";
  for(ll n:{1'000,10'000,100'000,200'000,1'000'000}) run(n);
  std::cout << "[PASS] All operation results and final sets matched.\n";
}
