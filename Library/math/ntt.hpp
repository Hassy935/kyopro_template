#pragma once
#include <Library/math/modint.hpp>

namespace ntt_detail {

constexpr bool is_prime(ll n){
  if(n<2 || n%2==0) return n==2;
  for(ll p=3;p<=n/p;p+=2) if(n%p==0) return false;
  return true;
}

template<ll MOD>
constexpr static_modint<MOD> primitive_root(){
  using Mint=static_modint<MOD>;
  if constexpr(MOD==2) return 1;
  std::array<ll,16> factors{};
  int count=0;
  ll n=MOD-1;
  for(ll p=2;p<=n/p;++p){
    if(n%p!=0) continue;
    factors[count++]=p;
    do { n/=p; } while(n%p==0);
  }
  if(n>1) factors[count++]=n;
  for(ll g=2;g<MOD;++g){
    bool generator=true;
    for(int i=0;i<count;++i){
      if(Mint(g).pow((MOD-1)/factors[i])==1){
        generator=false;
        break;
      }
    }
    if(generator) return g;
  }
  return 0;
}

template<ll MOD>
struct Plan {
  static_assert(is_prime(MOD),"NTT requires a prime modulus");
  using Mint=static_modint<MOD>;
  static constexpr int rank=std::countr_zero(static_cast<unsigned>(MOD-1));
  static constexpr size_t max_length=size_t(1)<<rank;
  static constexpr Mint generator=primitive_root<MOD>();

  struct Tables {
    std::array<Mint,rank+1> root{},inverse_root{},scale{};
    std::array<Mint,rank+1> step2{},step4{},inverse_step2{},inverse_step4{};

    constexpr Tables(){
      root[rank]=generator.pow((MOD-1)>>rank);
      inverse_root[rank]=root[rank].inv();
      scale[rank]=Mint(max_length).inv();
      for(int i=rank;i>0;--i){
        root[i-1]=root[i]*root[i];
        inverse_root[i-1]=inverse_root[i]*inverse_root[i];
        scale[i-1]=scale[i]+scale[i];
      }
      // ブロック番号の末尾 t 個の1が落ちるときの回転因子比．
      for(int radix_bits:{1,2}){
        Mint down=1,up=1;
        auto& steps=radix_bits==1 ? step2 : step4;
        auto& inverse_steps=radix_bits==1 ? inverse_step2 : inverse_step4;
        for(int i=radix_bits+1;i<=rank;++i){
          steps[i-radix_bits-1]=root[i]*down;
          inverse_steps[i-radix_bits-1]=inverse_root[i]*up;
          down*=inverse_root[i];
          up*=root[i];
        }
      }
    }
  };

