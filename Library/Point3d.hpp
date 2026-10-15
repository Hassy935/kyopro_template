#pragma once

struct point3d{
  ll x, y, z;

  point3d() : x(0), y(0), z(0) {}

  point3d(ll x_, ll y_, ll z_) : x(x_), y(y_), z(z_) {}

  point3d operator+(const point3d& other) const {
    return point3d(x + other.x, y + other.y, z + other.z);
  }

  point3d operator-(const point3d& other) const {
    return point3d(x - other.x, y - other.y, z - other.z);
  }

  point3d operator*(ll c) const {
    return point3d(x * c, y * c, z * c);
  }

  bool operator<(const point3d& other) const {
    return tie(x, y, z) < tie(other.x, other.y, other.z);
  }

  bool operator==(const point3d& other) const {
    return x == other.x && y == other.y && z == other.z;
  }
};

istream& operator>>(istream& is, point3d& p){
  return is >> p.x >> p.y >> p.z;
}

ostream& operator<<(ostream& os, const point3d& p){
  return os << p.x << " " << p.y << " " << p.z;
}

point3d operator*(ll c, const point3d& p){
  return p * c;
}

ll dot(const point3d& p, const point3d& q){
  return p.x * q.x + p.y * q.y + p.z * q.z;
}

ll norm2(const point3d& p){
  return dot(p, p);
}

ll dist2(const point3d& p, const point3d& q){
  return norm2(p - q);
}