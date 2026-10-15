#pragma once

struct point2d{
  ll x, y;

  point2d() : x(0), y(0) {}

  point2d(ll x_, ll y_) : x(x_), y(y_) {}

  point2d operator+(const point2d& other) const {
    return point2d(x + other.x, y + other.y);
  }

  point2d& operator+=(const point2d& other) {
    x+=other.x;
    y+=other.y;
    return *this;
  }

  point2d operator-(const point2d& other) const {
    return point2d(x - other.x, y - other.y);
  }

  point2d& operator-=(const point2d& other) {
    x-=other.x;
    y-=other.y;
    return *this;
  }

  point2d operator*(ll c) const {
    return point2d(x * c, y * c);
  }

  point2d& operator*=(ll c) {
    x*=c;
    y*=c;
    return *this;
  }

  bool operator<(const point2d& other) const {
    return tie(x, y) < tie(other.x, other.y);
  }

  bool operator==(const point2d& other) const {
    return x == other.x && y == other.y;
  }
};

istream& operator>>(istream& is, point2d& p){
  return is >> p.x >> p.y;
}

ostream& operator<<(ostream& os, const point2d& p){
  return os << p.x << " " << p.y;
}

point2d operator*(ll c, const point2d& p){
  return p * c;
}

ll dot(const point2d& p, const point2d& q){
  return p.x * q.x + p.y * q.y;
}

ll cross(const point2d& p, const point2d& q){
  return p.x * q.y - p.y * q.x;
}

ll norm2(const point2d& p){
  return dot(p, p);
}

ll dist2(const point2d& p, const point2d& q){
  return norm2(p - q);
}

// a -> b -> c が反時計回りに曲がるならtrue．
// 一直線上ならfalse．
bool ccw(const point2d& a, const point2d& b, const point2d& c){
  return cross(b - a, c - a) > 0;
}

// 偏角ソート用：正のx軸から反時計回りに [0, 2π) の順
// (1,0) -> (0,1) -> (-1,0) -> (0,-1)
int half_plane(const point2d& p) {
  if (p.y > 0) return 0;
  if (p.y == 0 && p.x >= 0) return 0;
  return 1;
}

bool arg_less(const point2d& a, const point2d& b) {
  bool za = (a.x == 0 && a.y == 0);
  bool zb = (b.x == 0 && b.y == 0);

  // 原点ベクトルは偏角が定義できないので，ここでは先頭に置く
  if (za || zb) return za && !zb;

  int ha = half_plane(a);
  int hb = half_plane(b);

  if (ha != hb) return ha < hb;

  ll cr = cross(a, b);

  // 同じ半平面内では，外積が正なら a の方が反時計回りで手前
  if (cr != 0) return cr > 0;

  // 同じ方向のベクトルは原点から近い順
  return norm2(a) < norm2(b);
}