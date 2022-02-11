#ifndef VAPID_SOA_H
#define VAPID_SOA_H

#include <algorithm>
#include <vector>
#include <tuple>
#include <iostream>
#include <ostream>

namespace vapid {

    template <typename Idx, typename T>
    struct TupleDumper;

    template <uint16_t idx, typename... Ts>
    struct TupleDumper<std::integral_constant<uint16_t, idx>, std::tuple<Ts...>> {
        static void dump(std::basic_ostream<char>& ss, const std::tuple<Ts...>& t) {
            ss << std::get<idx>(t) << ", ";
            TupleDumper<std::integral_constant<uint16_t, idx + 1>, std::tuple<Ts...>>::dump(ss, t);
        }
    };

    template <typename... Ts>
    struct TupleDumper<std::integral_constant<uint16_t, std::tuple_size<std::tuple<Ts...>>::value - 1>, std::tuple<Ts...>> {
        static void dump(std::basic_ostream<char>& ss, const std::tuple<Ts...>& t) {
            ss << std::get< std::tuple_size<std::tuple<Ts...>>::value - 1>(t);
        }
    };

    template <typename T>
    void dump_tuple(std::basic_ostream<char>& ss, const T& t) {
        TupleDumper<std::integral_constant<uint16_t, 0>, T>::dump(ss, t);
    }

    template <typename... Ts>
    class soa {
    public:
        using backing_type = std::tuple<std::vector<Ts>...>;

        template <size_t col_idx>
        using nth_col_type = typename std::tuple_element<col_idx, backing_type>::type;

        template <size_t col_idx>
        using col_type = typename nth_col_type<col_idx>::value_type;

        template<size_t col_idx>
        const nth_col_type<col_idx>& get_column() const {
            return std::get<col_idx>(data_);
        }

        template<size_t col_idx>
        nth_col_type<col_idx>& get_column() {
            const soa<Ts...>* const_this = this;
            return const_cast<nth_col_type<col_idx>&>(const_this->get_column<col_idx>());
        }

        size_t size() const {
            return get_column<0>().size();
        }

        bool empty() const {
            return get_column<0>().empty();
        }

        template <typename... Xs>
        void insert(Xs... xs) {
            insert_impl(std::index_sequence_for<Ts...>{}, std::forward_as_tuple(xs...));
            sort_order_reference_.push_back(size() - 1);
        }

        auto operator[](size_t idx) const {
            return get_row_impl(std::index_sequence_for<Ts...>{}, idx);
        }

        auto operator[](size_t idx) {
            return get_row_impl(std::index_sequence_for<Ts...>{}, idx);
        }

        template <size_t... I>
        auto view(size_t row) const {
            return get_row_impl(std::integer_sequence<size_t, I...>{}, row);
        }

        template <size_t... I>
        auto view(size_t row) {
            return get_row_impl(std::integer_sequence<size_t, I...>{}, row);
        }

        void clear() {
            return clear_impl(std::index_sequence_for<Ts...>{});
        }

        void resize(size_t size) {
            return resize_impl(std::index_sequence_for<Ts...>{}, size);
        }

        void reserve(size_t size) {
            return reserve_impl(std::index_sequence_for<Ts...>{}, size);
        }

        template <size_t col_idx, typename C>
        void sort_by_field(C&& comparator) {
            reset_sort_reference();

            auto& col = get_column<col_idx>();

            auto comparator_wrapper = [=](size_t a, size_t b) {
                return comparator(col[a], col[b]);
            };

            std::stable_sort(sort_order_reference_.begin(),
                sort_order_reference_.end(),
                comparator_wrapper);

            for (size_t i = 0; i < sort_order_reference_.size(); ++i) {
                sort_order_reference_inv_[sort_order_reference_[i]] = i;
            }

            size_t curr = 0;
            size_t cycle_min = 0;
            size_t cycle_len = 0;
            size_t num_visited = 0;
            size_t cycle_idx = 0;

            // std::cout << "Sort order reference size " << sort_order_reference_.size() << "\n";
            // for(size_t i = 0; i < sort_order_reference_.size(); ++i) {
            //     std::cout << sort_order_reference_[i] << " ";
            // }
            // std::cout << "\n";

            while (num_visited < sort_order_reference_.size()) {
                size_t next = sort_order_reference_[curr];
                cycle_min = std::min(cycle_min, next);
                cycle_visited_[curr] = 1;
                ++num_visited;
                ++cycle_len;

                std::cout << "curr " << curr << ", next " << next << "\n";

                if (cycle_min == next) {
                    cycle_mins_[cycle_idx] = cycle_min;
                    cycle_sizes_[cycle_idx] = cycle_len;
                    // we have completely explored this cycle
                    std::cout << "Explored cycle " << cycle_min << " with " << cycle_len << " elements\n";

                    if (num_visited == sort_order_reference_.size()) {
                        // we have explored every cycle
                        // std::cout << "Explored every cycle. \n";
                        break;
                    }

                    // walk up to the next cycle
                    curr = cycle_min;
                    while (cycle_visited_[curr]) {
                        ++curr;
                    }
                    ++cycle_idx;
                    cycle_min = curr;
                    cycle_len = 0;
                } else {
                    curr = next;
                }
            }
                
            sort_by_reference_impl(std::index_sequence_for<Ts...>{});
        }

        template <size_t col_idx>
        void sort_by_field() {
            sort_by_field<col_idx>([](auto&& a, auto&& b) { return a < b; });
        }

