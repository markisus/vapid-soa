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

    enum class SoaPref { speed, space };

    template <SoaPref pref, typename... Ts>
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
            const soa<pref, Ts...>* const_this = this;
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
            if constexpr (pref == SoaPref::speed)
               sort_order_reference_.push_back(size() - 1);
        }

        auto get_row(size_t row) const {
            return get_row_impl(std::index_sequence_for<Ts...>{}, row);
        }

        auto get_row(size_t row) {
            return get_row_impl(std::index_sequence_for<Ts...>{}, row);
        }

        auto operator[](size_t idx) const {
            return get_row(idx);
        }

        auto operator[](size_t idx) {
            return get_row(idx);
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
            if constexpr (pref == SoaPref::speed) {
               reset_sort_reference();
               auto& col = get_column<col_idx>();

               auto comparator_wrapper = [=](size_t a, size_t b) {
                   return comparator(col[a], col[b]);
               };

                std::stable_sort(sort_order_reference_.begin(),
                     sort_order_reference_.end(),
                     comparator_wrapper);
               sort_by_reference_impl(std::index_sequence_for<Ts...>{});
             } else {
                size_t num_elems =  size();
                thread_local std::vector<size_t> sort_order_tmp;
                thread_local std::vector<size_t> sort_order_reference;
                
                sort_order_tmp.resize(num_elems);
                sort_order_reference.resize(num_elems);
                for (size_t i = 0; i < num_elems; ++i) {
                    sort_order_tmp[i] = i;
                }
                auto& col = get_column<col_idx>();

                auto comparator_wrapper = [=](size_t a, size_t b) {
                    return comparator(col[a], col[b]);
                };

                std::stable_sort(sort_order_tmp.begin(),
                                 sort_order_tmp.end(),
                                 comparator_wrapper);
                for (size_t i=0; i<num_elems; ++i)
                   sort_order_reference[sort_order_tmp[i]] = i;
            
                sort_by_reference_impl(sort_order_tmp, sort_order_reference, std::index_sequence_for<Ts...>{});
            }
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
                const auto t = get_row(i);
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
            if constexpr (pref == SoaPref::speed) {
                // sorting requires the temporary buffers in data_tmp_
                // this function pre-allocates those temporary buffers
                // so that no allocation is done during the sort call
                resize_impl(std::index_sequence_for<Ts...>{}, data_tmp_, size());  
            }
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
            if constexpr (pref == SoaPref::speed) {
                for (size_t i = 0; i < size(); ++i) {
                    sort_order_reference_[i] = i;
                }
            }
        }

        template <size_t... I>
        void sort_by_reference_impl(std::vector<size_t> &sort_order_tmp,
                                    const std::vector<size_t> &sort_order_reference,
                                    std::integer_sequence<size_t, I...>) {
            ((sort_col_by_reference(sort_order_tmp, sort_order_reference, std::integral_constant<size_t, I>{})), ...);
        }

        template <size_t... I>
        void sort_by_reference_impl(std::integer_sequence<size_t, I...>) {
            ((sort_col_by_reference(std::integral_constant<size_t, I>{})), ...);
        }

        
        // o provides the new position for each index. For example if o[0] = 3, 
        // it means that the values at index 0 should be moved to index 3 to be sorted.
        // c contains the values to be reordered according to o.
        template <class C>
        void reorder(C &c, std::vector<size_t> &o)
        {
            using std::swap;
            size_t num_elems = o.size();
            for (size_t i=0; i<num_elems; ++i)
            {
                while (o[i] != i)
                {
                    swap(c[i], c[o[i]]);
                    swap(o[i], o[o[i]]);
                }
            }
        }

        template <size_t col_idx>
        void sort_col_by_reference(std::vector<size_t> &sort_order_tmp,
                                   const std::vector<size_t> &sort_order_reference,
                                   std::integral_constant<size_t, col_idx>) {
            auto& col = std::get<col_idx>(data_);
            sort_order_tmp = sort_order_reference;
            reorder(col, sort_order_tmp);
        }

        template <size_t col_idx>
        void sort_col_by_reference(std::integral_constant<size_t, col_idx>) {
            auto& src = std::get<col_idx>(data_);
            auto& dst = std::get<col_idx>(data_tmp_);
            dst.resize(src.size());
            for (size_t idx = 0; idx < src.size(); ++idx) {
                dst[idx] = std::move(src[sort_order_reference_[idx]]);
            }
            std::swap(src, dst);
        }

        std::tuple<std::vector<Ts>...> data_;

        struct empty_ { };
        using ts = std::conditional_t<pref == SoaPref::speed, std::tuple<std::vector<Ts>...>, empty_>;
        using cond_size_vec = std::conditional_t<pref == SoaPref::speed, std::vector<size_t>, empty_>;

        
        // tmp buffers for reordering when sorting
        ts data_tmp_;

        // the reference permutation describing sorted order
        cond_size_vec sort_order_reference_;
    };

    template <SoaPref pref, typename... Ts>
    std::ostream& operator<<(std::ostream& cout, const vapid::soa<pref, Ts...>& soa) {
        soa.dump(cout);
        return cout;
    }
}


#endif /* VAPID_SOA_H */
