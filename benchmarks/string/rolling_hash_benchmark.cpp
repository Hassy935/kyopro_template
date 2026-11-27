#include <Library/string/rolling_hash.hpp>

using Clock = chrono::steady_clock;

volatile ull sink=0;

struct ConcatQuery {
  ull h1,h2;
  ll len2;
};

double ms(Clock::time_point x, Clock::time_point y){
  return chrono::duration<double,milli>(y-x).count();
}

double median(vector<double> v){
  sort(v.begin(),v.end());
  return v[v.size()/2];
}

array<double,4> run_benchmark(
  const string& s,
  const vl& a,
  const vector<pair<ll,ll>>& q,
  const vector<ConcatQuery>& cq
){
  double string_build,integer_build;
  double range_queries,concatenation;

  // Warm string build / Range queries
  {
    auto start=Clock::now();

    RollingHash rh(s);

    auto mid=Clock::now();

    ull checksum=0;
    for(auto [l,r]:q){
      checksum+=rh.get(l,r);
    }

    auto end=Clock::now();

    sink=checksum;

    string_build=ms(start,mid);
    range_queries=ms(mid,end);
  }

  // Warm integer build
  {
    auto start=Clock::now();

    RollingHash rh(a);

    auto end=Clock::now();

    sink=rh.get_all();

    integer_build=ms(start,end);
  }

  // Concatenation
  {
    ull checksum=0;

    auto start=Clock::now();

    for(auto [h1,h2,len2]:cq){
      checksum+=RollingHash::concat(h1,h2,len2);
    }

    auto end=Clock::now();

    sink=checksum;

    concatenation=ms(start,end);
  }

  return {
    string_build,
    integer_build,
    range_queries,
    concatenation
  };
}

int main(){
  constexpr ll N=1000000;
  constexpr ll Q=2000000;
  constexpr ll REPEAT=5;

  mt19937_64 rng(123456);

  // 入力データの生成
  string s(N,'a');

  for(char& c:s){
    c=char('a'+rng()%26);
  }

  vl a(N);

  for(ll& x:a){
    x=bit_cast<ll>(rng());
  }

  vector<pair<ll,ll>> q(Q);

  for(auto& [l,r]:q){
    l=rng()%(N+1);
    r=rng()%(N+1);

    if(l>r) swap(l,r);
  }

  // Cold string build
  double cold_build;
  vector<ConcatQuery> cq;
  cq.reserve(Q);

  {
    auto start=Clock::now();

    RollingHash rh(s);

    auto end=Clock::now();

    sink=rh.get_all();

    cold_build=ms(start,end);

    // concat用の入力を事前生成
    for(auto [l,r]:q){
      ll m=l+(r-l)/2;

      cq.push_back({
        rh.get(l,m),
        rh.get(m,r),
        r-m
      });
    }
  }

  // Warm benchmark
  array<vector<double>,4> times;

  for(ll i=0;i<REPEAT;i++){
    auto res=run_benchmark(s,a,q,cq);

    for(ll j=0;j<4;j++){
      times[j].push_back(res[j]);
    }
  }

  // 計測結果
  cout << "RollingHash (Mersenne 61)\n";
  cout << "N: " << N << '\n';
  cout << "Q: " << Q << '\n';
  cout << "Repeat: " << REPEAT << '\n';
  cout << '\n';

  cout << fixed << setprecision(3);

  auto print=[](const char* name,double time){
    cout << left << setw(24) << name
         << right << setw(10) << time
         << " ms\n";
  };

  cout << "Cold build (single measurement)\n";
  print("String build",cold_build);

  cout << '\n';
  cout << "Warm benchmark (median)\n";

  print("String build",median(times[0]));
  print("Integer build",median(times[1]));
  print("Range queries",median(times[2]));
  print("Concatenation",median(times[3]));

  cout << '\n';
  cout << "Throughput\n";

  cout << left << setw(24) << "Range queries"
       << right << setw(10)
       << median(times[2])*1e6/Q
       << " ns/query\n";

  cout << left << setw(24) << "Concatenation"
       << right << setw(10)
       << median(times[3])*1e6/Q
       << " ns/query\n";

  return 0;
}