#include <Library/template.hpp>

void test(){
  using RH=rolling_hash;
  mt19937_64 rng(20261126);
  static_assert(is_same_v<RH::hash_type,ull>);
  static_assert(is_same_v<RH,RollingHash>);
  RH empty;
  assert(empty.size()==0&&empty.empty());
  assert(empty.get_all()==RH("").get_all());
  assert(empty.lcp(0,0)==0);
  string bytes{"\0\xff\x80" "a",4};
  assert(RH(bytes).get_all()==RH(vl{0,255,128,97}).get_all());
  assert(RH(vector<bool>{true,false}).get_all()==RH(vl{1,0}).get_all());
  vl extreme{LLONG_MIN,LLONG_MAX,-1,0,1,998244353LL*1000000007LL};
  RH ex(extreme);
  for(ll i=0;i<ssize(extreme);i++){
    for(ll j=i+1;j<=ssize(extreme);j++){
      assert(ex.get(i,j)==RH(vl(extreme.begin()+i,extreme.begin()+j)).get_all());
    }
    for(ll j=i+1;j<ssize(extreme);j++) assert(!ex.equal(i,i+1,j,j+1));
  }
  RH small("abc"), large(string(20000,'x'));
  assert(small.get_all()==RH("abc").get_all());
  assert(RH::concat(small.get_all(),large.get_all(),large.size())==RH("abc"+string(20000,'x')).get_all());
  auto h=RH::concat(RH("a").get_all(),RH("").get_all(),0);
  assert(h==RH("a").get_all());
  // Exercise power-table growth from concat itself.
  auto zero=empty.get_all();
  assert(RH::concat(zero,zero,30000)==zero);
  RH copy=ex;
  RH moved=std::move(copy);
  assert(moved.get_all()==ex.get_all());
  copy=RH("abc");
  assert(copy.get_all()==small.get_all());

  ll cnt=0;
  // All pairs of binary strings up to length 5, all suffix starts.
  vector<string> all;
  for(int n=0;n<=5;n++) for(int mask=0;mask<(1<<n);mask++){
    string s(n,'a');
    for(int i=0;i<n;i++) if(mask>>i&1) s[i]='b';
    all.push_back(s);
  }
  vector<RH> hashes;
  for(auto& s:all) hashes.emplace_back(s);
  for(ll i=0;i<ssize(all);i++) for(ll j=0;j<ssize(all);j++){
    auto& a=all[i]; auto& b=all[j]; auto& ha=hashes[i]; auto& hb=hashes[j];
    for(ll l=0;l<=ssize(a);l++) for(ll k=0;k<=ssize(b);k++){
      ll ans=0;
      while(l+ans<ssize(a)&&k+ans<ssize(b)&&a[l+ans]==b[k+ans]) ans++;
      assert(ha.lcp(l,hb,k)==ans);
      assert(ha.equal(l,ssize(a),hb,k,ssize(b))==(a.substr(l)==b.substr(k)));
      cnt++;
    }
  }
  cout << "Exhaustive suffix pairs: " << cnt << '\n';
  for(int tc=0;tc<20000;tc++){
    ll n=rng()%65,m=rng()%65;
    vl a(n),b(m);
    auto val=[&]()->ll{
      switch(rng()%4){
        case 0:return ll(rng()%6)-3;
        case 1:return bit_cast<ll>(rng());
        case 2:return LLONG_MIN+ll(rng()%5);
        default:return LLONG_MAX-ll(rng()%5);
      }
    };
    for(auto& x:a) x=val();
    for(auto& x:b) x=val();
    if(tc%3==0) b=a;
    RH ha(a),hb(b);
    m=ssize(b);
    ll l=rng()%(n+1),r=rng()%(n+1),k=rng()%(m+1),t=rng()%(m+1);
    if(l>r) swap(l,r);
    if(k>t) swap(k,t);
    vl x(a.begin()+l,a.begin()+r),y(b.begin()+k,b.begin()+t);
    assert(ha.get(l,r)==RH(x).get_all());
    assert(hb.get(k,t)==RH(y).get_all());
    assert(ha.equal(l,r,hb,k,t)==(x==y));
    assert(ha.equal(l,r,l,r));
    ll ans=0;
    while(ans<ssize(x)&&ans<ssize(y)&&x[ans]==y[ans]) ans++;
    assert(ha.lcp(l,r,hb,k,t)==ans);
    ll same=0;
    while(l+same<r&&same<n&&a[l+same]==a[same]) same++;
    assert(ha.lcp(l,r,0,n)==same);
    assert(ha.lcp(l,l)==n-l);
    auto joined=x;
    joined.insert(joined.end(),y.begin(),y.end());
    assert(RH::concat(ha.get(l,r),hb.get(k,t),t-k)==RH(joined).get_all());
  }
  cout << "Random: 20000 cases passed\n";
}

int main(){
  cout << "Mersenne 61\n";
  test();
  // Check specialized reduction against 128-bit modulo.
  mt19937_64 rng(123);
  namespace P=rolling_hash_detail;
  for(int i=0;i<100000;i++){
    ull a=rng()%P::M,b=rng()%P::M,c=rng()%P::M;
    assert(P::mul_add(a,b,c)==ull((__uint128_t(a)*b+c)%P::M));
    ull x=rng();
    assert(P::encode(x)==x%P::M);
  }
  assert(P::mul_add(P::M-1,P::M-1,P::M-1)==0);
  cout << "All tests passed.\n";
}
