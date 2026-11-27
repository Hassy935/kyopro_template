#include <Library/io/io.hpp>

namespace {

void check(bool ok, const char* label){
  if(ok) return;
  cerr << "io: " << label << '\n';
  exit(1);
}

template<class T>
void expect_input(const char* label, const char* input, T value, const T& expected){
  istringstream in(input);
  check(&(in >> value)==&in, "stream reference");
  check(!in.fail() && value==expected, label);
  string extra;
  check(bool(in >> extra) && extra=="end", "remaining input");
}

void stream_failures(){
  deque<ll> short_input(3,99);
  istringstream missing("-2");
  missing >> short_input;
  check(missing.fail() && missing.eof() && !missing.bad(), "missing input state");
  check(short_input==deque<ll>{-2,99,99}, "missing input values and size");

  deque<ll> invalid_input(3,99);
  istringstream invalid("5 x 19");
  invalid >> invalid_input;
  check(invalid.fail() && !invalid.eof() && !invalid.bad(), "invalid input state");
  check(invalid_input.size()==3 && invalid_input.front()==5 &&
        invalid_input.back()==99, "invalid input values and size");
  invalid.clear();
  string token;
  ll next=0;
  check(bool(invalid >> token >> next) && token=="x" && next==19,
        "input remaining after clearing failbit");
}

} // namespace

int main(){
  expect_input("empty deque", "end", deque<ll>{}, deque<ll>{});
  expect_input("one integer", "-9 end", deque<ll>(1), deque<ll>{-9});
  expect_input("several integers", "3 -4 0 end", deque<ll>(3), deque<ll>{3,-4,0});
  expect_input("strings", "red blue end", deque<str>(2), deque<str>{"red","blue"});

  using P=pair<ll,ll>;
  expect_input("deque of pairs", "1 -2 3 4 end",
               deque<P>(2), deque<P>{{1,-2},{3,4}});
  expect_input("deque of vectors", "1 2 -3 end",
               deque<vl>{vl(2),{},vl(1)}, deque<vl>{{1,2},{},{-3}});
  expect_input("deque of deques", "1 2 -3 end",
               deque<deque<ll>>{deque<ll>(1),{},deque<ll>(2)},
               deque<deque<ll>>{{1},{},{2,-3}});
  expect_input("vector of deques", "1 2 -3 end",
               vc<deque<ll>>{deque<ll>(2),{},deque<ll>(1)},
               vc<deque<ll>>{{1,2},{},{-3}});
  expect_input("pair of deques", "1 -2 3 end",
               pair{deque<ll>(1),deque<ll>(2)}, pair{deque<ll>{1},deque<ll>{-2,3}});
  expect_input("vector of integers", "-1 2 end", vl(2), vl{-1,2});
  expect_input("vector of vectors", "2 -1 end", vvl{vl(1),vl(1)}, vvl{{2},{-1}});
  expect_input("pair of scalars", "-1 ok end", pair<ll,str>{}, pair<ll,str>{-1,"ok"});

  istringstream in("4 -5 text 7 8");
  deque<ll> integers(2);
  deque<str> words(1);
  pair<vl,ll> last{vl(1),0};
  check(bool(in >> integers >> words >> last), "chained input state");
  check(integers==deque<ll>{4,-5} && words==deque<str>{"text"} &&
        last==pair{vl{7},8LL}, "chained input values");
  stream_failures();
  cout << "io: all checks passed\n";
}
