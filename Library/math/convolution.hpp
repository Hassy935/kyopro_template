#pragma once
#include <Library/math/ntt.hpp>

namespace convolution_detail {

inline constexpr size_t naive_limit=48;

template<ll MOD>
std::vector<static_modint<MOD>> naive(const std::vector<static_modint<MOD>>& a,
                                      const std::vector<static_modint<MOD>>& b,size_t length){
  std::vector<static_modint<MOD>> result(length);
  for(size_t i=0;i<a.size();++i)
    for(size_t j=0;j<b.size();++j) result[i+j]+=a[i]*b[j];
  return result;
}

template<ll MOD>
void transformed(std::vector<static_modint<MOD>>& a,std::vector<static_modint<MOD>>& b,
                 size_t length){
  if(length>ntt_detail::Plan<MOD>::max_length || std::bit_ceil(length)>a.max_size()){
    assert(false && "convolution: transform length exceeds limit");
    throw std::length_error("convolution: transform length exceeds limit");
  }
  const size_t padded=std::bit_ceil(length);
  a.resize(padded);
  b.resize(padded);
  ntt_detail::forward(a);
  ntt_detail::forward(b);
  for(size_t i=0;i<padded;++i) a[i]*=b[i];
  ntt_detail::inverse(a);
  a.resize(length);
  const auto scale=ntt_detail::Plan<MOD>::table.scale[std::countr_zero(padded)];
  for(auto& x:a) x*=scale;
}

} // namespace convolution_detail

template<ll MOD>
std::vector<static_modint<MOD>> convolution(std::vector<static_modint<MOD>> a,
                                           std::vector<static_modint<MOD>> b){
  static_assert(ntt_detail::is_prime(MOD),"convolution requires a prime modulus");
  if(a.empty() || b.empty()) return {};
  if(a.size()<b.size()) a.swap(b);
  if(b.size()-1>a.max_size()-a.size()){
    assert(false && "convolution: result length exceeds vector::max_size()");
    throw std::length_error("convolution: result length exceeds vector::max_size()");
  }
  const size_t length=a.size()+(b.size()-1);
  if(b.size()==1){
    for(auto& x:a) x*=b.front();
    return a;
  }
  if(b.size()<=convolution_detail::naive_limit) return convolution_detail::naive(b,a,length);
  convolution_detail::transformed(a,b,length);
  return a;
}
