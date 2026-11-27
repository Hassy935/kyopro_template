#pragma once
#include <Library/core/common_setting.hpp>

namespace segment_tree_detail {

inline size_t checked_base(ll n,size_t limit){
  assert(n>=0);
  if(n<0) throw std::length_error("segment tree: negative size");
  // 上限以下の最大の2冪と比較してから切り上げる．
  if(limit==0 || ull(n)>std::bit_floor(limit)){
    assert(false && "segment tree: size exceeds vector::max_size()");
    throw std::length_error("segment tree: size exceeds vector::max_size()");
  }
  return n==0 ? 1 : std::bit_ceil(size_t(n));
}

// 境界から順に節点を消費し，最初に不成立となった節点だけを降りる．
template<class S,auto op,auto e,class G,class Read,class Push>
ll max_right(size_t k,size_t base,ll n,G& pred,Read read,Push push){
  S acc=e();
  do {
    while(!(k&1)) k>>=1;
    S joined=op(acc,read(k));
    if(!pred(joined)){
      while(k<base){
        push(k);
        k*=2;
        S left=op(acc,read(k));
        if(pred(left)){
          acc=std::move(left);
          ++k;
        }
      }
      return ll(k-base);
    }
    acc=std::move(joined);
    ++k;
  } while(!std::has_single_bit(k));
  return n;
}

template<class S,auto op,auto e,class G,class Read,class Push>
ll min_left(size_t k,size_t base,G& pred,Read read,Push push){
  S acc=e();
  do {
    --k;
    while(k>1 && (k&1)) k>>=1;
    S joined=op(read(k),acc);
    if(!pred(joined)){
      while(k<base){
        push(k);
        k=2*k+1;
        S right=op(read(k),acc);
        if(pred(right)){
          acc=std::move(right);
          --k;
        }
      }
      return ll(k+1-base);
    }
    acc=std::move(joined);
  } while(!std::has_single_bit(k));
  return 0;
}

} // namespace segment_tree_detail

template<class S,auto op,auto e>
class SegmentTree {
  ll n_;
  size_t base_;
  std::vector<S> data_;

  decltype(auto) read(size_t k) const { return data_[k]; }
  void update(size_t k){ data_[k]=op(read(2*k),read(2*k+1)); }

public:
  SegmentTree(): SegmentTree(0) {}
  explicit SegmentTree(ll n)
      : n_(n),base_(segment_tree_detail::checked_base(n,std::vector<S>{}.max_size()/2)),
        data_(2*base_,e()) {}

  explicit SegmentTree(const std::vector<S>& v): SegmentTree(std::ssize(v)){
    for(size_t i=0;i<v.size();++i) data_[base_+i]=v[i];
    for(size_t k=base_-1;k>0;--k) update(k);
  }

  SegmentTree(const SegmentTree&)=default;
  SegmentTree(SegmentTree&& other) noexcept
      : n_(std::exchange(other.n_,0)),base_(std::exchange(other.base_,1)),
        data_(std::move(other.data_)) {}

  SegmentTree& operator=(SegmentTree other) noexcept {
    std::swap(n_,other.n_);
    std::swap(base_,other.base_);
    data_.swap(other.data_);
    return *this;
  }

  ll size() const { return n_; }
  bool empty() const { return n_==0; }

  void set(ll p,S x){
    assert(0<=p && p<n_);
    size_t k=base_+p;
    data_[k]=std::move(x);
    while((k>>=1)!=0) update(k);
  }

  S get(ll p) const {
    assert(0<=p && p<n_);
    return data_[base_+p];
  }
  S operator[](ll p) const { return get(p); }

  S prod(ll l,ll r) const {
    assert(0<=l && l<=r && r<=n_);
    S left=e(),right=e();
    for(size_t a=base_+l,b=base_+r;a<b;a>>=1,b>>=1){
      if(a&1) left=op(left,read(a++));
      if(b&1) right=op(read(--b),right);
    }
    return op(left,right);
  }

  S all_prod() const { return empty() ? e() : S(data_[1]); }

  template<auto g>
  ll max_right(ll l) const { return max_right(l,g); }

  template<class G>
  ll max_right(ll l,G g) const {
    assert(0<=l && l<=n_ && g(e()));
    if(l==n_) return n_;
    return segment_tree_detail::max_right<S,op,e>(base_+l,base_,n_,g,
        [this](size_t k) -> decltype(auto) { return read(k); },[](size_t){});
  }

  template<auto g>
  ll min_left(ll r) const { return min_left(r,g); }

  template<class G>
  ll min_left(ll r,G g) const {
    assert(0<=r && r<=n_ && g(e()));
    if(r==0) return 0;
    return segment_tree_detail::min_left<S,op,e>(base_+r,base_,g,
        [this](size_t k) -> decltype(auto) { return read(k); },[](size_t){});
  }

  std::vector<S> to_vector() const {
    if(empty()) return {};
    const auto first=data_.cbegin()+base_;
    return std::vector<S>(first,first+n_);
  }

  auto debug_view() const {
    if constexpr(std::is_same_v<S,bool>){
      return to_vector();
    }else{
      if(empty()) return std::span<const S>{};
      return std::span<const S>(data_.data()+base_,size_t(n_));
    }
  }
};

template<class S, auto op, auto e>
using segtree = SegmentTree<S,op,e>;
