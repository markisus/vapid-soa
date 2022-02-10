#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include <array>
#include "vapid/soa.h"

using Id = unsigned short;

std::default_random_engine gen;
std::uniform_int_distribution<Id> sensor_id_gen(0, 100);
std::uniform_int_distribution<Id> object_id_gen(0, 10);
std::uniform_real_distribution<double> real_gen(-10.0, 10.0);
std::uniform_int_distribution<int> char_gen('a', 'z');


struct ArraySensorData {
    std::array<double, 18> xyz;

    static ArraySensorData random() {
        ArraySensorData s;
        for (size_t i = 0; i < s.xyz.size(); ++i) {
            s.xyz[i] = real_gen(gen);
        }
        return s;
    };
};

struct StringSensorData {
// contribution from greg7mdp during code review
// for testing string data
// https://github.com/markisus/vapid-soa/pull/3#issuecomment-1035390996
    
    std::string data;

    static StringSensorData random() {
        StringSensorData res;
        char buff[31];
        for (size_t i = 0; i < 30; ++i) {
            buff[i] = char_gen(gen);
        }       
        buff[30] = 0;
        res.data = buff;
        return res;
    };
};

template <typename SensorData>
struct Measurement {
    Id sensor_id;
    Id object_id;
    double timestamp;
    SensorData data;

    static Measurement random() {
        Measurement m;
        m.sensor_id = sensor_id_gen(gen);
        m.object_id = object_id_gen(gen);
        m.timestamp = real_gen(gen);
        m.data = SensorData::random();
        return m;
    }
};

template <typename SensorData>
struct TestCase {
    vapid::soa<Id, Id, double, SensorData> measurements_soa;
    std::vector<Measurement<SensorData>> measurements_vec;

    static TestCase random() {
        TestCase t;
        for (int i = 0; i < 100000; ++i) {
            Measurement<SensorData> m = Measurement<SensorData>::random();
            t.measurements_vec.push_back(m);
            t.measurements_soa.insert(m.sensor_id, m.object_id, m.timestamp, m.data);
        }
        t.measurements_soa.prepare_tmp();
        return t;
    }
};

const auto random_array_data = TestCase<ArraySensorData>::random();
const auto random_string_data = TestCase<StringSensorData>::random();

static void BM_SoaSortBySensorId_ArrayData(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto soa = random_array_data.measurements_soa;
        state.ResumeTiming();

        soa.sort_by_field<0>();
        benchmark::DoNotOptimize(soa.get_column<0>()[0]);
    }
}

static void BM_SoaSortBySensorId_StringData(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto soa = random_string_data.measurements_soa;
        state.ResumeTiming();

        soa.sort_by_field<0>();
        benchmark::DoNotOptimize(soa.get_column<0>()[0]);
    }
}

static void BM_VecSortBySensorId_ArrayData(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto vec = random_array_data.measurements_vec;
        state.ResumeTiming();

        std::stable_sort(vec.begin(),
                         vec.end(),
                         [](auto& m1, auto& m2) {
                             return m1.sensor_id < m2.sensor_id;
                         });
        benchmark::DoNotOptimize(vec[0]);
    }
}

static void BM_VecSortBySensorId_StringData(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto vec = random_string_data.measurements_vec;
        state.ResumeTiming();

        std::stable_sort(vec.begin(),
                         vec.end(),
                         [](auto& m1, auto& m2) {
                             return m1.sensor_id < m2.sensor_id;
                         });
        benchmark::DoNotOptimize(vec[0]);
    }
}

static void BM_SoaSumTimestamps_ArrayData(benchmark::State& state) {
    const auto& soa = random_array_data.measurements_soa;
    for (auto _ : state) {
        double sum = 0;
        for (double d : soa.get_column<2>()) {
            sum += d;
        }
        benchmark::DoNotOptimize(sum);
    }
}

static void BM_VecSumTimestamps_ArrayData(benchmark::State& state) {
    const auto& vec = random_array_data.measurements_vec;
    for (auto _ : state) {
        double sum = 0;
        for (const auto& meas : vec) {
            sum += meas.timestamp;
        }
        benchmark::DoNotOptimize(sum);
    }
}


// Register the function as a benchmark
BENCHMARK(BM_SoaSortBySensorId_ArrayData);
BENCHMARK(BM_VecSortBySensorId_ArrayData);
BENCHMARK(BM_SoaSortBySensorId_StringData);
BENCHMARK(BM_VecSortBySensorId_StringData);

BENCHMARK(BM_SoaSumTimestamps_ArrayData);
BENCHMARK(BM_VecSumTimestamps_ArrayData);
// Run the benchmark
BENCHMARK_MAIN();
