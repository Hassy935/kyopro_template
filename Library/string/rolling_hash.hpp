#pragma once
#include <Library/core/common_setting.hpp>

namespace rolling_hash_detail {

inline constexpr ull M=(1ULL<<61)-1;
inline ull encode(ull x){
  x=(x&M)+(x>>61);
  return x>=M?x-M:x;
}
inline ull mul_add(ull a,ull b,ull c){
  __uint128_t x=__uint128_t(a)*b+c;
  ull y=ull(x&M)+ull(x>>61);
  return y>=M?y-M:y;
}
inline ull sub(ull a,ull b){ return a>=b?a-b:a+M-b; }

inline ull mix(ull x){
  x+=0x9e3779b97f4a7c15ULL;
  x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;
  x=(x^(x>>27))*0x94d049bb133111ebULL;
  return x^(x>>31);
}

} // namespace rolling_hash_detail

class RollingHash {
public:
  using P=ull;
  using hash_type=P;

private:
  struct Shared {
    P base;
    ull salt;
    vector<P> power{1};

    Shared(){
      random_device rd;
      ull t=ull(chrono::steady_clock::now().time_since_epoch().count());
      seed_seq seed{rd(),rd(),unsigned(t),unsigned(t>>32)};
      mt19937_64 rng(seed);
      base=uniform_int_distribution<ull>(257,rolling_hash_detail::M-2)(rng);
      salt=rng();
    }

    void ensure(size_t n){
      if(n>=power.max_size()) throw length_error("RollingHash: length too large");
      while(power.size()<=n){
        power.push_back(rolling_hash_detail::mul_add(power.back(),base,0));
      }
    }

    P symbol(ull x) const {
      // Bytes and integer values 0..255 use identical, nonzero codes.
      // Other 64-bit integers use a per-run salted bijective mixer.
      return rolling_hash_detail::encode(x<=255?x+1:rolling_hash_detail::mix(x+salt));
    }
  };

  static Shared& shared(){
    static Shared s;
    return s;
  }

  vector<P> pref{0};

  template<class F>
  void build(size_t n,F value){
    auto& s=shared();
    if(n>=pref.max_size()) throw length_error("RollingHash: length too large");
    s.ensure(n);
    pref.resize(n+1);
    pref[0]=0;
    for(size_t i=0;i<n;i++){
      pref[i+1]=rolling_hash_detail::mul_add(pref[i],s.base,s.symbol(value(i)));
    }
  }

  void check(ll l,ll r) const {
    assert(0<=l&&l<=r&&r<=size());
  }

public:
  RollingHash()=default;
  explicit RollingHash(string_view S){
    build(S.size(),[&](size_t i){ return ull(static_cast<unsigned char>(S[i])); });
  }

  template<integral T> requires (sizeof(T)<=sizeof(ull))
  explicit RollingHash(const vector<T>& A){
    build(A.size(),[&](size_t i){ return ull(A[i]); });
  }

  ll size() const { return pref.empty()?0:ll(pref.size()-1); }
  bool empty() const { return size()==0; }

  P get(ll l,ll r) const {
    check(l,r);
    if(l==r) return 0;
    return rolling_hash_detail::sub(pref[r],rolling_hash_detail::mul_add(pref[l],shared().power[r-l],0));
  }

  P get_all() const { return get(0,size()); }

  static P concat(P h1,P h2,ll len2){
    assert(len2>=0);
    auto& s=shared();
    s.ensure(size_t(len2));
    return rolling_hash_detail::mul_add(h1,s.power[len2],h2);
  }

  bool equal(ll l1,ll r1,const RollingHash& other,ll l2,ll r2) const {
    check(l1,r1);
    other.check(l2,r2);
    return r1-l1==r2-l2&&get(l1,r1)==other.get(l2,r2);
  }

  bool equal(ll l1,ll r1,ll l2,ll r2) const {
    return equal(l1,r1,*this,l2,r2);
  }

  ll lcp(ll l1,ll r1,const RollingHash& other,ll l2,ll r2) const {
    check(l1,r1);
    other.check(l2,r2);
    ll lo=0,hi=std::min(r1-l1,r2-l2);
    while(lo<hi){
      ll mid=lo+(hi-lo)/2+1;
      if(get(l1,l1+mid)==other.get(l2,l2+mid)) lo=mid;
      else hi=mid-1;
    }
    return lo;
  }

  ll lcp(ll l1,ll r1,ll l2,ll r2) const {
    return lcp(l1,r1,*this,l2,r2);
  }

  ll lcp(ll l1,const RollingHash& other,ll l2) const {
    return lcp(l1,size(),other,l2,other.size());
  }

  ll lcp(ll l1,ll l2) const { return lcp(l1,*this,l2); }
};

using rolling_hash=RollingHash;
