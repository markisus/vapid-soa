#ifndef _BASICRELATION_H_
#define _BASICRELATION_H_

#include <vector>
#include <tuple>
#include <iostream>
#include <functional>
#include <random>
#include <ostream>

#include "GenScatterHierarchy.h"

namespace vapid {

template <typename Idx, typename T>
struct TupleDumper;

template <uint16_t idx, typename... Ts>
struct TupleDumper<std::integral_constant<uint16_t, idx>, std::tuple<Ts...>> {
  static void dump(std::basic_ostream<char>& ss, const std::tuple<Ts...>& t) {
      ss << std::get<idx>(t) << ", ";
      TupleDumper<std::integral_constant<uint16_t,idx+1>, std::tuple<Ts...>>::dump(ss, t);
    }
};

template <typename... Ts>
struct TupleDumper<std::integral_constant<uint16_t, std::tuple_size<std::tuple<Ts...>>::value - 1>, std::tuple<Ts...>> {
  static void dump(std::basic_ostream<char>& ss, const std::tuple<Ts...>& t) {
      ss << std::get< std::tuple_size<std::tuple<Ts...>>::value -1>(t);
    }
};

template <typename T>
void dump_tuple(std::basic_ostream<char>& ss, const T& t) {
  TupleDumper<std::integral_constant<uint16_t, 0>, T>::dump(ss, t);
}

template <typename DataType>
struct Column : public std::vector<DataType> {};

struct ClearOp {
  template <typename ColumnT>
  static void execute(ColumnT& c) {
    c.clear();
  }
};

struct ResizeOp {
  ResizeOp(size_t s) : new_size(s) {};
  template <typename ColumnT>
  void execute(ColumnT& c) {
    c.resize(new_size);
  }
  size_t new_size = 0;
};

struct ReserveOp {
  ReserveOp(size_t s) : new_size(s) {};
  template <typename ColumnT>
  void execute(ColumnT& c) {
    c.reserve(new_size);
  }
  size_t new_size = 0;
};

struct SwapIndicesOp {
  SwapIndicesOp(size_t a, size_t b) :
      index_a(a), index_b(b) {};
  template <typename ColumnT>
  void execute(ColumnT& c) {
    std::swap(c[index_a], c[index_b]);
  }
  size_t index_a = 0;
  size_t index_b = 0;
};

template <typename... Ts>
class soa : public GenScatterHierarchy<Column, Ts...> {
public:
  static constexpr uint16_t Dimension = sizeof...(Ts);

  template <int col_idx>
  auto& get_column() {
      return FieldAt<col_idx>(*this);
  }

  void insert(Ts... args) {
    insert_impl(args...);
  }

  size_t size() {
    using first_column_type = Column<typename Head<TypeList<Ts...>>::type>;
    using tail_type = typename Tail<TypeList<Column<Ts>...>>::type;
    Tagged<first_column_type, tail_type>& tagged_column = *this;
    return tagged_column.size();
  }

  bool empty() {
    using first_column_type = Column<typename Head<TypeList<Ts...>>::type>;
    using tail_type = typename Tail<TypeList<Column<Ts>...>>::type;
    Tagged<first_column_type, tail_type>& tagged_column = *this;
    return tagged_column.empty();
  }

  void clear() {
    iterate_columns_impl(ClearOp{}, TypeList<Column<Ts>...>{});
  }

  void resize(size_t new_size) {
    ResizeOp op(new_size);
    iterate_columns_impl(op, TypeList<Column<Ts>...>{});
  }

  void reserve(size_t new_size) {
    ReserveOp op(new_size);
    iterate_columns_impl(op, TypeList<Column<Ts>...>{});
  }

  void swap_indices(size_t a, size_t b) {
    SwapIndicesOp op(a, b);
    iterate_columns_impl(op, TypeList<Column<Ts>...>{});
  }

  // if optional end = size_t(-1), it is set to the size of the list
  // operates within the range [begin, end)
  // returns the index of last write or end if no swaps
  size_t swap_matches_to_back(
      std::function<bool(const std::tuple<Ts...>&)> predicate,
      size_t begin = 0, size_t end = size_t(-1)) {
    size_t write_head = (end == size_t(-1) ? size() : end);

    if (begin >= end) {
      return end;
    }

    size_t read_head = write_head;
    while (read_head != begin) {
      --read_head;
      if (predicate(this->as_tuple(read_head))) {
        --write_head;
        swap_indices(read_head, write_head);
      }
    }
    return write_head;
  }

  void overwrite(size_t idx, std::tuple<Ts...> t) {
    overwrite_impl(idx, t, TypeList<Ts...>{});
  }

  size_t overwrite_matches_to_front(
      std::function<bool(const std::tuple<Ts...>&)> predicate) {
    size_t write_head = 0;
    size_t read_head = 0;
    size_t the_size = size();
    while (read_head < the_size) {
      if (predicate(this->as_tuple(read_head))) {
        write_head;
        swap_indices(read_head, write_head);
      }
    }
    return write_head;
  }

  std::tuple<Ts...> as_tuple(size_t idx) {
    std::tuple<Ts...> result;
    fill_tuple(&result, idx, std::integral_constant<uint16_t, 0>{});
    return result;
  }

  // quick sort the elements at indices [being, end)
  void quick_sort(std::mt19937& rng,
                  size_t begin = 0,
                  size_t end = size_t(-1)) {
    // default comparator
    auto comparator = [](
        const std::tuple<Ts...>& a, const std::tuple<Ts...>& b) {
        return a < b;
      };

    quick_sort(rng, comparator, begin, end);
  }
  

