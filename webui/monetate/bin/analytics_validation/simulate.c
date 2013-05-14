#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#define DSFMT_DO_NOT_USE_OLD_NAMES
#include "dSFMT.h"


typedef struct Summary {
    uint32_t y0;
    double y1;
    double y2;
} Summary;


/*
 */
static double
parse_double(char* s)
{
    char* end;
    double r = strtod(s, &end);
    if (*end) {
        fprintf(stderr, "Invalid double characters: %s\n", s);
        exit(2);
    }
    return r;
}


/* Parses and splits csv line and into Summary struct
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
    summary->y0 = atoi(strtok_r(NULL, ",", &token_state));
    summary->y1 = atof(strtok_r(NULL, ",", &token_state));
    summary->y2 = atof(strtok_r(NULL, ",", &token_state));
}


/* Write a Summary to a file with Group
 */
static void
output_summary(FILE* fp, int g, Summary* summary)
{
    fprintf(fp, "%d,%d,%lf,%lf\n",
        g,
        summary->y0,
        summary->y1,
        summary->y2);
}


/* Pull in visitor Summaries, run random simulations and write simulation Summaries back out.
 */
static void
simulate(FILE* in, FILE* out, double* group_weights, int groups, int simulations)
{
    /* Compute group cdf */
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

    /* Initialize simulation summaries to zero */
    Summary *totals = calloc(simulations * groups, sizeof(Summary));

    /* Initialize random number generator */
    dsfmt_t dsfmt;
    dsfmt_init_gen_rand(&dsfmt, 1234);
    double *batch_random = (double *) memalign(16, sizeof(double) * simulations);

    /* For each line in the file:
     *   Parse visitor line and return Summary struct
     *   Run simulations and increment the simulation Summary indexed by group and simulation
     */
    char line[1024];
    while (fgets(line, sizeof(line), in)) {
        Summary summary;
        parse_visitor_summary(&summary, line);

        /* Generate simulations doubles in the interval [0, 1). */
        dsfmt_fill_array_close_open(&dsfmt, batch_random, simulations);

        int row = 0;
        for (int i = 0; i < simulations; ++i) {
            /* Select group for visitor this iteration */
            double rnd = batch_random[i];
            int g = 0;
            while (g < groups - 1 && cdf[g] < rnd) { ++g; }

            totals[row + g].y0 += summary.y0;
            totals[row + g].y1 += summary.y1;
            totals[row + g].y2 += summary.y2;

            row += groups;
        }
    }

    /* Write our simulation Summaries to file
     */
    for (int g = 0; g < groups; ++g) {
        for (int i = 0; i < simulations; ++i) {
          output_summary(out, g, &totals[groups * i + g]);
        }
    }

    free(batch_random);
    free(totals);
}

int
main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: simulate iterations weight0 weight1 ...");
        exit(2);
    }

    int simulations = atoi(argv[1]);
    int groups = argc - 2;

    double group_weights[groups];
    for (int i = 0; i < groups; ++i) {
        group_weights[i] = parse_double(argv[i + 2]);
    }

    simulate(stdin, stdout, group_weights, groups, simulations);

    return 0;
}
