#pragma once
#include <Library/core/common_setting.hpp>

// IndexedSet（平方分割による順序統計付き集合）．
// Inspired by tatyam's SortedSet.
// https://github.com/tatyam-prime/SortedSet/blob/main/SortedSet.py
// https://qiita.com/tatyam/items/492c70ac4c955c055602

template<class T, class Compare, bool Multi>
class IndexedSetBase {
  static constexpr ll BUCKET_RATIO=4;
  static constexpr ll MIN_BASE=16;

  static constexpr ll initial_B(){
    ll b=1;
    while(b*b<MIN_BASE*BUCKET_RATIO) ++b;
    return b;
  }

  [[no_unique_address]] Compare cmp{};
  vvc<T> a;
  ll n=0,base=MIN_BASE,B=initial_B();

  // 比較順にソート済みの配列から構築
  void build(const vc<T>& v){
    n=std::ssize(v);
    base=std::max(MIN_BASE,n);
    B=std::ceil(std::sqrt(ld(base)*BUCKET_RATIO));
    vvc<T> nxt;
    if(n>0){
      ll k=(n+B-1)/B;
      nxt.reserve(k);
      rep(i,k){
        nxt.eb(v.begin()+n*i/k,v.begin()+n*(i+1)/k);
      }
    }
    a.swap(nxt);
  }

  void adjust(){
    if(n==0){
      clear();
      return;
    }
    if(n>2*base || (base>MIN_BASE && n<base/2)){
      vc<T> v;
      v.reserve(n);
      for(const auto& b:a) v.insert(v.end(),b.begin(),b.end());
      build(v);
    }
  }

  // 空集合では呼ばない
  ll locate(const T& x) const {
    auto it=std::lower_bound(nall(a),x,
      [&](const vc<T>& b,const T& value){
        return cmp(b.back(),value);
      });
    return std::min<ll>(it-a.begin(),std::ssize(a)-1);
  }

  void split(ll i){
    auto& b=a[i];
    if(std::ssize(b)<=2*B) return;
    ll m=std::ssize(b)/2;
    vc<T> r(b.begin()+m,b.end());
    b.erase(b.begin()+m,b.end());
    a.insert(a.begin()+i+1,std::move(r));
  }

  void repair(ll i){
    if(a[i].empty()){
      a.erase(a.begin()+i);
      return;
    }
    if(std::ssize(a)==1 || std::ssize(a[i])>=B/2) return;
    if(i==std::ssize(a)-1) i--;
    auto& l=a[i];
    auto& r=a[i+1];
    l.insert(l.end(),r.begin(),r.end());
    a.erase(a.begin()+i+1);
    split(i);
  }

  // lower/upper_bound の位置を {バケット番号，バケット内添字} で返す．
  pair<ll,ll> bound_pos(const T& x,bool upper) const {
    if(empty()) return {0,0};
    ll i;
    if(upper){
      // 同値な要素が複数バケットにまたがる場合も飛ばす．
      auto it=std::upper_bound(nall(a),x,
        [&](const T& value,const vc<T>& b){ return cmp(value,b.back()); });
      i=it-a.begin();
    }else i=locate(x);
    if(i==std::ssize(a)) return {i,0};
    const auto& b=a[i];
    auto it=upper
      ? std::upper_bound(nall(b),x,std::cref(cmp))
      : std::lower_bound(nall(b),x,std::cref(cmp));
    if(it==b.end()) return {i+1,0};
    return {i,it-b.begin()};
  }

  optional<T> next_value(const T& x,bool upper) const {
    auto [i,j]=bound_pos(x,upper);
    if(i==std::ssize(a)) return nullopt;
    return a[i][j];
  }

  optional<T> prev_value(const T& x,bool upper) const {
    auto [i,j]=bound_pos(x,upper);
    if(j>0) return a[i][j-1];
    if(i>0) return a[i-1].back();
    return nullopt;
  }

