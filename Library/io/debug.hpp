#pragma once
#include <Library/core/common_setting.hpp>
#include <Library/io/io.hpp>

#ifdef LOCAL

namespace debug_detail {

enum class Format { compact, sequence, rows, bytes, mapping, top_adapter, front_adapter };

template<class T> inline constexpr bool vector_type=false;
template<class T, class A> inline constexpr bool vector_type<vector<T,A>> = true;

template<class T> inline constexpr bool array_type=false;
template<class T, size_t N> inline constexpr bool array_type<array<T,N>> = true;

template<class T> inline constexpr bool tuple_type=false;
template<class T, class U> inline constexpr bool tuple_type<pair<T,U>> = true;
template<class... Ts> inline constexpr bool tuple_type<tuple<Ts...>> = true;

template<class T> inline constexpr bool optional_type=false;
template<class T> inline constexpr bool optional_type<optional<T>> = true;

template<class T>
constexpr Format debug_format(type_identity<T>){ return Format::compact; }

template<class T, class A>
constexpr Format debug_format(type_identity<vector<T,A>>){
  if constexpr(vector_type<T> || is_same_v<T,string>) return Format::rows;
  if constexpr(is_same_v<T,char>) return Format::bytes;
  return Format::sequence;
}

template<class... Ts>
constexpr Format debug_format(type_identity<deque<Ts...>>){ return Format::sequence; }

template<class T, class C>
constexpr Format debug_format(type_identity<stack<T,C>>){ return Format::top_adapter; }

template<class T, class C>
constexpr Format debug_format(type_identity<queue<T,C>>){ return Format::front_adapter; }

template<class T, class C, class Compare>
constexpr Format debug_format(type_identity<priority_queue<T,C,Compare>>){ return Format::top_adapter; }

template<class... Ts>
constexpr Format debug_format(type_identity<set<Ts...>>){ return Format::sequence; }

template<class... Ts>
constexpr Format debug_format(type_identity<multiset<Ts...>>){ return Format::sequence; }

template<class... Ts>
constexpr Format debug_format(type_identity<unordered_set<Ts...>>){ return Format::sequence; }

template<class... Ts>
constexpr Format debug_format(type_identity<unordered_multiset<Ts...>>){ return Format::sequence; }

template<class... Ts>
constexpr Format debug_format(type_identity<map<Ts...>>){ return Format::mapping; }

template<class... Ts>
constexpr Format debug_format(type_identity<unordered_map<Ts...>>){ return Format::mapping; }

inline void write_unsigned(__uint128_t x){
  char digits[39];
  char* end=digits+sizeof(digits);
  char* p=end;
  do {
    *--p=char('0'+x%10);
    x/=10;
  } while(x!=0);
  cerr.write(p,end-p);
}

template<bool Lines=false, class T>
void write(T&& x){
  using U=remove_cvref_t<T>;
  constexpr auto format=debug_detail::debug_format(type_identity<U>{});

  if constexpr(requires(const U& value){ value.debug_view(); }){
    ::debug_detail::write<Lines>(std::as_const(x).debug_view());
  }else if constexpr(format==Format::top_adapter || format==Format::front_adapter){
    U copy(std::as_const(x));
    if constexpr(!Lines) cerr << '[';
    bool first=true;
    while(!copy.empty()){
      if constexpr(!Lines){
        if(!exchange(first,false)) cerr << ", ";
      }
      if constexpr(format==Format::front_adapter)
        ::debug_detail::write(std::as_const(copy).front());
      else
        ::debug_detail::write(std::as_const(copy).top());
      if constexpr(Lines) cerr << ' ';
      copy.pop();
    }
    if constexpr(Lines) cerr << '\n';
    else cerr << ']';
  }else if constexpr(Lines && format==Format::rows){
    for(auto&& item:std::as_const(x)) ::debug_detail::write<true>(item);
  }else if constexpr(Lines && format==Format::mapping){
    for(const auto& [key,value]:std::as_const(x)){
      cerr << '(';
      ::debug_detail::write(key);
      cerr << ',';
      ::debug_detail::write(value);
      cerr << ") ";
    }
    cerr << '\n';
  }else if constexpr(Lines && (format==Format::sequence || format==Format::bytes)){
    for(auto&& item:std::as_const(x)){
      if constexpr(format==Format::bytes) ::debug_detail::write(int(item));
      else ::debug_detail::write(item);
      cerr << ' ';
    }
    cerr << '\n';
  }else{
    if constexpr(is_array_v<U> && is_same_v<remove_extent_t<U>,char>){
      size_t length=0;
      while(length<extent_v<U> && x[length]!='\0') ++length;
      cerr << string_view(x,length);
    }else if constexpr(is_same_v<U,i128>){
      auto magnitude=static_cast<__uint128_t>(x);
      if(x<0){
        cerr << '-';
        magnitude=__uint128_t(0)-magnitude;
      }
      write_unsigned(magnitude);
    }else if constexpr(is_same_v<U,__uint128_t>){
      write_unsigned(x);
    }else if constexpr(tuple_type<U>){
      cerr << '(';
      bool first=true;
      std::apply([&](auto&&... items){
        auto emit=[&](auto&& item){
          if(!exchange(first,false)) cerr << ", ";
          ::debug_detail::write(std::forward<decltype(item)>(item));
        };
        (emit(std::forward<decltype(items)>(items)),...);
      },std::forward<T>(x));
      cerr << ')';
    }else if constexpr(optional_type<U>){
      if(x) ::debug_detail::write(*std::forward<T>(x));
      else cerr << "nullopt";
    }else if constexpr(!is_array_v<U> && !array_type<U> &&
                       requires { cerr << std::forward<T>(x); }){
      cerr << std::forward<T>(x);
    }else if constexpr(std::ranges::range<T>){
      cerr << '[';
      bool first=true;
      auto last=std::ranges::end(x);
      for(auto it=std::ranges::begin(x);it!=last;++it){
        if(!exchange(first,false)) cerr << ", ";
        if constexpr(format==Format::bytes) ::debug_detail::write(int(*it));
        else ::debug_detail::write(*it);
      }
      cerr << ']';
    }else if constexpr(requires(const U& value){
      { value.val() } -> integral;
      { U::mod() } -> integral;
    }){
      ::debug_detail::write(+std::as_const(x).val());
    }else{
      static_assert(false,"debug: unsupported type");
    }
    if constexpr(Lines) cerr << '\n';
  }
}

} // namespace debug_detail

inline void debug_out(){
  cerr << '\n';
}

template<class Head, class... Tail>
void debug_out(Head&& head,Tail&&... tail){
  debug_detail::write(std::forward<Head>(head));
  ((cerr << ", ", debug_detail::write(std::forward<Tail>(tail))),...);
  cerr << '\n';
}

inline void debug(const string& value){
  debug_detail::write<true>(value);
}

template<class T>
void debug(T&& value){
  debug_detail::write<true>(std::forward<T>(value));
}

#define dbg(...) (::std::cerr << "[" << #__VA_ARGS__ << "] = ", ::debug_out(__VA_ARGS__))

#else

#define dbg(...) ((void)0)
#define debug(...) ((void)0)

#endif
