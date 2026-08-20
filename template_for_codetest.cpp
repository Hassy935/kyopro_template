#ifdef LOCAL
#define _GLIBCXX_DEBUG
#endif

#include <bits/stdc++.h>

using namespace std;

#define emp emplace

#define pb push_back
#define eb emplace_back

#define pf push_front
#define ef emplace_front

#define fi first
#define se second

#define rep(i, n) for (ll i = 0; i < (ll)(n); i++)
#define drep(i, n) for (ll i = (ll)(n) - 1; i >= 0; i--) //i=n-1, n-2, ... , 0 (n times)
#define forn(i, s, n) for (ll i = (s); i < (ll)(n); i++) //i=s, s+1, ... , n-1 (n-s times)
#define forc(i, s, n) for (ll i = (s); i <= (ll)(n); i++) //i=s, s+1, ... , n (n-s+1 times)
#define dforc(i, s, g) for (ll i = (s); i >= (ll)(g); i--) //i=s, s-1, ... , g (s-g+1 times)
#define forr(c, A) for (auto&& c: A)

#define nall(v) (v).begin(), (v).end()
#define rall(v) (v).rbegin(), (v).rend()

using ll=long long;
using ld=long double;
using i128=__int128;

using ull=unsigned long long;

using str=string;
using vstr=vector<string>;

using vl=vector<ll>;
using vvl=vector<vl>;
using vvvl=vector<vvl>;

using vb=vector<char>;
using vvb=vector<vb>;
using vvvb=vector<vvb>;

template<typename T> using vc=vector<T>;
template<typename T> using vvc=vc<vc<T>>;
template<typename T> using vvvc=vc<vvc<T>>;

template<class T>
auto make_vec(size_t n, const T& x){
  return vector<T>(n,x);
}

template<class T, class... Args>
requires (sizeof...(Args)>=1)
auto make_vec(size_t n, size_t m, Args... args){
  return vector(n,make_vec<T>(m,args...));
}

template<class T>
using uod_set = unordered_set<T>;

template<class K, class V>
using uod_map = unordered_map<K, V>;

template<class T>
using priority_queue_g=priority_queue<T,vc<T>,greater<T>>;

template<typename T> bool chmin(T& a, T b){if(a > b){a = b; return true;} return false;}
template<typename T> bool chmax(T& a, T b){if(a < b){a = b; return true;} return false;}

ll popcountll(ll a){
  assert(a>=0);
  return popcount((unsigned long long)a);
}

template<typename T>
vector<pair<T,ll>> rle(const vector<T>& A){
  ll N=ssize(A);
  vector<pair<T,ll>> ret;
  rep(i,N){
    if(i==0 || A[i]!=ret.back().first){
      ret.emplace_back(A[i],1);
    }else{
      ret.back().second++;
    }
  }
  return ret;
}

vector<pair<char,ll>> rle(const string& S){
  ll N=ssize(S);
  vector<pair<char,ll>> ret;
  rep(i,N){
    if(i==0 || S[i]!=ret.back().first){
      ret.emplace_back(S[i],1);
    }else{
      ret.back().second++;
    }
  }
  return ret;
}

template<class T>
struct compress {
  vector<T> xs;

  compress() {}

  explicit compress(const vector<T>& v) : xs(v) {
    build();
  }

  void add(const T& x) {
    xs.push_back(x);
  }

  void build() {
    sort(xs.begin(), xs.end());
    xs.erase(unique(xs.begin(), xs.end()), xs.end());
  }

  // x の圧縮後 index を返す
  // x が登録されていない場合は assert で落ちる
  ll get(const T& x) const {
    auto it = lower_bound(xs.begin(), xs.end(), x);
    assert(it != xs.end());
    assert(*it == x);
    return it - xs.begin();
  }

  // 圧縮後 index i の元の値を返す
  const T& operator[](ll i) const {
    assert(0 <= i && i < (ll)xs.size());
    return xs[i];
  }

