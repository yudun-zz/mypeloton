/*-------------------------------------------------------------------------
 *
 * value_vector.h
 * file description
 *
 * Copyright(c) 2015, CMU
 *
 * /n-store/src/common/value_vector.h
 *
 *-------------------------------------------------------------------------
 */

#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <array>
#include <iostream>
#include <sstream>

#include "common/types.h"
#include "common/value.h"

namespace nstore {

//===--------------------------------------------------------------------===//
// Value Vector
//===--------------------------------------------------------------------===//

/**
 * Fixed size Array of Values. Less flexible but faster than std::vector of Value.
 *
 * The constructor initializes all data with zeros, resulting in type==INVALID.
 * Destructors of the Value objects inside of this class will not be called.
 * Just the data area is revoked, which means Value should not rely on destructor.
 */

template <typename V> class GenericValueArray {
 public:

  inline GenericValueArray() : size(0),
  data_(reinterpret_cast<V*>(new char[sizeof(V) * size])) {
    ::memset(data_, 0, sizeof(V) * size);
  }

  inline GenericValueArray(int size) : size(size),
      data_(reinterpret_cast<V*>(new char[sizeof(V) * size])) {
    ::memset(data_, 0, sizeof(V) * size);
  }

  inline GenericValueArray(const GenericValueArray &rhs) : size(rhs.size),
      data_(reinterpret_cast<V*>(new char[sizeof(V) * size])) {
    ::memcpy(data_, rhs.data_, sizeof(V) * size);
  }

  inline ~GenericValueArray() {
    delete[] reinterpret_cast<char*>(data_);
  }

  inline void Reset(int _size) {
    delete[] reinterpret_cast<char*>(data_);
    size = _size;
    data_ = reinterpret_cast<V*>(new char[sizeof(V) * size]);
    ::memset(data_, 0, sizeof(V) * size);
  }

  GenericValueArray<V>& operator=(const GenericValueArray<V> &rhs);

  inline const V* getRawPointer() const {
    return data_;
  }

  inline V& operator[](int index) {
    assert (index >= 0);
    assert (index < size);
    return data_[index];
  }

  inline const V& operator[](int index) const {
    assert (index >= 0);
    assert (index < size);
    return data_[index];
  }

  inline int GetSize() const {
    return size;
  }

  inline int CompareValue (const GenericValueArray &rhs) const {
    assert (size == rhs.size);
    for (int i = 0; i < size; ++i) {
      int ret = data_[i].compare(rhs.data_[i]);
      if (ret != 0) return ret;
    }
    return 0;
  }

  // Get a string representation of this value vector
  friend std::ostream& operator<<(std::ostream& os, const GenericValueArray<V>& vector);

 private:
  int size;

  V* data_;
};

template <typename V> inline bool operator == (const GenericValueArray<V> &lhs, const GenericValueArray<V> &rhs) {
  return lhs.compareValue(rhs) == 0;
}

template <typename V> inline bool operator != (const GenericValueArray<V> &lhs, const GenericValueArray<V> &rhs) {
  return lhs.compareValue(rhs) != 0;
}

template <typename V> inline bool operator < (const GenericValueArray<V> &lhs, const GenericValueArray<V> &rhs) {
  return lhs.compareValue(rhs) < 0;
}

template <typename V> inline bool operator > (const GenericValueArray<V> &lhs, const GenericValueArray<V> &rhs) {
  return lhs.compareValue(rhs) > 0;
}

template <typename V> inline bool operator <= (const GenericValueArray<V> &lhs, const GenericValueArray<V> &rhs) {
  return !(lhs > rhs);
}

template <typename V> inline bool operator >= (const GenericValueArray<V> &lhs, const GenericValueArray<V> &rhs) {
  return !(lhs < rhs);
}

template<> inline GenericValueArray<Value>::~GenericValueArray() {
  delete[] reinterpret_cast<char*>(data_);
}

template<> inline GenericValueArray<Value>&
GenericValueArray<Value>::operator=(const GenericValueArray<Value> &rhs) {
  delete[] data_;
  size = rhs.size;
  data_ = reinterpret_cast<Value*>(new char[sizeof(Value) * size]);
  ::memcpy(data_, rhs.data_, sizeof(Value) * size);
  return *this;
}

template<Value>
std::ostream& operator<<(std::ostream& os, const GenericValueArray<Value>& vector) {

  os << "\t[ ";
  for (int i = 0; i < vector.size ; i++) {
    os << vector.data_[i] << " ";
  }
  os << " ]\n";

  return os;
}

typedef GenericValueArray<Value> ValueArray;

//===--------------------------------------------------------------------===//
// Compartors
//===--------------------------------------------------------------------===//

// Comparator for ValueArray.
class ValueArrayComparator {