  // 添字からバケット内の位置を取得．
  pair<ll,ll> index_pos(ll k) const {
    assert(0<=k && k<n && "IndexedSetBase: index out of range");
    rep(i,std::ssize(a)){
      if(k<std::ssize(a[i])) return {i,k};
      k-=std::ssize(a[i]);
    }
    std::abort();
  }

  // 大量挿入後，1つのバケットを複数へ分割．
  void split_many(ll i){
    ll m=std::ssize(a[i]);
    if(m<=2*B) return;
    ll k=(m+B-1)/B;
    vvc<T> chunks;
    chunks.reserve(k);
    rep(j,k){
      chunks.eb(a[i].begin()+m*j/k,a[i].begin()+m*(j+1)/k);
    }
    a[i]=std::move(chunks[0]);
    a.insert(a.begin()+i+1,std::make_move_iterator(chunks.begin()+1),
             std::make_move_iterator(chunks.end()));
  }

  // [ (i,j), (r,t) ) を削除．終端は {a.size(),0} も可．
  void erase_span(ll i,ll j,ll r,ll t,ll k){
    n-=k;
    if(i==r){
      auto& b=a[i];
      b.erase(b.begin()+j,b.begin()+t);
    }else{
      auto& b=a[i];
      b.erase(b.begin()+j,b.end());
      if(r<std::ssize(a)){
        const auto& tail=a[r];
        b.insert(b.end(),tail.begin()+t,tail.end());
        r++;
      }
      a.erase(a.begin()+i+1,a.begin()+r);
    }
    if(std::ssize(a[i])>2*B) split(i);
    else repair(i);
    adjust();
  }

public:
  IndexedSetBase()=default;

  explicit IndexedSetBase(Compare comp):cmp(std::move(comp)) {}

  explicit IndexedSetBase(vc<T> v,Compare comp=Compare{})
    :cmp(std::move(comp)){
    if constexpr(Multi){
      std::stable_sort(nall(v),std::cref(cmp));
    }else{
      std::sort(nall(v),std::cref(cmp));
      v.erase(std::unique(nall(v),
        [&](const T& x,const T& y){ return !cmp(x,y) && !cmp(y,x); }),v.end());
    }
    build(v);
  }

  ll size() const { return n; }
  bool empty() const { return n==0; }

  void clear(){
    vvc<T>().swap(a);
    n=0;
    base=MIN_BASE;
    B=initial_B();
  }

  bool contains(const T& x) const {
    if(empty()) return false;
    const auto& b=a[locate(x)];
    return std::binary_search(nall(b),x,std::cref(cmp));
  }

  ll count(const T& x) const {
    if constexpr(!Multi){
      return contains(x);
    }else{
      auto [i,j]=bound_pos(x,false);
      if(i==std::ssize(a) || cmp(x,a[i][j])) return 0;

      const auto& b=a[i];
      // 同値要素が1バケット内で完結する場合
      if(cmp(x,b.back())){
        auto it=std::upper_bound(b.begin()+j,b.end(),x,std::cref(cmp));
        return it-(b.begin()+j);
      }

      // 同値要素が後続バケットにも存在し得る場合
      auto [r,t]=bound_pos(x,true);
      ll res=std::ssize(b)-j+t;
      forn(k,i+1,r) res+=std::ssize(a[k]);
      return res;
    }
  }

  pair<ll,ll> equal_range(const T& x) const {
    return {lower_bound(x),upper_bound(x)};
  }

