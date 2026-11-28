#include <Library/data_structure/union_find.hpp>
#include <Library/core/common_setting.hpp>
#include <Library/old/data_structure/UnionFind.hpp>
#include <atcoder/dsu>

namespace {
using Clock=chrono::steady_clock;
using Edges=vector<pair<int,int>>;
constexpr int repeats=5;
constexpr ull seed=0x554642454e4348;

struct Sample {double ns; ull checksum;};

template<class Tree>
void observe(const Tree& tree){ asm volatile("" : : "g"(&tree) : "memory"); }

template<class Tree>
bool unite(Tree& tree,int x,int y){
  if constexpr(is_same_v<Tree,atcoder::dsu>){
    if(tree.same(x,y)) return false;
    tree.merge(x,y);
    return true;
  }else return tree.merge(x,y);
}

template<class F>
void compare(int n,string_view label,int count,F measure){
  array<array<double,repeats>,3> times{};
  optional<ull> expected;
  for(int r=0;r<repeats;++r) for(int offset=0;offset<3;++offset){
    int k=(r+offset)%3;
    Sample s;
    if(k==0) s=measure.template operator()<UnionFind>();
    else if(k==1) s=measure.template operator()<DSU>();
    else s=measure.template operator()<atcoder::dsu>();
    if(!expected) expected=s.checksum;
    if(s.checksum!=*expected) throw runtime_error("UnionFind checksum mismatch");
    times[k][r]=s.ns/count;
  }
  cout<<n<<' '<<label<<' '<<count;
  for(auto& t:times){ranges::sort(t);cout<<' '<<t[2];}
  cout<<' '<<*expected<<'\n';
}

template<class F>
void operation(int n,string_view label,const Edges& setup,const Edges& q,F execute){
  compare(n,label,int(q.size()),[&]<class Tree>{
    Tree tree(n);
    for(auto [x,y]:setup) tree.merge(x,y);
    ull sum=0;
    auto start=Clock::now();
    for(size_t i=0;i<q.size();++i){
      auto [x,y]=q[i];
      sum+=execute(tree,x,y,i);
    }
    observe(tree);
    auto stop=Clock::now();
    sum+=ull(tree.size(n-1))+ull(tree.leader(n-1));
    return Sample{chrono::duration<double,nano>(stop-start).count(),sum};
  });
}

Edges balanced(int n,int block){
  Edges result;
  for(int stride=1;stride<block && stride<n;stride*=2)
    for(int x=0;x+stride<n;x+=2*stride) result.emplace_back(x,x+stride);
  return result;
}

ull fold(ull sum,ull x){ return sum*0x100000001b3ULL+x; }

void groups_case(int n,int block,string_view label){
  auto setup=balanced(n,block);
  compare(n,label,n,[&]<class Tree>{
    Tree before(n);
    for(auto [x,y]:setup) before.merge(x,y);
    Tree tree=before;
    auto start=Clock::now();
    auto g=tree.groups();
    auto stop=Clock::now();
    ull checksum=g.size();
    for(const auto& group:g){
      checksum=fold(checksum,group.size());
      for(ll x:group) checksum=fold(checksum,ull(x));
    }
    return Sample{chrono::duration<double,nano>(stop-start).count(),checksum};
  });
}

void run(int n){
  mt19937_64 rng(seed);
  Edges q(200000);
  for(auto& [a,b]:q){a=int(rng()%n);b=int(rng()%n);}
  const auto setup=balanced(n,64);
  int builds=max(4,1'000'000/n);
  compare(n,"build",builds,[&]<class Tree>{
    double ns=0;
    ull checksum=0;
    for(int i=0;i<builds;++i){
      auto start=Clock::now();
      Tree tree(n);
      observe(tree);
      auto stop=Clock::now();
      ns+=chrono::duration<double,nano>(stop-start).count();
      checksum+=tree.size(n-1);
    }
    return Sample{ns,checksum};
  });
  operation(n,"merge",{},q,[](auto& t,int x,int y,size_t){return ull(unite(t,x,y));});
  operation(n,"leader",setup,q,[](auto& t,int x,int,size_t){return ull(t.leader(x));});
  operation(n,"same",setup,q,[](auto& t,int x,int y,size_t){return ull(t.same(x,y));});
  operation(n,"size",setup,q,[](auto& t,int x,int,size_t){return ull(t.size(x));});
  operation(n,"mixed",setup,q,[](auto& t,int x,int y,size_t i){
    switch(i%4){
      case 0: return ull(unite(t,x,y));
      case 1: return ull(t.leader(x));
      case 2: return ull(t.same(x,y));
      default: return ull(t.size(x));
    }
  });
  // 辺は重み i+1 の昇順で与え，ソートを含めず採択辺の重みを照合する．
  operation(n,"kruskal",{},q,[](auto& t,int x,int y,size_t i){
    return unite(t,x,y) ? ull(i+1) : 0ULL;
  });
  groups_case(n,1,"groups-singletons");
  groups_case(n,bit_ceil(unsigned(n)/4),"groups-large");
  groups_case(n,8,"groups-small");
  groups_case(n,n,"groups-one");
}
}

int main(){
  cout<<fixed<<setprecision(3)<<"seed "<<seed<<" repeats "<<repeats<<'\n';
  cout<<"sizeof "<<sizeof(UnionFind)<<' '<<sizeof(DSU)<<' '<<sizeof(atcoder::dsu)<<'\n';
  cout<<"N operation count own_ns old_ns ACL_ns checksum\n";
  for(int n:{1024,65536,200000,1000000}) run(n);
}
