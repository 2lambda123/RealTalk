
#ifndef _REAL_TALK_VM_ARRAY_VALUE_H_
#define _REAL_TALK_VM_ARRAY_VALUE_H_

#include <cstddef>
#include <iostream>
#include <vector>

namespace real_talk {
namespace vm {

class DataStorage;
template<typename T> class ArrayValue;
template<typename T> bool operator==(
    const ArrayValue<T> &lhs, const ArrayValue<T> &rhs) noexcept;

template<typename T> class ArrayValue {
 public:
  static ArrayValue<T> Unidimensional(size_t size);
  static ArrayValue<T> Unidimensional(size_t values_count, DataStorage *values);
  static ArrayValue<T> Unidimensional(const std::vector<T> &values);
  static ArrayValue<T> Multidimensional(
      std::vector<size_t>::iterator dimensions_start,
      std::vector<size_t>::iterator dimensions_end);
  static ArrayValue<T> Multidimensional(
      uint8_t dimensions_count, size_t values_count, DataStorage *values);
  static ArrayValue<T> Multidimensional(std::vector< ArrayValue<T> > &&values);
  ArrayValue();
  ArrayValue(ArrayValue<T> &&value) noexcept;
  ArrayValue(const ArrayValue<T>&) = delete;
  ArrayValue<T> &operator=(const ArrayValue<T>&) = delete;
  void Set(ArrayValue<T> &&value, uint8_t dimensions_count) noexcept;
  ArrayValue<T> Clone() noexcept;
  void Destroy(uint8_t dimensions_count) noexcept;
  T &GetItem(size_t index) noexcept;
  const T &GetItem(size_t index) const noexcept;
  ArrayValue<T> &GetItemArray(size_t index) noexcept;
  const ArrayValue<T> &GetItemArray(size_t index) const noexcept;
  bool IsDeeplyEqual(
      const ArrayValue<T> &rhs, uint8_t dimensions_count) const noexcept;
  friend bool operator== <>(const ArrayValue<T> &lhs, const ArrayValue<T> &rhs)
      noexcept;
  std::ostream &Print(std::ostream &stream, uint8_t dimensions_count) const;

 private:
  class Storage;

  static Storage *CreateStorage(size_t items_count, size_t item_size);
  explicit ArrayValue(Storage *storage) noexcept;
  void DecRefsCount(uint8_t dimensions_count) noexcept;

  Storage *storage_;
};
}
}
#endif