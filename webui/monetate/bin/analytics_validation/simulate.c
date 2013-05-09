#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

typedef int Group;

typedef struct Summary {
    uint32_t y0;
    double y1;
    double y2;
} Summary;

static int
rand_int(int n)
{
  int limit = RAND_MAX - RAND_MAX % n;
  int rnd;

  do {
    rnd = random();
  } while (rnd >= limit);
  return rnd % n;
}

/* Parses and splits command line argument into array of ints
 *
 * Arguments:
 * s -- command line argument of form "20,80"
 * group_counts -- array indexed by Group to hold counts
 * num_groups -- total number of groups
 */
void
parse_group_counts(char* s, int* group_counts, int num_groups)
{
  int i;
  char* tok;
  tok = (strtok(s, ","));
  for(i=0; i < num_groups; i++) {
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
void
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
void
output_summary(FILE* fp, Group g, Summary* summary)
{
  fprintf(fp, "%d,%d,%f,%f\n",
      g,
      summary->y0,
      summary->y1,
      summary->y2);
}

/* Return a random group from possible choices
 *
 * Arguments:
 * visitor_count -- Total possible visitors/choices
 * choices -- array of possible group choices  (ex: [1, 2, 2, 3, 3, 3])
 */
Group
get_random_group(int visitor_count, int *choices)
{
  return choices[rand_int(visitor_count)];
}

/* Pull in visitor Summaries, run random simulations and write simulation Summaries back out.
 */
void
simulate(FILE* in, FILE* out, int num_groups, int* group_counts, int num_simulations)
{
  srand (time(NULL));

  int i;

  int s;
  Group g;
  int visitor_count=0;
  for(g=0; g < num_groups; g++){
    visitor_count += group_counts[g];
  }

  /* Create choices array that we can pull a random weighted group
   * Ex: num_groups = 3, group_counts [1, 2, 3]
   * Will create choices = [0, 1, 1, 2, 2, 2]
   * We can then use choices in get_random_group to pull out a weighted random group.
   */
  int j, v = 0;
  int *choices = malloc(visitor_count * sizeof(Group));
  for(g=0; g < num_groups; g++){
    for(j=0; j < group_counts[g]; j++) {
      choices[v] = g;
      v++; // v is visitor index
    }
  }

  /* Initialize simulation summaries to zero
   */
  Summary *simulations = calloc(num_groups * num_simulations, sizeof(Summary));

  /* For each line in the file:
   *   Parse visitor line and return Summary struct
   *   Run simulations and increment the simulation Summary indexed by group and simulation
   */
  char line[1024];
  while (fgets(line, sizeof(line), in)){
    Summary summary;
    parse_visitor_summary(&summary, line);

    for(s=0; s < num_simulations; s++) {
      Group group = get_random_group(visitor_count, choices);

      simulations[num_groups * s + group].y0 += summary.y0;
      simulations[num_groups * s + group].y1 += summary.y1;
      simulations[num_groups * s + group].y2 += summary.y2;
    }
  }

  /* Write our simulation Summaries to file
   */
  for(g=0; g < num_groups; g++){
    for(s=0; s < num_simulations; s++){
      output_summary(out, g, &simulations[num_groups * s + g]);
    }
  }

  free(simulations);
  free(choices);
}

int
main(int argc, char** argv)
{
  int num_groups = atoi(argv[1]);
  int num_simulations = atoi(argv[3]);
  int group_counts[num_groups];
  parse_group_counts(argv[2], group_counts, num_groups);
  simulate(stdin, stdout, num_groups, group_counts, num_simulations);
  return 0;
}
