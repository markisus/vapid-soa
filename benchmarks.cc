#include <benchmark/benchmark.h>
#include <random>
#include <vector>
#include "vapid/soa.h"

using Id = unsigned short;

std::default_random_engine gen;
std::uniform_int_distribution<Id> sensor_id_gen(0, 100);
std::uniform_int_distribution<Id> object_id_gen(0, 10);
std::uniform_real_distribution<double> real_gen(-10.0, 10.0);

struct SensorData {
    std::array<double, 18> xyz;

    static SensorData random() {
        SensorData s;
        for (size_t i = 0; i < s.xyz.size(); ++i) {
            s.xyz[i] = real_gen(gen);
        }
        return s;
    };
};

template <typename... Ts>
std::ostream& operator<<(std::ostream& cout, const SensorData& s) {
    cout << "{" << s.xyz[0] << ", " << s.xyz[1] << ", " << s.xyz[2] << "}";
    return cout;
}

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

template <typename... Ts>
std::ostream& operator<<(std::ostream& cout, const Measurement& m) {
    cout << "Measurement{" << m.sensor_id << ", " << m.object_id << ", " << m.timestamp << ", " << m.data << "}";
    return cout;
}

struct TestCase {
    vapid::soa<Id, Id, double, SensorData> measurements_soa;
    std::vector<Measurement> measurements_vec;

    static TestCase random() {
        TestCase t;
        for (int i = 0; i < 100000; ++i) {
            Measurement m = Measurement::random();
            t.measurements_vec.push_back(m);
            t.measurements_soa.insert(m.sensor_id, m.object_id, m.timestamp, m.data);
        }
        return t;
    }
};

const TestCase random_data = TestCase::random();

static void BM_SoaSortBySensorId(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto soa = random_data.measurements_soa;
        state.ResumeTiming();

        soa.sort_by_field<0>();
        benchmark::DoNotOptimize(soa.get_column<0>()[0]);
    }
}

static void BM_VecSortBySensorId(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        auto vec = random_data.measurements_vec;
        state.ResumeTiming();

        std::stable_sort(vec.begin(),
                         vec.end(),
                         [](auto& m1, auto& m2) {
                             return m1.sensor_id < m2.sensor_id;
                         });
        benchmark::DoNotOptimize(vec[0]);
    }
}

static void BM_SoaSumTimestamps(benchmark::State& state) {
    const auto& soa = random_data.measurements_soa;
    for (auto _ : state) {
        double sum = 0;
        for (double d : soa.get_column<2>()) {
            sum += d;
        }
        benchmark::DoNotOptimize(sum);
    }
}

static void BM_VecSumTimestamps(benchmark::State& state) {
    const auto& vec = random_data.measurements_vec;
    for (auto _ : state) {
        double sum = 0;
        for (const auto& meas : vec) {
            sum += meas.timestamp;
        }
        benchmark::DoNotOptimize(sum);
    }
}


// Register the function as a benchmark
BENCHMARK(BM_SoaSortBySensorId);
BENCHMARK(BM_VecSortBySensorId);
BENCHMARK(BM_SoaSumTimestamps);
BENCHMARK(BM_VecSumTimestamps);
// Run the benchmark
BENCHMARK_MAIN();