  inline static constexpr Tables table{};
};

template<ll MOD>
bool valid_length(size_t n){
  if(std::has_single_bit(n) && n<=Plan<MOD>::max_length) return true;
  assert(false && "NTT requires a supported positive power of two");
  return false;
}

template<class T>
void reverse_bits(std::vector<T>& a){
  for(size_t i=1,j=0;i<a.size();++i){
    size_t bit=a.size()>>1;
    while(j&bit){ j^=bit; bit>>=1; }
    j^=bit;
    if(i<j) std::swap(a[i],a[j]);
  }
}

// 通常順から，正指数の周波数をビット反転順に格納する．
template<ll MOD>
void forward(std::vector<static_modint<MOD>>& a){
  using Mint=static_modint<MOD>;
  if constexpr(Plan<MOD>::rank<2){
    if(a.size()==2){
      Mint x=a[0],y=a[1];
      a[0]=x+y;
      a[1]=x-y;
    }
    return;
  }
  constexpr auto& table=Plan<MOD>::table;
  const int rank=std::countr_zero(a.size());
  int used=0;
  while(used<rank){
    const size_t blocks=size_t(1)<<used;
    Mint rot=1;
    if(rank-used==1){
      for(size_t s=0;s<blocks;++s){
        Mint x=a[2*s],y=a[2*s+1]*rot;
        a[2*s]=x+y;
        a[2*s+1]=x-y;
        if(s+1<blocks) rot*=table.step2[std::countr_one(s)];
      }
      ++used;
    }else{
      const size_t width=size_t(1)<<(rank-used-2);
      for(size_t s=0;s<blocks;++s){
        Mint rot2=rot*rot,rot3=rot2*rot;
        auto* row=a.data()+4*s*width;
        for(size_t j=0;j<width;++j){
          // 積を先に還元せず，法の倍数で負の差を避ける．全中間値は 4*MOD^2 未満．
          constexpr uint64_t square=uint64_t(MOD)*MOD;
          uint64_t x0=row[j].val(),x1=uint64_t(row[j+width].val())*rot.val();
          uint64_t x2=uint64_t(row[j+2*width].val())*rot2.val();
          uint64_t x3=uint64_t(row[j+3*width].val())*rot3.val();
          uint64_t even=x0+x2,odd=x1+x3,diff=x0+square-x2;
          uint64_t cross=((x1+square-x3)%MOD)*table.root[2].val();
          // 4点 DFT の出力を周波数 0, 2, 1, 3 の順で置く．
          row[j]=Mint::raw((even+odd)%MOD);
          row[j+width]=Mint::raw((even+2*square-odd)%MOD);
          row[j+2*width]=Mint::raw((diff+cross)%MOD);
          row[j+3*width]=Mint::raw((diff+square-cross)%MOD);
        }
        if(s+1<blocks) rot*=table.step4[std::countr_one(s)];
      }
      used+=2;
    }
  }
}

// ビット反転順の周波数から通常順へ戻す．正規化は行わない．
template<ll MOD>
void inverse(std::vector<static_modint<MOD>>& a){
  using Mint=static_modint<MOD>;
  if constexpr(Plan<MOD>::rank<2){
    if(a.size()==2){
      Mint x=a[0],y=a[1];
      a[0]=x+y;
      a[1]=x-y;
    }
    return;
  }
  constexpr auto& table=Plan<MOD>::table;
  const int rank=std::countr_zero(a.size());
  for(int used=rank;used>0;){
    Mint rot=1;
    if(used%2!=0){
      const size_t blocks=size_t(1)<<(used-1);
      for(size_t s=0;s<blocks;++s){
        Mint x=a[2*s],y=a[2*s+1];
        a[2*s]=x+y;
        a[2*s+1]=(x-y)*rot;
        if(s+1<blocks) rot*=table.inverse_step2[std::countr_one(s)];
      }
      --used;
    }else{
      const size_t blocks=size_t(1)<<(used-2);
      const size_t width=size_t(1)<<(rank-used);
      for(size_t s=0;s<blocks;++s){
        Mint rot2=rot*rot,rot3=rot2*rot;
        auto* row=a.data()+4*s*width;
        for(size_t j=0;j<width;++j){
          uint64_t f0=row[j].val(),f2=row[j+width].val();
          uint64_t f1=row[j+2*width].val(),f3=row[j+3*width].val();
          uint64_t even=f0+f2,odd=f1+f3,diff=f0+MOD-f2;
          uint64_t cross=((f1+MOD-f3)*table.inverse_root[2].val())%MOD;
          row[j]=Mint::raw((even+odd)%MOD);
          row[j+width]=Mint::raw(((diff+cross)*rot.val())%MOD);
          row[j+2*width]=Mint::raw(((even+2*MOD-odd)*rot2.val())%MOD);
          row[j+3*width]=Mint::raw(((diff+MOD-cross)*rot3.val())%MOD);
        }
        if(s+1<blocks) rot*=table.inverse_step4[std::countr_one(s)];
      }
      used-=2;
    }
  }
}

template<ll MOD>
void normalize(std::vector<static_modint<MOD>>& a){
  if constexpr(Plan<MOD>::rank==0) return;
  const auto factor=Plan<MOD>::table.scale[std::countr_zero(a.size())];
  for(auto& x:a) x*=factor;
}

} // namespace ntt_detail

template<ll MOD>
void ntt(std::vector<static_modint<MOD>>& a){
  if(!ntt_detail::valid_length<MOD>(a.size()) || a.size()==1) return;
  ntt_detail::forward(a);
  ntt_detail::reverse_bits(a);
}

template<ll MOD>
void intt(std::vector<static_modint<MOD>>& a){
  if(!ntt_detail::valid_length<MOD>(a.size()) || a.size()==1) return;
  ntt_detail::reverse_bits(a);
  ntt_detail::inverse(a);
  ntt_detail::normalize(a);
}