  bool insert(const T& x){
    if constexpr(Multi){
      auto [i,j]=bound_pos(x,true);
      if(i==std::ssize(a)){
        if(empty()) a.eb();
        i=std::ssize(a)-1;
        j=std::ssize(a[i]);
      }
      a[i].insert(a[i].begin()+j,x);
      n++;
      split(i);
      adjust();
      return true;
    }
    if(empty()){
      a.pb({x});
      n++;
      return true;
    }
    ll i=locate(x);
    auto& b=a[i];
    auto it=std::lower_bound(nall(b),x,std::cref(cmp));
    if(it!=b.end() && !cmp(x,*it)) return false;
    b.insert(it,x);
    n++;
    split(i);
    adjust();
    return true;
  }

private:
  bool erase_one_impl(const T& x){
    if(empty()) return false;
    ll i=locate(x);
    auto& b=a[i];
    auto it=std::lower_bound(nall(b),x,std::cref(cmp));
    if(it==b.end() || cmp(x,*it)) return false;
    b.erase(it);
    n--;
    repair(i);
    adjust();
    return true;
  }

public:
  bool erase(const T& x) requires (!Multi) {
    return erase_one_impl(x);
  }

  bool erase_one(const T& x) requires Multi {
    return erase_one_impl(x);
  }

  // x を k 個追加する．実際の追加数を返す．
  ll insert(const T& x,ll k) requires Multi {
    assert(k>=0 && "IndexedMultiset::insert(): negative count");
    if(k==0) return 0;
    if(k==1){ insert(x); return 1; }
    auto [i,j]=bound_pos(x,true);
    if(i==std::ssize(a)){
      if(empty()) a.eb();
      i=std::ssize(a)-1;
      j=std::ssize(a[i]);
    }
    a[i].insert(a[i].begin()+j,k,x);
    n+=k;
    if(n>2*base) adjust();
    else split_many(i);
    return k;
  }

  // 比較順で先頭から最大 k 個の同値要素を削除．
  ll erase_many(const T& x,ll k) requires Multi {
    assert(k>=0 && "IndexedMultiset::erase_many(): negative count");
    if(k==0 || empty()) return 0;
    auto [i,j]=bound_pos(x,false);
    if(i==std::ssize(a) || cmp(x,a[i][j])) return 0;
    auto [r,t]=bound_pos(x,true);
    ll cnt=t-j;
    if(i!=r){
      cnt=std::ssize(a[i])-j+t;
      forn(b,i+1,r) cnt+=std::ssize(a[b]);
    }
    k=std::min(k,cnt);
    ll e=i,f=j,rest=k;
    while(rest>=std::ssize(a[e])-f){
      rest-=std::ssize(a[e])-f;
      e++; f=0;
      if(rest==0) break;
    }
    f+=rest;
    erase_span(i,j,e,f,k);
    return k;
  }

  ll erase_all(const T& x) requires Multi { return erase_many(x,n); }

  // 添字で指定した要素そのものを削除する．
  void erase_at(ll k){
    auto [i,j]=index_pos(k);
    a[i].erase(a[i].begin()+j);
    n--;
    repair(i);
    adjust();
  }

  T pop(ll k){
    auto [i,j]=index_pos(k);
    T x=a[i][j];
    a[i].erase(a[i].begin()+j);
    n--;
    repair(i);
    adjust();
    return x;
  }

  // 比較順で x より前にある要素数
  ll lower_bound(const T& x) const {
    ll res=0;
    for(const auto& b:a){
      if(!cmp(b.back(),x)){
        return res+(std::lower_bound(nall(b),x,std::cref(cmp))-b.begin());
      }
      res+=std::ssize(b);
    }
    return res;
  }

  // 比較順で x と同値または前にある要素数
  ll upper_bound(const T& x) const {
    ll res=0;
    for(const auto& b:a){
      if(cmp(x,b.back())){
        return res+(std::upper_bound(nall(b),x,std::cref(cmp))-b.begin());
      }
      res+=std::ssize(b);
    }
    return res;
  }

  // 比較順における半開区間 [l,r)
  ll count(const T& l,const T& r) const {
    assert(!cmp(r,l) && "IndexedSetBase::count(): invalid range");
    return lower_bound(r)-lower_bound(l);
  }

  // 比較順で k 番目の要素 (0-indexed)
  T operator[](ll k) const {
    assert(0<=k && k<n && "IndexedSetBase::operator[]: index out of range");
    auto [i,j]=index_pos(k);
    return a[i][j];
  }

