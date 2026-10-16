#include "../include/metrics.hpp"
#include "../include/generators.hpp"
#include "../include/algorithms.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iomanip>
#include <functional>
#include <cstdlib>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

using namespace bench;

struct MetricStats {
    double mean = 0.0;
    double median = 0.0;
    double stdev = 0.0;
    double min_removed = 0.0;
    double max_removed = 0.0;
};

// Calcula a Media Aparada (Trimmed Mean): ordena os resultados, retira o melhor (minimo)
// e o pior (maximo) quando houver >= 3 passadas, e tira a media e desvio das centrais.
MetricStats compute_stats(std::vector<double> vals) {
    MetricStats s;
    if (vals.empty()) return s;
    std::sort(vals.begin(), vals.end());

    s.min_removed = vals.front();
    s.max_removed = vals.back();

    std::vector<double> clean_vals;
    if (vals.size() >= 3) {
        clean_vals.assign(vals.begin() + 1, vals.end() - 1);
    } else {
        clean_vals = vals;
    }

    size_t k = clean_vals.size();
    double sum = std::accumulate(clean_vals.begin(), clean_vals.end(), 0.0);
    s.mean = sum / static_cast<double>(k);

    if (k % 2 == 1) {
        s.median = clean_vals[k / 2];
    } else {
        s.median = 0.5 * (clean_vals[k / 2 - 1] + clean_vals[k / 2]);
    }

    if (k > 1) {
        double sq_sum = 0.0;
        for (double v : clean_vals) {
            double d = v - s.mean;
            sq_sum += d * d;
        }
        s.stdev = std::sqrt(sq_sum / static_cast<double>(k - 1));
    }
    return s;
}

struct AlgorithmRunner {
    std::string name;
    std::function<void(std::vector<int32_t>&)> native_fn;
    std::function<void(std::vector<TrackedInt32>&)> tracked_fn;
    bool is_quadratic;
};

