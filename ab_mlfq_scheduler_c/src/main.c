#include "csv_writer.h"
#include "scheduler.h"
#include "workload.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char algorithm[32];
    char scenario[32];
    int n;
    int seed;
} Config;

static const char *all_algorithms[] = {"fcfs", "sjf", "rr", "mlfq", "ab_mlfq"};
static const char *all_scenarios[] = {
    "short_jobs",
    "long_jobs",
    "mixed",
    "interactive",
    "starvation_case",
    "behavior_sensitive"
};
static const int algorithm_count = 5;
static const int scenario_count = 6;

static int is_valid_algorithm(const char *value) {
    if (strcmp(value, "all") == 0) {
        return 1;
    }
    for (int i = 0; i < algorithm_count; ++i) {
        if (strcmp(value, all_algorithms[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_valid_scenario(const char *value) {
    if (strcmp(value, "all") == 0) {
        return 1;
    }
    for (int i = 0; i < scenario_count; ++i) {
        if (strcmp(value, all_scenarios[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void print_usage(void) {
    printf("Usage:\n");
    printf("  ./scheduler <algorithm> <scenario> <n> <seed>\n");
    printf("  ./scheduler --algorithm all --scenario mixed --n 30 --seed 42\n");
}

static int parse_args(int argc, char *argv[], Config *config) {
    strcpy(config->algorithm, "all");
    strcpy(config->scenario, "all");
    config->n = 30;
    config->seed = 42;

    if (argc == 5) {
        strncpy(config->algorithm, argv[1], sizeof(config->algorithm) - 1);
        strncpy(config->scenario, argv[2], sizeof(config->scenario) - 1);
        config->n = atoi(argv[3]);
        config->seed = atoi(argv[4]);
    } else if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
                strncpy(config->algorithm, argv[++i], sizeof(config->algorithm) - 1);
            } else if (strcmp(argv[i], "--scenario") == 0 && i + 1 < argc) {
                strncpy(config->scenario, argv[++i], sizeof(config->scenario) - 1);
            } else if (strcmp(argv[i], "--n") == 0 && i + 1 < argc) {
                config->n = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
                config->seed = atoi(argv[++i]);
            } else {
                print_usage();
                return 0;
            }
        }
    }

    if (!is_valid_algorithm(config->algorithm) || !is_valid_scenario(config->scenario) || config->n <= 0) {
        print_usage();
        return 0;
    }
    if (config->n > MAX_PROCESSES) {
        config->n = MAX_PROCESSES;
    }
    return 1;
}

static int run_algorithm(
    const char *algorithm,
    Process processes[],
    int n,
    const char *scenario
) {
    Metrics metrics;

    if (strcmp(algorithm, "fcfs") == 0) {
        run_fcfs(processes, n, scenario, &metrics);
    } else if (strcmp(algorithm, "sjf") == 0) {
        run_sjf(processes, n, scenario, &metrics);
    } else if (strcmp(algorithm, "rr") == 0) {
        run_rr(processes, n, scenario, &metrics);
    } else if (strcmp(algorithm, "mlfq") == 0) {
        run_mlfq(processes, n, scenario, &metrics);
    } else if (strcmp(algorithm, "ab_mlfq") == 0) {
        run_ab_mlfq(processes, n, scenario, &metrics);
    } else {
        return 0;
    }

    append_summary_csv(&metrics);
    printf("Completed: scenario=%s algorithm=%s\n", scenario, algorithm);
    return 1;
}

int main(int argc, char *argv[]) {
    Config config;
    int scenario_start = 0;
    int scenario_end = scenario_count;
    int algo_start = 0;
    int algo_end = algorithm_count;

    if (!parse_args(argc, argv, &config)) {
        return 1;
    }

    ensure_output_dirs();
    init_summary_csv();

    if (strcmp(config.scenario, "all") != 0) {
        for (int i = 0; i < scenario_count; ++i) {
            if (strcmp(config.scenario, all_scenarios[i]) == 0) {
                scenario_start = i;
                scenario_end = i + 1;
                break;
            }
        }
    }
    if (strcmp(config.algorithm, "all") != 0) {
        for (int i = 0; i < algorithm_count; ++i) {
            if (strcmp(config.algorithm, all_algorithms[i]) == 0) {
                algo_start = i;
                algo_end = i + 1;
                break;
            }
        }
    }

    for (int s = scenario_start; s < scenario_end; ++s) {
        Process base_processes[MAX_PROCESSES];
        int actual_n = generate_workload(all_scenarios[s], config.n, config.seed, base_processes);
        if (actual_n <= 0) {
            fprintf(stderr, "Failed to generate workload for scenario %s\n", all_scenarios[s]);
            continue;
        }
        for (int a = algo_start; a < algo_end; ++a) {
            run_algorithm(all_algorithms[a], base_processes, actual_n, all_scenarios[s]);
        }
    }

    return 0;
}
