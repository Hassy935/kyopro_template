#pragma once

#include <Library/data_structure/dual_segment_tree.hpp>
#include <algorithm>
#include <limits>
#include <optional>

namespace dual_segment_tree_preset_detail {

// 区間加算
template<class T> T add_mapping(T f,T x){ return x+f; }
template<class T> T add_composition(T f,T g){ return f+g; }
template<class T> T add_id(){ return T{}; }

// 区間代入（番兵値を使わない）
template<class T> T update_mapping(std::optional<T> f,T x){ return f.value_or(x); }
template<class T> std::optional<T> update_composition(std::optional<T> f,std::optional<T> g){
  return f ? f : g;
}
template<class T> std::optional<T> update_id(){ return std::nullopt; }

// 区間 chmin
template<class T> T chmin_mapping(T f,T x){ return std::min(f,x); }
template<class T> T chmin_composition(T f,T g){ return std::min(f,g); }
template<class T> T chmin_id(){
  static_assert(std::numeric_limits<T>::is_specialized);
  if constexpr(std::numeric_limits<T>::has_infinity) return std::numeric_limits<T>::infinity();
  else return std::numeric_limits<T>::max();
}

// 区間 chmax
template<class T> T chmax_mapping(T f,T x){ return std::max(f,x); }
template<class T> T chmax_composition(T f,T g){ return std::max(f,g); }
template<class T> T chmax_id(){
  static_assert(std::numeric_limits<T>::is_specialized);
  if constexpr(std::numeric_limits<T>::has_infinity) return -std::numeric_limits<T>::infinity();
  else return std::numeric_limits<T>::lowest();
}

} // namespace dual_segment_tree_preset_detail

template<class T=ll>
using DualSegmentTreeAdd = DualSegmentTree<T,T,
    dual_segment_tree_preset_detail::add_mapping<T>,
    dual_segment_tree_preset_detail::add_composition<T>,
    dual_segment_tree_preset_detail::add_id<T>>;

template<class T=ll>
using DualSegmentTreeUpdate = DualSegmentTree<T,std::optional<T>,
    dual_segment_tree_preset_detail::update_mapping<T>,
    dual_segment_tree_preset_detail::update_composition<T>,
    dual_segment_tree_preset_detail::update_id<T>>;

template<class T=ll>
using DualSegmentTreeChmin = DualSegmentTree<T,T,
    dual_segment_tree_preset_detail::chmin_mapping<T>,
    dual_segment_tree_preset_detail::chmin_composition<T>,
    dual_segment_tree_preset_detail::chmin_id<T>>;

template<class T=ll>
using DualSegmentTreeChmax = DualSegmentTree<T,T,
    dual_segment_tree_preset_detail::chmax_mapping<T>,
    dual_segment_tree_preset_detail::chmax_composition<T>,
    dual_segment_tree_preset_detail::chmax_id<T>>;

template<class T=ll> using dual_segtree_add = DualSegmentTreeAdd<T>;
template<class T=ll> using dual_segtree_update = DualSegmentTreeUpdate<T>;
template<class T=ll> using dual_segtree_chmin = DualSegmentTreeChmin<T>;
template<class T=ll> using dual_segtree_chmax = DualSegmentTreeChmax<T>;