 public:

  // copy constructor is needed as std::map takes instance of comparator, not a pointer.
  ValueArrayComparator(const ValueArrayComparator &rhs)
 : colCount_(rhs.colCount_), column_types_(new ValueType[colCount_]) {
    ::memcpy(column_types_, rhs.column_types_, sizeof(ValueType) * colCount_);
  }

  ValueArrayComparator(const std::vector<ValueType> &column_types)
  : colCount_(column_types.size()), column_types_(new ValueType[colCount_]) {
    for (int i = (int)colCount_ - 1; i >= 0; --i) {
      column_types_[i] = column_types[i];
    }
  }

  ValueArrayComparator(int colCount, const ValueType* column_types)
  : colCount_(colCount), column_types_(new ValueType[colCount_]) {
    ::memcpy(column_types_, column_types, sizeof(ValueType) * colCount_);
  }

  ~ValueArrayComparator() {
    delete[] column_types_;
  }

  inline bool operator()(const ValueArray& lhs, const ValueArray& rhs) const {
    assert (lhs.GetSize() == rhs.GetSize());
    return lhs.CompareValue(rhs) < 0;
  }

 private:

  size_t colCount_;
  ValueType* column_types_;
};

// Comparator for ValueArray.
template <std::size_t N>
class ValueArrayComparator2 {

 public:

  // copy constructor is needed as std::map takes instance of comparator, not a pointer.
  ValueArrayComparator2(const ValueArrayComparator2 &rhs)
 : column_count(rhs.column_count), column_types_(new ValueType[column_count]) {
    ::memcpy(column_types_, rhs.column_types_, sizeof(ValueType) * column_count);
  }

  ValueArrayComparator2(const std::vector<ValueType> &column_types)
  : column_count(column_types.size()), column_types_(new ValueType[column_count]) {
    for (int i = (int)column_count - 1; i >= 0; --i) {
      column_types_[i] = column_types[i];
    }
  }

  ValueArrayComparator2(int colCount, const ValueType* column_types)
  : column_count(colCount), column_types_(new ValueType[column_count]) {
    ::memcpy(column_types_, column_types, sizeof(ValueType) * column_count);
  }

  ~ValueArrayComparator2() {
    delete[] column_types_;
  }

  inline bool operator()(const std::array<Value, N> &lhs,
                         const std::array<Value, N> &rhs) const {
    int cmp = 0;
    for (int i = 0; i < N; ++i) {
      cmp = lhs[i].compareValue(rhs[i], column_types_[i]);
      if (cmp != 0) return (cmp < 0);
    }
    return (cmp < 0);
  }

 private:
  size_t column_count;

  ValueType* column_types_;
};

/// Comparator for ValueArray.

class ValueArrayEqualityTester {

 public:
  /// copy constructor is needed as std::map takes instance of comparator, not a pointer.

  ValueArrayEqualityTester(const ValueArrayEqualityTester &rhs)
 : column_count(rhs.column_count), column_types_(new ValueType[column_count]) {
    ::memcpy(column_types_, rhs.column_types_, sizeof(ValueType) * column_count);
  }

  ValueArrayEqualityTester(const std::vector<ValueType> &column_types)
  : column_count(column_types.size()), column_types_(new ValueType[column_count]) {
    for (int i = (int)column_count - 1; i >= 0; --i) {
      column_types_[i] = column_types[i];
    }
  }

  ValueArrayEqualityTester(int colCount, const ValueType* column_types)
  : column_count(colCount), column_types_(new ValueType[column_count]) {
    ::memcpy(column_types_, column_types, sizeof(ValueType) * column_count);
  }

  ~ValueArrayEqualityTester() {
    delete[] column_types_;
  }

  inline bool operator()(const ValueArray& lhs, const ValueArray& rhs) const {
    assert (lhs.size() == rhs.size());
    assert (lhs.size() == static_cast<int>(column_count));
    return lhs.CompareValue(rhs) == 0;
  }

 private:

  size_t column_count;

  ValueType* column_types_;
};

} // End nstore namespace
