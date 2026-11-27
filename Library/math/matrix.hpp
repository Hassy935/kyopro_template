#pragma once
#include <Library/core/common_setting.hpp>

template<class T>
class Matrix {
  static_assert(!std::is_same_v<T,bool>);

  ll h=0,w=0;
  std::vector<T> values;

  static size_t checked_size(ll height,ll width){
    assert(height>=0 && width>=0);
    if(height<0 || width<0) throw std::length_error("Matrix: negative size");
    const size_t limit=std::vector<T>{}.max_size();
    if(width!=0 && ull(height)>limit/ull(width)){
      assert(false && "Matrix: size exceeds vector::max_size()");
      throw std::length_error("Matrix: size exceeds vector::max_size()");
    }
    return size_t(ull(height)*ull(width));
  }

  struct FromRows {};

  template<class Rows>
  Matrix(FromRows,const Rows& rows)
      : h(std::ssize(rows)),w(h==0 ? 0 : std::ssize(*rows.begin())){
    const size_t count=checked_size(h,w);
    for(const auto& row:rows){
      assert(std::ssize(row)==w);
      if(std::ssize(row)!=w) throw std::invalid_argument("Matrix: uneven row lengths");
    }
    values.reserve(count);
    for(const auto& row:rows) values.insert(values.end(),row.begin(),row.end());
  }

public:
  Matrix()=default;
  Matrix(ll height,ll width): Matrix(height,width,T{}) {}
  Matrix(ll height,ll width,const T& value)
      : h(height),w(width),values(checked_size(height,width),value) {}
  Matrix(std::initializer_list<std::initializer_list<T>> rows): Matrix(FromRows{},rows) {}
  explicit Matrix(const std::vector<std::vector<T>>& rows): Matrix(FromRows{},rows) {}

  Matrix(const Matrix&)=default;
  Matrix(Matrix&& other) noexcept
      : h(std::exchange(other.h,0)),w(std::exchange(other.w,0)),values(std::move(other.values)){
    other.values.clear();
  }

  Matrix& operator=(const Matrix& other){
    if(this!=&other) *this=Matrix(other);
    return *this;
  }

  Matrix& operator=(Matrix&& other) noexcept {
    if(this!=&other){
      values=std::move(other.values);
      h=std::exchange(other.h,0);
      w=std::exchange(other.w,0);
      other.values.clear();
    }
    return *this;
  }

  ll height() const { return h; }
  ll width() const { return w; }

  std::span<T> operator[](ll i){
    assert(0<=i && i<h);
    if(w==0) return {};
    return {values.data()+size_t(i)*size_t(w),size_t(w)};
  }

  std::span<const T> operator[](ll i) const {
    assert(0<=i && i<h);
    if(w==0) return {};
    return {values.data()+size_t(i)*size_t(w),size_t(w)};
  }

  static Matrix identity(ll n){
    Matrix result(n,n);
    for(ll i=0;i<n;++i) result[i][i]=T(1);
    return result;
  }

  Matrix& operator+=(const Matrix& rhs){
    assert(h==rhs.h && w==rhs.w);
    for(size_t i=0;i<values.size();++i) values[i]+=rhs.values[i];
    return *this;
  }

  Matrix& operator-=(const Matrix& rhs){
    assert(h==rhs.h && w==rhs.w);
    for(size_t i=0;i<values.size();++i) values[i]-=rhs.values[i];
    return *this;
  }

  friend Matrix operator+(Matrix lhs,const Matrix& rhs){ return std::move(lhs+=rhs); }
  friend Matrix operator-(Matrix lhs,const Matrix& rhs){ return std::move(lhs-=rhs); }

  Matrix operator-() const {
    Matrix result=*this;
    for(auto& x:result.values) x=-x;
    return result;
  }

  Matrix& operator*=(T scalar){
    for(auto& x:values) x=x*scalar;
    return *this;
  }

  friend Matrix operator*(Matrix lhs,T scalar){
    lhs*=std::move(scalar);
    return lhs;
  }

  friend Matrix operator*(T scalar,Matrix rhs){
    for(auto& x:rhs.values) x=scalar*x;
    return rhs;
  }

  friend Matrix operator*(const Matrix& lhs,const Matrix& rhs){
    assert(lhs.w==rhs.h);
    Matrix result(lhs.h,rhs.w);
    if(lhs.w==0 || result.values.empty()) return result;
    for(ll i=0;i<lhs.h;++i){
      const auto row=lhs[i];
      auto out=result[i];
      for(ll k=0;k<lhs.w;++k){
        const auto right=rhs[k];
        const T& x=row[k];
        for(ll j=0;j<rhs.w;++j) out[j]+=x*right[j];
      }
    }
    return result;
  }

  Matrix& operator*=(const Matrix& rhs){ return *this=*this*rhs; }

  friend std::vector<T> operator*(const Matrix& lhs,const std::vector<T>& rhs){
    assert(lhs.w==std::ssize(rhs));
    std::vector<T> result(lhs.h);
    if(lhs.w==0) return result;
    for(ll i=0;i<lhs.h;++i){
      const auto row=lhs[i];
      for(ll j=0;j<lhs.w;++j) result[i]+=row[j]*rhs[j];
    }
    return result;
  }

  Matrix pow(ll n) const {
    assert(h==w && n>=0);
    if(n==0) return identity(h);
    if(n==1) return *this;
    Matrix base=*this;
    while(n>0 && !(n&1)){
      base*=base;
      n>>=1;
    }
    Matrix result=base;
    while((n>>=1)>0){
      base*=base;
      if(n&1) result*=base;
    }
    return result;
  }

  Matrix transpose() const {
    Matrix result(w,h);
    if(values.empty()) return result;
    for(ll i=0;i<h;++i)
      for(ll j=0;j<w;++j) result[j][i]=(*this)[i][j];
    return result;
  }

  friend bool operator==(const Matrix&,const Matrix&)=default;

  friend std::istream& operator>>(std::istream& is,Matrix& a){
    for(auto& x:a.values) if(!(is>>x)) break;
    return is;
  }

  friend std::ostream& operator<<(std::ostream& os,const Matrix& a){
    if(a.values.empty()) return os;
    for(ll i=0;i<a.h;++i){
      if(i!=0) os<<'\n';
      auto row=a[i];
      for(ll j=0;j<a.w;++j){
        if(j!=0) os<<' ';
        os<<row[j];
      }
    }
    return os;
  }

  std::vector<std::span<const T>> debug_view() const & {
    std::vector<std::span<const T>> rows;
    rows.reserve(h);
    for(ll i=0;i<h;++i) rows.push_back((*this)[i]);
    return rows;
  }
};
