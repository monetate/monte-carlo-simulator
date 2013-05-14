/*
 * Copyright 2013 Monetate, Inc.
 *
 * Links against Double precision SIMD-oriented Fast Mersenne Twister (dSFMT) library.
 *    http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/index.html
 *
 * build
 *    gcc -O3 -std=c99 -msse2 -DHAVE_SSE2 -DDSFMT_MEXP=19937 -o simulate dSFMT.c simulate.c
 *
 * run
 *    simulate iterations group_weight_0 group_weight_1 ...
 */

#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define DSFMT_DO_NOT_USE_OLD_NAMES
#include "dSFMT.h"


typedef struct Summary {
    double y0;
    double y1;
    double y2;
} Summary;



static uint32_t
parse_uint32(char const* s)
{
    char* end;
    uint32_t r = strtol(s, &end, 10);
    if (*end) {
        fprintf(stderr, "Non integral characters: %s\n", s);
        exit(2);
    }
    return r;
}


static double
parse_double(char const* s)
{
    char* end;
    double r = strtod(s, &end);
    if (*end) {
        fprintf(stderr, "Non floating point characters: %s\n", s);
        exit(2);
    }
    return r;
}


/*
 * Parses and splits csv line and into Summary struct
 *
 * Arguments:
 * summary -- Summary struct to use
 * line -- Comma separated line of form "2.1000305503.1365983513775,1,0.0,0.0"
 */
static void
parse_visitor_summary(Summary* summary, char* line)
{
    char* token_state;
    strtok_r(line, ",", &token_state); // monetate_id, which we don't care about
    summary->y0 = parse_double(strtok_r(NULL, ",", &token_state));
    summary->y1 = parse_double(strtok_r(NULL, ",", &token_state));
    summary->y2 = parse_double(strtok_r(NULL, "\n", &token_state));
}


/*
 * Pull in visitor Summaries, run random simulations and write simulation Summaries back out.
 */
static void
simulate(FILE* in, FILE* out, double* group_weights, int groups, int simulations)
{
    /* Compute group cdf. */
    double cdf[groups];
    double total_weight = 0;
    for (int g = 0; g < groups; ++g) {
        total_weight += group_weights[g];
    }
    double cumulative_weight = 0;
    for (int g = 0; g < groups; ++g) {
        cumulative_weight += group_weights[g];
        cdf[g] = (double) cumulative_weight / total_weight;
    }

    /* Initialize group_summaries to zero. */
    Summary *group_summaries = calloc(simulations * groups, sizeof(Summary));

    /* Initialize random number generator. */
    dsfmt_t dsfmt;
    dsfmt_init_gen_rand(&dsfmt, 1234);

    /* 128 bit alignment for sse2 ops. */
    double *visitor_randoms = (double *) memalign(16, sizeof(double) * simulations);

    /* For each input line:
     *   Parse visitor summary line.
     *   Accumulate into random summary group for each simulation trial.
     */
    char line[128];
    while (fgets(line, sizeof(line), in)) {
        Summary visitor_summary;
        parse_visitor_summary(&visitor_summary, line);

        /* Generate random doubles in the interval [0, 1). */
        dsfmt_fill_array_close_open(&dsfmt, visitor_randoms, simulations);

        Summary *row = group_summaries;
        for (int i = 0; i < simulations; ++i) {
            /* Select group for visitor this iteration */
            double rnd = visitor_randoms[i];
            int g = 0;
            while (g < groups - 1 && cdf[g] < rnd) { ++g; }

            Summary *group_summary = row + g;
            group_summary->y0 += visitor_summary.y0;
            group_summary->y1 += visitor_summary.y1;
            group_summary->y2 += visitor_summary.y2;

            row += groups;
        }
    }

    /* Output group_summaries. */
    Summary *group_summary = group_summaries;
    for (int i = 0; i < simulations; ++i) {
        for (int g = 0; g < groups; ++g) {
            fprintf(out, "%d,%d,%lf,%lf,%lf\n", i, g,
                group_summary->y0, group_summary->y1, group_summary->y2);
            ++group_summary;
        }
    }

    free(visitor_randoms);
    free(group_summaries);
}

int
main(int argc, char** argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: simulate iterations weight0 weight1 ...\n");
        exit(1);
    }

    int simulations = atoi(argv[1]);
    int groups = argc - 2;

    double group_weights[groups];
    for (int i = 0; i < groups; ++i) {
        group_weights[i] = parse_double(argv[i + 2]);
    }

    /* TODO: Assert simulations even number >= 382 sse2 implementation. */
    simulate(stdin, stdout, group_weights, groups, simulations);

    return 0;
}
