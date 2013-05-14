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



uint32_t
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
parse_double(char* s)
{
    char* end;
    double r = strtod(s, &end);
    if (*end) {
        fprintf(stderr, "Non floating point characters: %s\n", s);
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
    summary->y0 = parse_uint32(strtok_r(NULL, ",", &token_state));
    summary->y1 = parse_double(strtok_r(NULL, ",", &token_state));
    summary->y2 = parse_double(strtok_r(NULL, "\n", &token_state));
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
    Summary *summaries = calloc(simulations * groups, sizeof(Summary));

    /* Initialize random number generator */
    dsfmt_t dsfmt;
    dsfmt_init_gen_rand(&dsfmt, 1234);
    double *visitor_random = (double *) memalign(16, sizeof(double) * simulations);

    /* For each line in the file:
     *   Parse visitor line and return Summary struct
     *   Run simulations and increment the simulation Summary indexed by group and simulation
     */
    char line[1024];
    while (fgets(line, sizeof(line), in)) {
        Summary visitor_summary;
        parse_visitor_summary(&visitor_summary, line);

        /* Generate simulations doubles in the interval [0, 1). */
        dsfmt_fill_array_close_open(&dsfmt, visitor_random, simulations);

        int row = 0;
        for (int i = 0; i < simulations; ++i) {
            /* Select group for visitor this iteration */
            double rnd = visitor_random[i];
            int g = 0;
            while (g < groups - 1 && cdf[g] < rnd) { ++g; }

            summaries[row + g].y0 += visitor_summary.y0;
            summaries[row + g].y1 += visitor_summary.y1;
            summaries[row + g].y2 += visitor_summary.y2;

            row += groups; // groups * i
        }
    }

    /* Write our simulation Summaries to file
     */
    for (int i = 0; i < simulations; ++i) {
        for (int g = 0; g < groups; ++g) {
            Summary *summary = &summaries[groups * i + g];
            fprintf(out, "%d,%d,%d,%lf,%lf\n",
                i, g, summary->y0, summary->y1, summary->y2);
        }
    }

    free(visitor_random);
    free(summaries);
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