  // quick sort the elements at indices [being, end)
  void quick_sort(std::mt19937& rng,
                  std::function<bool(
                      const std::tuple<Ts...>&, const std::tuple<Ts...>&)
                  > comparator,
                  size_t begin = 0,
                  size_t end = size_t(-1)) {
    if (end == size_t(-1)) {
      end = size();
    }

    if (begin + 1 >= end) {
      return;
    }

    std::uniform_int_distribution<size_t> uni(begin, end-1);

    // [begin ... | pivot | ... end ]
    size_t pivot_idx = uni(rng);

    swap_indices(begin, pivot_idx);
    // [pivot | begin ... | ... | ... end ]

    std::tuple<Ts...> pivot_element = this->as_tuple(begin);
    auto Is_gte_pivot = [pivot_element, comparator](const std::tuple<Ts...>& t) {
      return !comparator(t, pivot_element);
    };

    size_t greater_than_pivot_begin = swap_matches_to_back(
        Is_gte_pivot, begin+1, end);

    // [pivot | lt_pivot ... | greater_than_pivot_begin ... ]

    // swap pivot into its correct location
    swap_indices(greater_than_pivot_begin-1, begin);
    // [ lt_pivot ... | pivot| greater_than_pivot_begin ... ]

    // quick sort the two parts
    quick_sort(rng, comparator, greater_than_pivot_begin, end);
    quick_sort(rng, comparator, begin, greater_than_pivot_begin-1);
  }


  template <uint16_t idx>
  void quick_sort_by_field(std::mt19937& rng,
                           size_t begin = 0,
                           size_t end = size_t(-1)) {

    using field_type = typename NthType<idx, TypeList<Ts...>>::type;
    auto comparator = [](
        field_type a,
        field_type b) {
      return a < b;
    };
    quick_sort_by_field<idx>(rng, comparator, begin, end);
  }

  template <uint16_t idx>
  void quick_sort_by_field(std::mt19937& rng,
                           std::function<bool(
                               typename NthType<idx, TypeList<Ts...>>::type,
                               typename NthType<idx, TypeList<Ts...>>::type)>
                           comparator,
                           size_t begin = 0,
                           size_t end = size_t(-1)) {
    auto tuple_comparator = [comparator](
        const std::tuple<Ts...>& a, const std::tuple<Ts...>& b) {
      return comparator(std::get<idx>(a), std::get<idx>(b));
    };
    quick_sort(rng, tuple_comparator, begin, end);
  }

  void dump(std::basic_ostream<char>& ss) {
    constexpr size_t MAX_NUM_ELEMENTS_TO_PRINT = 25;
    size_t num_elements_to_print = size();
    bool too_many_elements = false;
    if (num_elements_to_print > MAX_NUM_ELEMENTS_TO_PRINT) {
      num_elements_to_print = MAX_NUM_ELEMENTS_TO_PRINT;
      too_many_elements = true;
    }
    ss << "soa {\n";
    for (size_t i = 0; i < num_elements_to_print; ++i) {
      std::tuple<Ts...> t = this->as_tuple(i);
      ss << "\t";
      dump_tuple(ss, t);
      ss << std::endl;
    }
    if (size() > MAX_NUM_ELEMENTS_TO_PRINT) {
      ss << "\t..." << std::endl;
    }
    ss << "}" << std::endl;
  }

 private:
  template <typename Op, typename X, typename... Xs>
  void iterate_columns_impl(Op op, TypeList<X, Xs...>) { 
    Tagged<X, TypeList<Xs...>>& tagged_column = *this;
    X& column = tagged_column;
    op.execute(column);
    iterate_columns_impl(op, TypeList<Xs...>{});
  }

  template <typename Op>
  void iterate_columns_impl(Op, TypeList<>) {
    // no op
  }

  template <typename X, typename... Xs>
  void insert_impl(X x, Xs... xs) {
    Tagged<Column<X>, TypeList<Column<Xs>...>>& tagged_column = *this;
    Column<X>& column = tagged_column;
    column.push_back(x);

    insert_impl(xs...);
  }

  void insert_impl() {
    // no-op, end recursion
  }

  template <typename X, typename... Xs>
  void overwrite_impl(size_t idx, std::tuple<Ts...> t, TypeList<X, Xs...>) {
    constexpr size_t fields_left = sizeof...(Xs) + 1;
    constexpr size_t current_field = Dimension - fields_left;
    Tagged<Column<X>, TypeList<Column<Xs>...>>& column = (*this);
    column[idx] = std::get<current_field>(t);
    overwrite_impl(idx, t, TypeList<Xs...>{});
  }

  void overwrite_impl(size_t idx, std::tuple<Ts...> t, TypeList<>) {
    // no op
  }

  template <uint16_t field_idx>
  void fill_tuple(std::tuple<Ts...>* tuple, size_t idx, std::integral_constant<uint16_t, field_idx>) {
    std::get<field_idx>(*tuple) = get_column<field_idx>()[idx];
    fill_tuple(tuple, idx, std::integral_constant<uint16_t, field_idx+1>{});
  }

  void fill_tuple(std::tuple<Ts...>* tuple,
                  size_t idx,
                  std::integral_constant<uint16_t, sizeof...(Ts)>) {
    // no-op, end recursion
  }
};



}

#endif /* _BASICRELATION_H_ */