  ll size() const {
    return xs.size();
  }

  bool empty() const {
    return xs.empty();
  }

  bool contains(const T& x) const {
    auto it = lower_bound(xs.begin(), xs.end(), x);
    return it != xs.end() && *it == x;
  }

  // x 以上の最小 index
  // x が xs の最大値より大きい場合は size() を返す
  ll lower_bound_index(const T& x) const {
    return lower_bound(xs.begin(), xs.end(), x) - xs.begin();
  }

  // x より大きい最小 index
  // x が xs の最大値以上の場合は size() を返す
  ll upper_bound_index(const T& x) const {
    return upper_bound(xs.begin(), xs.end(), x) - xs.begin();
  }
};

template<class T>
vector<T> sorted(vector<T> v) {
  sort(v.begin(), v.end());
  return v;
}

ll sumvl(const vl& A) {
  return accumulate(A.begin(), A.end(), 0LL);
}

ll maxvl(const vl& A) {
  assert(!A.empty());
  return *max_element(A.begin(), A.end());
}

ll minvl(const vl& A) {
  assert(!A.empty());
  return *min_element(A.begin(), A.end());
}

template<class T>
bool contains(const vector<T>& A, T c){
  return find(nall(A),c)!=A.end();
}

bool contains(const string& S, char c){
  return S.find(c)!=string::npos;
}

// vectorがsort済みであることが必要．
template<class T>
void unique_erase(vector<T>& A){
  A.erase(unique(nall(A)),A.end());
}


// 入出力関連．
template<class T>
istream& operator>>(istream& is, vector<T>& v) {
  for(auto& x : v) is >> x;
  return is;
}

template <class T, class U>
ostream& operator<<(ostream& os, const pair<T, U>& p) {
    return os << "(" << p.first << ", " << p.second << ")";
}

void yes(void){cout << "Yes\n";}
void no(void){cout << "No\n";}

void iofaster(void){
  ios_base::sync_with_stdio(false);
  cin.tie(nullptr);
  return;
}

void coutld(int n){
  assert(n>=0);
  cout << fixed << setprecision(n);
  cerr << fixed << setprecision(n);
  return;
}

void coutvl(const vl& A){
  ll N=ssize(A);
  rep(i,N) cout << A[i] << (i<N-1?' ':'\n');
  if(A.empty()) cout << '\n';
}

void debug_ll() { cerr << '\n'; }

template<class Head, class... Tail>
void debug_ll(Head h, Tail... t) {
    cerr << (ll)h;
    if constexpr (sizeof...(t)) cerr << ", ";
    debug_ll(t...);
}

#define dbg(...) cerr << "[" << #__VA_ARGS__ << "] = ", debug_ll(__VA_ARGS__)

void debug(const string& S){
  cerr << S << '\n';
}

template<class T>
void debug(const vector<T>& A){
  forr(x,A){
    cerr << x << ' ';
  }
  cerr << '\n';
}

void debug(const vb& A){
  forr(x,A){
    cerr << (int)x << ' ';
  }
  cerr << '\n';
}

void debug(const vector<string>& A){
  forr(s,A){
    cerr << s << '\n';
  }
}

template<class T>
void debug(const vector<vector<T>>& A){
  forr(row,A){
    debug(row);
  }
}

template<class T>
void debug(const deque<T>& A){
  forr(c,A){
    cerr << c << " ";
  }
  cerr << '\n';
}

template<class T>
void debug(const set<T>& st){
  forr(c, st){
    cerr << c << " ";
  }
  cerr << '\n';
}

template<class T>
void debug(const multiset<T>& st){
  forr(c, st){
    cerr << c << " ";
  }
  cerr << '\n';
}

template<class T>
void debug(const unordered_set<T>& st){
  forr(c, st){
    cerr << c << " ";
  }
  cerr << '\n';
}

template<class T,class U>
void debug(const map<T,U>& mp){
  for (auto [k,v] : mp){
    cerr << "(" << k << "," << v << ") ";
  }
  cerr << '\n';
}

