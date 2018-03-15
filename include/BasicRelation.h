#ifndef _BASICRELATION_H_
#define _BASICRELATION_H_

#include <vector>
#include <tuple>
#include <iostream>
#include <functional>
#include "GenScatterHierarchy.h"

namespace vapid_db {

template <typename DataType>
struct Column {
  std::vector<DataType> data;
};

struct ClearOp {
  template <typename ColumnT>
  static void execute(ColumnT& c) {
    c.data.clear();
  }
};

struct ResizeOp {
  ResizeOp(size_t s) : new_size(s) {};
  template <typename ColumnT>
  void execute(ColumnT& c) {
    c.data.resize(new_size);
  }
  size_t new_size = 0;
};

struct ReserveOp {
  ReserveOp(size_t s) : new_size(s) {};
  template <typename ColumnT>
  void execute(ColumnT& c) {
    c.data.reserve(new_size);
  }
  size_t new_size = 0;
};

struct SwapIndicesOp {
  SwapIndicesOp(size_t a, size_t b) :
      index_a(a), index_b(b) {};
  template <typename ColumnT>
  void execute(ColumnT& c) {
    std::swap(c.data[index_a], c.data[index_b]);
  }
  size_t index_a = 0;
  size_t index_b = 0;
};

template <typename... Ts>
class BasicRelation : public GenScatterHierarchy<Column, Ts...> {
 public:
  static constexpr uint16_t Dimension = LengthOf<Ts...>::value;

  void insert(Ts... args) {
    insert_impl(args...);
  }

  size_t size() {
    using first_column_type = Column<typename Head<TypeList<Ts...>>::type>;
    using tail_type = typename Tail<TypeList<Column<Ts>...>>::type;
    Tagged<first_column_type, tail_type>& tagged_column = *this;
    return tagged_column.data.size();
  }

  bool empty() {
    using first_column_type = Column<typename Head<TypeList<Ts...>>::type>;
    using tail_type = typename Tail<TypeList<Column<Ts>...>>::type;
    Tagged<first_column_type, tail_type>& tagged_column = *this;
    return tagged_column.data.empty();
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

  size_t swap_matches_to_back(
      std::function<bool(const std::tuple<Ts...>&)> predicate) {
    size_t write_head = size();
    size_t read_head = write_head;
    while (read_head != 0) {
      --read_head;
      if (predicate((*this)[read_head])) {
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
      if (predicate((*this)[read_head])) {
        write_head;
        swap_indices(read_head, write_head);
      }
    }
    return write_head;
  }


  std::tuple<Ts...> operator[](size_t idx) {
    std::tuple<Ts...> result;
    fill_tuple(&result, idx, std::integral_constant<uint16_t, 0>{});
    return result;
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
    column.data.push_back(x);

    insert_impl(xs...);
  }

  void insert_impl() {
    // no-op, end recursion
  }

  template <typename X, typename... Xs>
  void overwrite_impl(size_t idx, std::tuple<Ts...> t, TypeList<X, Xs...>) {
    constexpr size_t fields_left = LengthOf<X, Xs...>::value;
    constexpr size_t current_field = Dimension - fields_left;
    Tagged<Column<X>, TypeList<Column<Xs>...>>& column = (*this);
    column.data[idx] = std::get<current_field>(t);
    overwrite_impl(idx, t, TypeList<Xs...>{});
  }

  void overwrite_impl(size_t idx, std::tuple<Ts...> t, TypeList<>) {
    // no op
  }

  template <uint16_t field_idx>
  void fill_tuple(std::tuple<Ts...>* tuple, size_t idx, std::integral_constant<uint16_t, field_idx>) {
    std::get<field_idx>(*tuple) = FieldAt<field_idx>(*this).data[idx];
    fill_tuple(tuple, idx, std::integral_constant<uint16_t, field_idx+1>{});
  }

  void fill_tuple(std::tuple<Ts...>* tuple,
                  size_t idx,
                  std::integral_constant<uint16_t, LengthOf<Ts...>::value>) {
    // no-op, end recursion
  }
};



}

#endif /* _BASICRELATION_H_ */
