#pragma once

#include<boost/unordered_set.hpp>
#include<boost/unordered_map.hpp>

template<class T>
using b_uod_set=boost::unordered_set<T>;

template<class T, class U>
using b_uod_map=boost::unordered_map<T,U>;