#include <Library/string/rolling_hash.hpp>

using Clock=chrono::steady_clock;
volatile ull sink=0;
inline ull digest(ull x){ return x; }
inline ull digest(pair<ll,ll> x){ return ull(x.first)^ (ull(x.second)<<32); }

template<class RH>
array<double,3> run_benchmark(const string& s,const vl& a,const vector<pair<ll,ll>>& q){
  auto zero=RH().get_all();
  RH::concat(zero,zero,ssize(s)); // warm shared power table
  auto start=Clock::now();
  RH rh(s);
  auto mid=Clock::now();
  ull checksum=0;
  for(auto [l,r]:q) checksum+=digest(rh.get(l,r));
  auto end=Clock::now();
  sink=checksum;
  auto t=Clock::now();
  RH ra(a);
  auto u=Clock::now();
  sink=digest(ra.get_all());
  auto ms=[](auto x,auto y){ return chrono::duration<double,milli>(y-x).count(); };
  return {ms(start,mid),ms(mid,end),ms(t,u)};
}
int main(){
  constexpr ll N=1000000,Q=2000000;
  mt19937_64 rng(123456);
  string s(N,'a');
  for(char& c:s) c=char('a'+rng()%26);
  vl a(N);
  for(ll& x:a) x=bit_cast<ll>(rng());
  vector<pair<ll,ll>> q(Q);
  for(auto& [l,r]:q){ l=rng()%(N+1); r=rng()%(N+1); if(l>r) swap(l,r); }
  vector<array<double,3>> d,m;
  for(int i=0;i<5;i++){
    if(i%2==0){ d.push_back(run_benchmark<rolling_hash>(s,a,q)); m.push_back(run_benchmark<rolling_hash61>(s,a,q)); }
    else { m.push_back(run_benchmark<rolling_hash61>(s,a,q)); d.push_back(run_benchmark<rolling_hash>(s,a,q)); }
  }
  auto print=[](const char* name, const auto& rows){
    cout << name;
    for(int j=0;j<3;j++){
      vector<double> v;
      for(auto r:rows) v.push_back(r[j]);
      sort(v.begin(),v.end());
      cout << ' ' << fixed << setprecision(3) << v[2];
    }
    cout << '\n';
  };
  cout << "Median ms: string build / range queries / integer build\n";
  print("Double mod",d); print("Mersenne61",m);
}