        void dump(std::basic_ostream<char>& ss) const {
            constexpr size_t MAX_NUM_ELEMENTS_TO_PRINT = 25;
            size_t num_elements_to_print = size();
            if (num_elements_to_print > MAX_NUM_ELEMENTS_TO_PRINT) {
                num_elements_to_print = MAX_NUM_ELEMENTS_TO_PRINT;
            }
            ss << "soa {\n";
            for (size_t i = 0; i < num_elements_to_print; ++i) {
                const auto t = operator[](i);
                ss << "\t";
                dump_tuple(ss, t);
                ss << std::endl;
            }
            if (size() > MAX_NUM_ELEMENTS_TO_PRINT) {
                ss << "\t..." << std::endl;
            }
            ss << "}" << std::endl;
        }

        void prepare_tmp() {
            // sorting requires the temporary buffers in data_tmp_
            // this function pre-allocates those temporary buffers
            // so that no allocation is done during the sort call
            return resize_impl(std::index_sequence_for<Ts...>{}, data_tmp_, size());
        }

    private:
        template <typename T, size_t... I>
        void insert_impl(std::integer_sequence<size_t, I...>, T t) {
            ((get_column<I>().push_back(std::get<I>(t))), ...);
        }

        template <size_t... I>
        auto get_row_impl(std::integer_sequence<size_t, I...>, size_t row) const {
            return std::tie(get_column<I>()[row]...);
        }

        template <size_t... I>
        auto get_row_impl(std::integer_sequence<size_t, I...>, size_t row) {
            return std::tie(get_column<I>()[row]...);
        }

        template <size_t... I>
        void clear_impl(std::integer_sequence<size_t, I...>) {
            ((get_column<I>().clear()), ...);
        }

        template <size_t... I, typename T>
        void resize_impl(std::integer_sequence<size_t, I...>, T&& data, size_t new_size) {
            ((std::get<I>(data).resize(new_size)), ...);
        }

        template <size_t... I>
        void reserve_impl(std::integer_sequence<size_t, I...>, size_t res_size) {
            ((get_column<I>().reserve(res_size)), ...);
        }

        void reset_sort_reference() {
            for (size_t i = 0; i < size(); ++i) {
                sort_order_reference_[i] = i;
            }
            sort_order_reference_inv_.resize(sort_order_reference_.size());
            cycle_visited_.resize(sort_order_reference_.size());
            for (auto& i : cycle_visited_) {
                i = 0;
            }
            cycle_mins_.resize(sort_order_reference_.size());
            cycle_sizes_.resize(sort_order_reference_.size());
        }

        template <size_t... I>
        void sort_by_reference_impl(std::integer_sequence<size_t, I...>) {
            ((sort_col_by_reference(std::integral_constant<size_t, I>{})), ...);
        }

        template <size_t col_idx>
        void sort_col_by_reference(std::integral_constant<size_t, col_idx>) {
            if (false) {
                auto& src = std::get<col_idx>(data_);
                auto& dst = std::get<col_idx>(data_tmp_);

                dst.resize(src.size());
                for (size_t idx = 0; idx < src.size(); ++idx) {
                    dst[idx] = std::move(src[sort_order_reference_[idx]]);
                }
                std::swap(src, dst);
            } else {
                std::cout << "cycle sizes\n ";
                for(size_t i = 0; i < cycle_sizes_.size(); ++i) {
                    std::cout << cycle_sizes_[i] << " ";
                }
                std::cout << "\n";
                std::cout << "cycle mins\n ";
                for(size_t i = 0; i < cycle_mins_.size(); ++i) {
                    std::cout << cycle_mins_[i] << " ";
                }
                std::cout << "\n";

                
                auto& src = std::get<col_idx>(data_);
                size_t cycle_idx = 0;
                size_t curr = 0;
                size_t num_visited = 0;
                for (size_t cycle_idx = 0; cycle_idx <= cycle_sizes_.size(); ++cycle_idx) {
                    std::cout << "Fixing cycle " << cycle_idx << "\n";
                    curr = cycle_mins_[cycle_idx];

                    size_t cycle_size = cycle_sizes_[cycle_idx];
                    if (cycle_size == 0) {
                        break;
                    }
                    
                    size_t cycle_min = cycle_mins_[cycle_idx];

                    std::cout << "\tsize " << cycle_size << "\n";
                    std::cout << "\tmin " << cycle_min << "\n";

                    for (size_t i = 0; i+1 < cycle_size; ++i) {
                        size_t next = sort_order_reference_[curr];
                        std::cout << "Swapping " << curr << ", " << next << "\n";

                        std::swap(src[curr], src[next]);
                        curr = next;
                        ++num_visited;
                    }
                }
            }
        }

        std::tuple<std::vector<Ts>...> data_;

        // tmp buffers for reordering when sorting
        std::tuple<std::vector<Ts>...> data_tmp_;

        // the reference permutation describing sorted order
        std::vector<size_t> sort_order_reference_;
        std::vector<size_t> sort_order_reference_inv_;
        std::vector<uint8_t> cycle_visited_;
        std::vector<size_t> cycle_sizes_;
        std::vector<size_t> cycle_mins_;
    };

    template <typename... Ts>
    std::ostream& operator<<(std::ostream& cout, const vapid::soa<Ts...>& soa) {
        soa.dump(cout);
        return cout;
    }
}


#endif /* VAPID_SOA_H */
