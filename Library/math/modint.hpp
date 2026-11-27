#pragma once
#include <Library/core/common_setting.hpp>

namespace modint_detail {

inline constexpr ll max_mod = 2147483647;

template<class T>
concept Integer = std::integral<T> || std::same_as<T,i128> || std::same_as<T,__uint128_t>;

template<Integer T>
constexpr uint32_t normalize(T x, int m){
  if constexpr(std::is_signed_v<T> || std::same_as<T,i128>){
    auto r=x%m;
    return uint32_t(r<0 ? r+m : r);
  }else{
    return uint32_t(x%uint32_t(m));
  }
}

constexpr uint32_t inverse(uint32_t x, int m){
  ll a=m,b=x;
  ll u=0,v=1;
  while(b!=0){
    ll q=a/b;
    a=std::exchange(b,a-q*b);
    u=std::exchange(v,u-q*v);
  }
  assert(a==1);
  return uint32_t(u<0 ? u+m : u);
}

template<ll M>
struct StaticMod {
  static constexpr int mod(){ return int(M); }
  static constexpr uint32_t multiply(uint32_t a,uint32_t b){
    return uint32_t(uint64_t(a)*b%M);
  }
};

template<ll ID>
struct DynamicMod {
  struct Reduction {
    uint32_t modulus;
    uint64_t reciprocal;
  };

  inline static Reduction reduction{998244353,UINT64_MAX/998244353};

  static int mod(){ return int(reduction.modulus); }

  template<Integer T>
  static void set_mod(T m){
    assert(m!=0 && __uint128_t(m)<=max_mod);
    auto modulus=uint32_t(m);
    reduction={modulus,UINT64_MAX/modulus};
  }

  static uint32_t multiply(uint32_t a,uint32_t b){
    const auto [m,r]=reduction;
    uint64_t product=uint64_t(a)*b;
    // r は 2^64/m 未満で，その差は高々1．近似商は真の商以下で差は高々1．
    uint64_t quotient=uint64_t((__uint128_t(product)*r)>>64);
    uint64_t rem=product-quotient*m;
    return uint32_t(rem>=m ? rem-m : rem);
  }
};

template<class Mint,class Mod>
class Operations {
  uint32_t residue=0;

  constexpr Mint& self(){ return static_cast<Mint&>(*this); }
  constexpr const Mint& self() const { return static_cast<const Mint&>(*this); }

public:
  constexpr Operations()=default;

  template<Integer T>
  constexpr Operations(T x): residue(normalize(x,mod())) {}

  static constexpr int mod(){ return Mod::mod(); }
  constexpr int val() const { return int(residue); }

  template<Integer T>
  static constexpr Mint raw(T x){
    assert(__uint128_t(x)<uint32_t(mod()));
    Mint result;
    result.residue=uint32_t(x);
    return result;
  }

  constexpr Mint& operator+=(const Mint& rhs){
    residue+=rhs.residue;
    if(residue>=uint32_t(mod())) residue-=uint32_t(mod());
    return self();
  }

  constexpr Mint& operator-=(const Mint& rhs){
    uint32_t m=uint32_t(mod());
    // 負の差は符号なしの折り返しで m 以上になる．
    residue-=rhs.residue;
    if(residue>=m) residue+=m;
    return self();
  }

  constexpr Mint& operator*=(const Mint& rhs){
    residue=Mod::multiply(residue,rhs.residue);
    return self();
  }

  constexpr Mint& operator/=(const Mint& rhs){ return self()*=rhs.inv(); }

  constexpr Mint operator+() const { return self(); }
  constexpr Mint operator-() const { return Mint{}-self(); }

  constexpr Mint& operator++(){
    if(++residue==uint32_t(mod())) residue=0;
    return self();
  }

  constexpr Mint& operator--(){
    if(residue==0) residue=uint32_t(mod());
    --residue;
    return self();
  }

  constexpr Mint operator++(int){ Mint old=self(); ++*this; return old; }
  constexpr Mint operator--(int){ Mint old=self(); --*this; return old; }

  constexpr Mint pow(ll n) const {
    assert(n>=0);
    Mint result=1,base=self();
    for(;n>0;n>>=1){
      if(n&1) result*=base;
      base*=base;
    }
    return result;
  }

  constexpr Mint inv() const { return raw(inverse(residue,mod())); }

  friend constexpr Mint operator+(Mint lhs,const Mint& rhs){ return lhs+=rhs; }
  friend constexpr Mint operator-(Mint lhs,const Mint& rhs){ return lhs-=rhs; }
  friend constexpr Mint operator*(Mint lhs,const Mint& rhs){ return lhs*=rhs; }
  friend constexpr Mint operator/(Mint lhs,const Mint& rhs){ return lhs/=rhs; }
  friend constexpr bool operator==(const Mint& lhs,const Mint& rhs){
    return lhs.residue==rhs.residue;
  }

  friend constexpr bool operator!=(const Mint& lhs,const Mint& rhs){ return !(lhs==rhs); }

  friend std::ostream& operator<<(std::ostream& os,const Mint& x){ return os<<x.val(); }
  friend std::istream& operator>>(std::istream& is,Mint& x){
    ll value;
    if(is>>value) x=Mint(value);
    return is;
  }
};

} // namespace modint_detail

template<ll MOD>
class static_modint:
    public modint_detail::Operations<static_modint<MOD>,modint_detail::StaticMod<MOD>> {
  static_assert(1<=MOD && MOD<=modint_detail::max_mod);
  using Base=modint_detail::Operations<static_modint<MOD>,modint_detail::StaticMod<MOD>>;
public:
  using Base::Base;
};

template<ll ID=0>
class dynamic_modint:
    public modint_detail::Operations<dynamic_modint<ID>,modint_detail::DynamicMod<ID>> {
  using Mod=modint_detail::DynamicMod<ID>;
  using Base=modint_detail::Operations<dynamic_modint<ID>,Mod>;
public:
  using Base::Base;

  // 法変更前に構築した同一 ID の値は以後使用できない．
  template<modint_detail::Integer T>
  static void set_mod(T m){ Mod::set_mod(m); }
};

using modint998244353=static_modint<998244353>;
using modint1000000007=static_modint<1000000007>;
using modint=dynamic_modint<>;
