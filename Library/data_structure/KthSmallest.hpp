#pragma once

template<class T, ll k>
struct kth_smallest{
  // kは1-indexed．
  // lower：小さい方からk個を入れる．
  // upper：それ以外を入れる．
  multiset<T> lower,upper;

  kthsmallest() : lower{}, upper{} {
    static_assert(k>=1);
  }

  ll size() const {
    return ssize(lower)+ssize(upper);
  }

  void rebalance(){
    while(ssize(lower)>k){
      auto it=prev(lower.end());
      upper.insert(*it);
      lower.erase(it);
    }
    while(ssize(upper)>max(0LL,size()-k)){
      auto it=upper.begin();
      lower.insert(*it);
      upper.erase(it);
    }
    return;
  }

  void insert(const T& x){
    if(lower.empty()){
      lower.insert(x);
      return;
    }

    const T& t=*lower.rbegin();
    if(x<=t) lower.insert(x);
    else upper.insert(x);

    rebalance();
    return;
  }

  void erase(const T& x){
    if(lower.contains(x)){
      auto it=lower.find(x);
      lower.erase(it);
    }else if(upper.contains(x)){
      auto it=upper.find(x);
      upper.erase(it);
    }

    rebalance();
    return;
  }

  const T& kth() const {
    assert(ssize(lower)>=k);

    return *lower.rbegin();
  }
};