void ensure_data_dir() {
#ifdef _WIN32
    _mkdir("data");
#else
    mkdir("data", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif
}

int main(int argc, char* argv[]) {
    bool quick_test = false;
    // Padrao: 10 passadas (descarta a melhor e a pior -> media de 8 execucoes centrais)
    int repetitions = 10;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--quick-test") {
            quick_test = true;
            // Quick-test: 5 passadas (descarta a melhor e a pior -> media de 3 centrais)
            repetitions = 5;
        } else if (arg == "--reps" && i + 1 < argc) {
            repetitions = std::max(3, std::atoi(argv[++i]));
        }
    }

    ensure_data_dir();

    std::vector<AlgorithmRunner> all_algorithms = {
        {"BubbleSort",    bubble_sort<int32_t>,    bubble_sort<TrackedInt32>,    true},
        {"InsertionSort", insertion_sort<int32_t>, insertion_sort<TrackedInt32>, true},
        {"MergeSort",     merge_sort<int32_t>,     merge_sort<TrackedInt32>,     false},
        {"PDQsort",       pdq_sort<int32_t>,       pdq_sort<TrackedInt32>,       false},
        {"PowerSort",     power_sort<int32_t>,     power_sort<TrackedInt32>,     false}
    };

    std::vector<Distribution> distributions = {
        Distribution::RANDOM_UNIFORM,
        Distribution::SORTED,
        Distribution::REVERSE_SORTED,
        Distribution::NEARLY_SORTED,
        Distribution::SORTED_RUNS,
        Distribution::ORGAN_PIPE
    };

    std::vector<size_t> quadratic_and_general_sizes = quick_test
        ? std::vector<size_t>{100, 500, 1000, 2000, 5000}
        : std::vector<size_t>{100, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000};

    std::vector<size_t> efficient_sizes = quick_test
        ? std::vector<size_t>{10, 100, 1000, 10000, 50000, 100000}
        : std::vector<size_t>{10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000, 500000, 1000000, 5000000, 10000000};

    struct BatteryConfig {
        std::string name;
        std::vector<std::string> alg_names;
        std::vector<size_t> sizes;
    };

    std::vector<BatteryConfig> batteries = {
        {"Bateria_1_Quadratica", {"BubbleSort", "InsertionSort"}, quadratic_and_general_sizes},
        {"Bateria_2_Eficiente",  {"MergeSort", "PDQsort", "PowerSort"}, efficient_sizes},
        {"Bateria_3_Geral",      {"BubbleSort", "InsertionSort", "MergeSort", "PDQsort", "PowerSort"}, quadratic_and_general_sizes}
    };

    std::ofstream raw_out("data/raw_results.csv");
    raw_out << "battery,algorithm,distribution,n,rep,seed,time_ms,throughput_eps,"
            << "cpu_cycles,instructions,ipc,branch_misses,cache_misses,"
            << "comparisons,swaps,writes,peak_aux_bytes,max_recursion_depth\n";

    std::ofstream sum_out("data/summary_stats.csv");
    sum_out << "battery,algorithm,distribution,n,reps_total,reps_valid_trimmed,"
            << "time_ms_mean,time_ms_median,time_ms_std,time_ms_best_removed,time_ms_worst_removed,"
            << "throughput_eps_mean,throughput_eps_median,throughput_eps_std,"
            << "cpu_cycles_mean,instructions_mean,ipc_mean,branch_misses_mean,cache_misses_mean,"
            << "comparisons_mean,swaps_mean,writes_mean,peak_aux_bytes,max_recursion_depth\n";

    PerfCounters perf;
    int valid_reps = (repetitions >= 3) ? (repetitions - 2) : repetitions;
    std::cout << "====================================================================\n";
    std::cout << " Benchmark Experimental de Algoritmos de Ordenacao (Padrao SBC)\n";
    std::cout << " Modo: " << (quick_test ? "QUICK-TEST (Validacao Rapida)" : "COMPLETO (Artigo)") << "\n";
    std::cout << " Passadas por cenario: " << repetitions << " (Descarta a melhor e a pior -> Media de "
              << valid_reps << " execucoes)\n";
    std::cout << " Contadores de Hardware (perf_event_open): "
              << (perf.is_available() ? "ATIVOS" : "FALLBACK SEGURO (Windows/VM/Sem permissao)") << "\n";
    std::cout << "====================================================================\n";

    for (const auto& bat : batteries) {
        std::cout << "\n>>> Executando " << bat.name << " ...\n";
        for (size_t n : bat.sizes) {
            for (Distribution dist : distributions) {
                std::string dist_str = distribution_to_string(dist);

                for (const std::string& alg_name : bat.alg_names) {
                    auto it = std::find_if(all_algorithms.begin(), all_algorithms.end(),
                                           [&](const AlgorithmRunner& r) { return r.name == alg_name; });
                    if (it == all_algorithms.end()) continue;
                    const AlgorithmRunner& runner = *it;

                    std::vector<double> v_time, v_thr, v_cyc, v_ins, v_ipc, v_br, v_ca;
                    std::vector<double> v_cmp, v_swp, v_wrt;
                    size_t peak_mem = 0;
                    size_t max_depth = 0;

                    uint64_t cached_cmp = 0, cached_swp = 0, cached_wrt = 0;
                    bool has_cached_pass2 = false;

                    for (int rep = 0; rep < repetitions; ++rep) {
                        uint32_t seed = BASE_SEED + static_cast<uint32_t>(rep * 10007U);
                        std::vector<int32_t> base_data = generate_dataset(n, dist, seed);

                        // PASSAGEM 1: Desempenho Nativo (int32_t) + Hardware
                        std::vector<int32_t> native_vec = base_data;
                        perf.start();
                        auto t0 = std::chrono::steady_clock::now();
                        runner.native_fn(native_vec);
                        auto t1 = std::chrono::steady_clock::now();
                        HardwareMetrics hw = perf.stop();

                        if (!std::is_sorted(native_vec.begin(), native_vec.end())) {
                            std::cerr << "[ERRO CRITICO] Falha de ordenacao em " << runner.name << "\n";
                            return 1;
                        }

                        double time_ns = std::chrono::duration<double, std::nano>(t1 - t0).count();
                        double time_ms = time_ns / 1e6;
                        double thr = (time_ns > 0.0) ? (static_cast<double>(n) / (time_ns / 1e9)) : 0.0;

                        // PASSAGEM 2: Instrumentacao Algoritmica (TrackedInt32)
                        bool run_pass2_now = !has_cached_pass2 || (n <= 10000 && !runner.is_quadratic);
                        if (run_pass2_now) {
                            std::vector<TrackedInt32> tracked_vec(n);
                            for (size_t idx = 0; idx < n; ++idx) {
                                tracked_vec[idx].value = base_data[idx];
                            }

                            AlgorithmMetrics::instance().reset();
                            runner.tracked_fn(tracked_vec);

                            cached_cmp = AlgorithmMetrics::instance().comparisons;
                            cached_swp = AlgorithmMetrics::instance().swaps;
                            cached_wrt = AlgorithmMetrics::instance().writes;
                            peak_mem   = AlgorithmMetrics::instance().peak_aux_bytes;
                            max_depth  = AlgorithmMetrics::instance().max_recursion_depth;
                            has_cached_pass2 = true;
                        }

                        v_time.push_back(time_ms);
                        v_thr.push_back(thr);
                        v_cyc.push_back(static_cast<double>(hw.cpu_cycles));
                        v_ins.push_back(static_cast<double>(hw.instructions));
                        v_ipc.push_back(hw.ipc);
                        v_br.push_back(static_cast<double>(hw.branch_misses));
                        v_ca.push_back(static_cast<double>(hw.cache_misses));
                        v_cmp.push_back(static_cast<double>(cached_cmp));
                        v_swp.push_back(static_cast<double>(cached_swp));
                        v_wrt.push_back(static_cast<double>(cached_wrt));

                        raw_out << bat.name << "," << runner.name << "," << dist_str << ","
                                << n << "," << rep << "," << seed << ","
                                << std::fixed << std::setprecision(6) << time_ms << ","
                                << std::fixed << std::setprecision(2) << thr << ","
                                << hw.cpu_cycles << "," << hw.instructions << ","
                                << std::fixed << std::setprecision(4) << hw.ipc << ","
                                << hw.branch_misses << "," << hw.cache_misses << ","
                                << cached_cmp << "," << cached_swp << "," << cached_wrt << ","
                                << peak_mem << "," << max_depth << "\n";
                    }

                    MetricStats st_time = compute_stats(v_time);
                    MetricStats st_thr  = compute_stats(v_thr);
                    MetricStats st_cyc  = compute_stats(v_cyc);
                    MetricStats st_ins  = compute_stats(v_ins);
                    MetricStats st_ipc  = compute_stats(v_ipc);
                    MetricStats st_br   = compute_stats(v_br);
                    MetricStats st_ca   = compute_stats(v_ca);
                    MetricStats st_cmp  = compute_stats(v_cmp);
                    MetricStats st_swp  = compute_stats(v_swp);
                    MetricStats st_wrt  = compute_stats(v_wrt);

                    sum_out << bat.name << "," << runner.name << "," << dist_str << ","
                            << n << "," << repetitions << "," << valid_reps << ","
                            << std::fixed << std::setprecision(6)
                            << st_time.mean << "," << st_time.median << "," << st_time.stdev << ","
                            << st_time.min_removed << "," << st_time.max_removed << ","
                            << std::fixed << std::setprecision(2)
                            << st_thr.mean << "," << st_thr.median << "," << st_thr.stdev << ","
                            << std::fixed << std::setprecision(1)
                            << st_cyc.mean << "," << st_ins.mean << ","
                            << std::fixed << std::setprecision(4) << st_ipc.mean << ","
                            << std::fixed << std::setprecision(1)
                            << st_br.mean << "," << st_ca.mean << ","
                            << st_cmp.mean << "," << st_swp.mean << "," << st_wrt.mean << ","
                            << peak_mem << "," << max_depth << "\n";
                }
            }
            std::cout << "  [OK] Concluido N = " << n << "\n";
        }
    }

    raw_out.close();
    sum_out.close();
    std::cout << "\n[SUCESSO] Resultados exportados em data/raw_results.csv e data/summary_stats.csv\n";
    return 0;
}
