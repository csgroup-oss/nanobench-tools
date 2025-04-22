// Random range objects.
//
// Code inspired by Chandler Carruth's presentation at CppCon17:
// "Going Nowhere Faster"
// Written from scratch by Luc Hermitte
// ======================================================================
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2025 CSGroup-FRANCE/SopraSteriaGroup, Luc Hermitte
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ======================================================================
#ifndef random_ranges_hpp
#define random_ranges_hpp

#include <random>
#include <cassert>
#include <limits>

template <typename T, bool is_integral>
struct uniform_distribution {};

template <typename T>
struct uniform_distribution<T, true> {
    using type = std::uniform_int_distribution<T>;
};

template <typename T>
struct uniform_distribution<T, false> {
    using type = std::uniform_real_distribution<T>;
};

template <typename T>
using uniform_distribution_t = typename uniform_distribution<T, std::is_integral<T>::value>::type;


template <typename T>
class RNG
{
public:
  explicit RNG(T min_, T max_, int count_)
    : m_device()
    , m_engine(m_device())
    , m_distribution(min_, max_)
    , m_count(count_)
    {}

  explicit RNG(int count)
      : RNG(std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), count)
      {}

  struct iterator {
    typedef std::forward_iterator_tag iterator_category;
    typedef std::ptrdiff_t            difference_type;
    typedef T                         value_type;
    typedef T*                        pointer;
    typedef T&                        reference;

    explicit iterator(RNG const& rng_, int count_) : rng(rng_), count(count_){}
    T operator*() const { return rng.generate(); }
    friend bool operator==(iterator const& lhs, iterator const& rhs) {
      assert(&lhs.rng == &rhs.rng);
      return lhs.count == rhs.count;
    }
    friend bool operator!=(iterator const& lhs, iterator const& rhs) {
      return !(lhs == rhs);
    }
    iterator& operator++()    { --count; return *this; }
    iterator  operator++(int) { iterator tmp; ++(*this); return tmp; }
  private:
    RNG const& rng;
    int        count;
  };

  iterator begin() const { return iterator{*this, m_count}; }
  iterator end  () const { return iterator{*this, 0};       }

  T generate() const { return m_distribution(m_engine); }

private:
  std::random_device               mutable m_device;
  std::default_random_engine       mutable m_engine;
  uniform_distribution_t<T>        mutable m_distribution;
  int                                      m_count;
};

#endif // random_ranges_hpp
