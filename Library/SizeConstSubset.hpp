#pragma once

// limit個の要素からちょうどsz個を選ぶ部分集合を列挙する．
// 各部分集合をビットマスクで表し，昇順に返す．
// O(binom(limit,sz)) 時間・空間．
vector<ll> size_subset(ll sz, ll limit){
  assert(sz>=0);
  assert(sz<=limit);
  assert(limit<=62);

  if(sz==0) return {0};

  vector<ll> res;
  ll mask=(1LL<<sz)-1, upper=1LL<<limit;
  while(mask<upper){
    res.pb(mask);

    ll lowest_bit=mask&(-mask),carried=mask+lowest_bit;
    mask=(((mask&~carried)/lowest_bit)>>1)|carried;
  }

  return res;
}