template<class T,class U>
void debug(const unordered_map<T,U>& mp){
  for (auto [k,v] : mp){
    cerr << "(" << k << "," << v << ") ";
  }
  cerr << '\n';
}

// 数学系．

const ld PI = numbers::pi_v<ld>;

array<ll,18+1> tens={1,10,100,1000,10000,100000,1000000,10000000
  ,100000000,1000000000,10000000000,100000000000,1000000000000,10000000000000,100000000000000,1000000000000000,10000000000000000,
  100000000000000000,1000000000000000000
};

ll sq(ll x){
  return x*x;
}

ll divf(ll N, ll d) {
    assert(d != 0);
    if (((N < 0 && d > 0) || (N > 0 && d < 0)) && N % d != 0)
        return N / d - 1;
    return N / d;
}

ll divc(ll N, ll d) {
    assert(d != 0);
    if (((N > 0 && d > 0) || (N < 0 && d < 0)) && N % d != 0)
        return N / d + 1;
    return N / d;
}

ll mod_pos(ll N, ll d) {
  assert(d>0);

  ll r = N % d;
  if (r < 0) r += d;
  return r;
}

ll floor_sqrt(ll n){
  assert(n >= 0);
  ll x = sqrtl((long double)n);
  while ((i128)(x+1) * (x+1) <= n) x++;
  while ((i128)x * x > n) x--;
  return x;
}

ll ceil_sqrt(ll n){
  assert(n >= 0);
  ll x = floor_sqrt(n);
  if ((i128)x * x < n) x++;
  return x;
}

bool is_sq(ll n){
  ll x=floor_sqrt(n);
  return (i128)x*x==n;
}

ll floor_log2(ll x) {
  assert(x > 0);
  return std::bit_width((unsigned long long)x) - 1;
}

ll ceil_log2(ll x) {
  assert(x > 0);
  return std::bit_width((unsigned long long)(x - 1));
}

ll nC2(ll x){
  assert(0 <= x && x <= 4294967296LL);

  if(x % 2 == 0) return x / 2 * (x - 1);
  else return (x - 1) / 2 * x;
}

// 結果が空集合になるときに注意する．
pair<ll,ll> cap(ll l, ll r, ll nl, ll nr){
  return {max(l,nl),min(r,nr)};
}

// S：a進数表記の非負整数 (string)．
// a,b：基数 (2 <= a,b <= 10)．
// 戻り値：b進数表記 (string)．
// 計算量：O(|S|^2)．
string base_change(string S, int a, int b) {
    assert(2 <= a && a <= 10);
    assert(2 <= b && b <= 10);

    if (S == "0") return "0";

    string T;
    while (!(S.size() == 1 && S[0] == '0')) {
        string next;
        int rem = 0;

        for (char c : S) {
            rem = rem * a + (c - '0');
            int q = rem / b;
            rem %= b;
            if (!next.empty() || q != 0)
                next.push_back(char('0' + q));
        }

        T.push_back(char('0' + rem));
        if (next.empty()) next = "0";
        S = next;
    }

    reverse(T.begin(), T.end());
    return T;
}

const vector<ll> dx = {1, 0, -1, 0};
const vector<ll> dy = {0, 1, 0, -1};

// const string dict="DRUL";

//const vector<ll> dx = { 1, 1, 0, -1, -1, -1,  0,  1 };
//const vector<ll> dy = { 0, 1, 1,  1,  0, -1, -1, -1 };

pair<ll,ll> dir_to_pair(char c){
  assert(c=='U'||c=='D'||c=='L'||c=='R');

  if(c=='U') return {-1,0};
  if(c=='D') return {1,0};
  if(c=='L') return {0,-1};
  if(c=='R') return {0,1};

  assert(false);
  return {0,0};
}

// ---------- ここから --------------------

int main() {
  iofaster();
  
  

  return 0;
}