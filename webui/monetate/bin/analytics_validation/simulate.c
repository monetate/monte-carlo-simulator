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


/* Parses and splits command line argument into array of ints
 *
 * Arguments:
 * s -- command line argument of form "20,80"
 * group_counts -- array indexed by Group to hold counts
 * num_groups -- total number of groups
 */
static void
parse_group_counts(char* s, int* group_counts, int num_groups)
{
    int i;
    char* tok;
    tok = (strtok(s, ","));
    for (i = 0; i < num_groups; i++) {
        group_counts[i] = atoi(tok);
        tok = strtok(NULL, ",");
    }
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
simulate(FILE* in, FILE* out, int num_groups, int* group_counts, int num_simulations)
{

    /* Compute double cdf for each group, we should get the input as weights instead of samples. */
    int total = 0;
    for (int g = 0; g < num_groups; ++g) {
        total += group_counts[g];
    }

    double cdf[num_groups];
    int cummulative = 0;
    for (int g = 0; g < num_groups; ++g) {
        cummulative += group_counts[g];
        cdf[g] = (double) cummulative / total;
    }


    /* Initialize simulation summaries to zero
     */
    Summary *totals = calloc(num_simulations * num_groups, sizeof(Summary));
    double *batch_random =  memalign(16, sizeof(double) * num_simulations);

    dsfmt_t dsfmt;
    dsfmt_init_gen_rand(&dsfmt, 1234);


    /* For each line in the file:
     *   Parse visitor line and return Summary struct
     *   Run simulations and increment the simulation Summary indexed by group and simulation
     */
    char line[1024];
    while (fgets(line, sizeof(line), in)) {
        Summary summary;
        parse_visitor_summary(&summary, line);

        /* genearte num_simulation doubles in the interval [0, 1). */
        dsfmt_fill_array_close_open(&dsfmt, batch_random, num_simulations);

        int row = 0;
        for (int i = 0; i < num_simulations; ++i) {

            double my_random = batch_random[i];

            /* select group for this visitor this iteration */
            int g = 0;
            while (cdf[g] < my_random && g < num_groups - 1) { ++g; }

            totals[row + g].y0 += summary.y0;
            totals[row + g].y1 += summary.y1;
            totals[row + g].y2 += summary.y2;

            row += num_groups;
        }
    }

    /* Write our simulation Summaries to file
     */
    for (int g = 0; g < num_groups; ++g) {
        for (int i = 0; i < num_simulations; ++i) {
          output_summary(out, g, &totals[num_groups * i + g]);
        }
    }

    free(batch_random);
    free(totals);
}

int
main(int argc, char** argv)
{
    int num_groups = atoi(argv[1]);

    int group_counts[num_groups];
    parse_group_counts(argv[2], group_counts, num_groups);

    int num_simulations = atoi(argv[3]);

    simulate(stdin, stdout, num_groups, group_counts, num_simulations);

    return 0;
}