  // x と同値な要素の添字 (0-indexed)
  ll operator()(const T& x) const {
    ll res=0;
    for(const auto& b:a){
      if(cmp(b.back(),x)){
        res+=std::ssize(b);
        continue;
      }
      auto it=std::lower_bound(nall(b),x,std::cref(cmp));
      assert(it!=b.end() && !cmp(x,*it) &&
             "IndexedSetBase::operator(): element not found");
      return res+(it-b.begin());
    }
    assert(false && "IndexedSetBase::operator(): element not found");
    std::abort();
  }

  T min() const {
    assert(!empty() && "IndexedSetBase::min(): empty set");
    return a.front().front();
  }

  T max() const {
    assert(!empty() && "IndexedSetBase::max(): empty set");
    return a.back().back();
  }

  // 以下の大小関係もすべて比較順に従う
  optional<T> lt(const T& x) const { return prev_value(x,false); }
  optional<T> le(const T& x) const { return prev_value(x,true); }
  optional<T> gt(const T& x) const { return next_value(x,true); }
  optional<T> ge(const T& x) const { return next_value(x,false); }

  T pop_min(){
    assert(!empty() && "IndexedSetBase::pop_min(): empty set");
    T x=a.front().front();
    a.front().erase(a.front().begin());
    n--;
    repair(0);
    adjust();
    return x;
  }

  T pop_max(){
    assert(!empty() && "IndexedSetBase::pop_max(): empty set");
    ll i=std::ssize(a)-1;
    T x=a[i].back();
    a[i].pop_back();
    n--;
    repair(i);
    adjust();
    return x;
  }

  // 双方向イテレータ（更新時は無効になる）
  class const_iterator {
    const IndexedSetBase* st=nullptr;
    ll i=0,j=0;

  public:
    using iterator_category=bidirectional_iterator_tag;
    using iterator_concept=bidirectional_iterator_tag;
    using value_type=T;
    using difference_type=std::ptrdiff_t;
    using reference=typename vc<T>::const_reference;
    using pointer=conditional_t<is_same_v<T,bool>,void,const T*>;

    const_iterator()=default;
    const_iterator(const IndexedSetBase* st,ll i,ll j):st(st),i(i),j(j) {}

    reference operator*() const { return st->a[i][j]; }
    pointer operator->() const requires (!is_same_v<T,bool>){
      return std::addressof(operator*());
    }

    const_iterator& operator++(){
      if(++j==std::ssize(st->a[i])){
        i++;
        j=0;
      }
      return *this;
    }

    const_iterator operator++(int){
      auto it=*this;
      ++*this;
      return it;
    }

    const_iterator& operator--(){
      if(j==0){
        i--;
        j=std::ssize(st->a[i]);
      }
      j--;
      return *this;
    }

    const_iterator operator--(int){
      auto it=*this;
      --*this;
      return it;
    }

    bool operator==(const const_iterator&) const=default;
  };

  using iterator=const_iterator;
  using const_reverse_iterator=std::reverse_iterator<const_iterator>;
  using reverse_iterator=const_reverse_iterator;

  const_iterator begin() const { return {this,0,0}; }
  const_iterator end() const { return {this,std::ssize(a),0}; }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
};

// IndexedSet は重複なし，IndexedMultiset は重複あり．
template<class T,class Compare=less<T>>
using IndexedSet=IndexedSetBase<T,Compare,false>;

template<class T,class Compare=less<T>>
using IndexedMultiset=IndexedSetBase<T,Compare,true>;

template<class T,class Compare=less<T>>
using idx_set=IndexedSet<T,Compare>;

template<class T,class Compare=less<T>>
using idx_multiset=IndexedMultiset<T,Compare>;

template<class T,class Compare=less<T>>
using indexed_set=IndexedSet<T,Compare>;

template<class T,class Compare=less<T>>
using indexed_multiset=IndexedMultiset<T,Compare>;
