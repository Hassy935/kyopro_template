#pragma once
#include <Library/data_structure/segment_tree.hpp>

// mapping(f,x) は f(x)．composition(f,g) は先に g，後に f を適用する．
template<class S,auto op,auto e,class F,auto mapping,auto composition,auto id>
class LazySegmentTree {
  ll n_;
  size_t base_;
  int height_;
  std::vector<S> data_;
  std::vector<F> lazy_;

  decltype(auto) read(size_t k) const { return data_[k]; }
  void update(size_t k){ data_[k]=op(read(2*k),read(2*k+1)); }

  void all_apply(size_t k,const F& f){
    data_[k]=mapping(f,read(k));
    if(k<base_) lazy_[k]=composition(f,F(lazy_[k]));
  }

  void push(size_t k){
    const F f=lazy_[k];
    all_apply(2*k,f);
    all_apply(2*k+1,f);
    lazy_[k]=id();
  }

  void push_path(size_t k){
    for(int h=height_;h>0;--h) push(k>>h);
  }

  // 端が節点の境界とずれる階層だけを伝播・再計算する．
  template<bool rebuild>
  void boundaries(size_t a,size_t b,int h){
    if(((a>>h)<<h)!=a){
      if constexpr(rebuild) update(a>>h);
      else push(a>>h);
    }
    if(((b>>h)<<h)!=b){
      if constexpr(rebuild) update((b-1)>>h);
      else push((b-1)>>h);
    }
  }

  // carry は祖先由来で，この節点に残る作用より新しい．
  void collect(size_t k,size_t first,size_t width,const F& carry,std::vector<S>& out) const {
    if(k>=base_){
      out.push_back(mapping(carry,read(k)));
      return;
    }
    const F next=composition(carry,F(lazy_[k]));
    width/=2;
    collect(2*k,first,width,next,out);
    if(first+width<size_t(n_)) collect(2*k+1,first+width,width,next,out);
  }

public:
  LazySegmentTree(): LazySegmentTree(0) {}
  explicit LazySegmentTree(ll n)
      : n_(n),base_(segment_tree_detail::checked_base(n,
            std::min(std::vector<S>{}.max_size()/2,std::vector<F>{}.max_size()))),
        height_(std::countr_zero(base_)),data_(2*base_,e()),lazy_(base_,id()) {}

  explicit LazySegmentTree(const std::vector<S>& v): LazySegmentTree(std::ssize(v)){
    for(size_t i=0;i<v.size();++i) data_[base_+i]=v[i];
    for(size_t k=base_-1;k>0;--k) update(k);
  }

  LazySegmentTree(const LazySegmentTree&)=default;
  LazySegmentTree(LazySegmentTree&& other) noexcept
      : n_(std::exchange(other.n_,0)),base_(std::exchange(other.base_,1)),
        height_(std::exchange(other.height_,0)),data_(std::move(other.data_)),
        lazy_(std::move(other.lazy_)) {}

  LazySegmentTree& operator=(LazySegmentTree other) noexcept {
    std::swap(n_,other.n_);
    std::swap(base_,other.base_);
    std::swap(height_,other.height_);
    data_.swap(other.data_);
    lazy_.swap(other.lazy_);
    return *this;
  }

  ll size() const { return n_; }
  bool empty() const { return n_==0; }

  void set(ll p,S x){
    assert(0<=p && p<n_);
    size_t k=base_+p;
    push_path(k);
    data_[k]=std::move(x);
    while((k>>=1)!=0) update(k);
  }

  S get(ll p){
    assert(0<=p && p<n_);
    size_t k=base_+p;
    push_path(k);
    return data_[k];
  }

  S operator[](ll p){ return get(p); }

  S prod(ll l,ll r){
    assert(0<=l && l<=r && r<=n_);
    if(l==r) return e();
    size_t a=base_+l,b=base_+r;
    for(int h=height_;h>0;--h) boundaries<false>(a,b,h);
    S left=e(),right=e();
    for(;a<b;a>>=1,b>>=1){
      if(a&1) left=op(left,read(a++));
      if(b&1) right=op(read(--b),right);
    }
    return op(left,right);
  }

  S all_prod() const { return empty() ? e() : S(data_[1]); }

  void apply(ll p,F f){
    assert(0<=p && p<n_);
    size_t k=base_+p;
    push_path(k);
    all_apply(k,f);
    while((k>>=1)!=0) update(k);
  }

  void apply(ll l,ll r,F f){
    assert(0<=l && l<=r && r<=n_);
    if(l==r) return;
    const size_t first=base_+l,last=base_+r;
    for(int h=height_;h>0;--h) boundaries<false>(first,last,h);
    for(size_t a=first,b=last;a<b;a>>=1,b>>=1){
      if(a&1) all_apply(a++,f);
      if(b&1) all_apply(--b,f);
    }
    for(int h=1;h<=height_;++h) boundaries<true>(first,last,h);
  }

  template<auto g>
  ll max_right(ll l){ return max_right(l,g); }

  template<class G>
  ll max_right(ll l,G g){
    assert(0<=l && l<=n_ && g(e()));
    if(l==n_) return n_;
    push_path(base_+l);
    return segment_tree_detail::max_right<S,op,e>(base_+l,base_,n_,g,
        [this](size_t k) -> decltype(auto) { return read(k); },[this](size_t k){ push(k); });
  }

  template<auto g>
  ll min_left(ll r){ return min_left(r,g); }

  template<class G>
  ll min_left(ll r,G g){
    assert(0<=r && r<=n_ && g(e()));
    if(r==0) return 0;
    push_path(base_+r-1);
    return segment_tree_detail::min_left<S,op,e>(base_+r,base_,g,
        [this](size_t k) -> decltype(auto) { return read(k); },[this](size_t k){ push(k); });
  }

  std::vector<S> to_vector() const {
    if(empty()) return {};
    std::vector<S> out;
    out.reserve(n_);
    collect(1,0,base_,id(),out);
    return out;
  }

  std::vector<S> debug_view() const { return to_vector(); }
};

template<
  class S, auto op, auto e,
  class F, auto mapping, auto composition, auto id
>
using lazy_segtree =
  LazySegmentTree<S,op,e,F,mapping,composition,id>;
