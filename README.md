# Monte Carlo Permutation Simulator

A command line binary written in C for performing fast Monte Carlo Permutation Tests.

Uses [Double precision SIMD-oriented Fast Mersenne Twister
(dSFMT)](http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/) for wicked fast pseudo-randomization.

## Monte Carlo Testing Intro

*Jump right to [Building](#building) and [CLI Usage](#cli-usage) if you're already familiar with Monte Carlo Testing.*

Say you're running an A/B Test on a site to see if the experiment variant had a significant effect on revenue per visit. 

To simplify things a bit, let's just look at three of these visitors. Our first visitor, John, visits twice but does not buy anything. Suzy visits once and makes one $9 purchase. Bob visits
twice and makes two purchases at $8 and $9.

*In this case, a visitor may have multiple visits but the A/B Test randomizes based on visitor to give everyone a
consistent experience.*

| User   | Group      | Visits (y0) | Purchase Amount Sum (y1) | Purchase Amount Sum of Squares (y2) |
| ------ | -------    | -------     | --------------------     | ------------------------------      |
| john   | Experiment | 2           | 0                        | 0                                   |
| suzy   | Control    | 1           | 9                        | 81                                  |
| bob    | Experiment | 2           | 17                       | 245                                 |
| ...    | ...        | ...         | ...                      | ...                                 |

From storing the info this way, we can compute our **observed difference** in revenue per visit between the two groups.
Let's say our experiment group bought $1.50 more per visit than the control group. We can also compute the statistical
significance of this result.

To verify the computed significance, we can also send this data through a Monte Carlo Simulator to determine how likely
the difference was due to randomness or not.

If we go through every visitor 10,000 times and randomly place them in a theoretical Experimental or Control group,
we can sum up y0, y1 and y2 for each group. We can then calculate the difference
between the two groups for each of the ten thousand simulations and determine how many differences lay outside our $1.50
**observed difference**.

If we go back to looking at only our three visitors, we can see that in this case, the first simulation put all three
visitors in the Experiment group and none in the Control group. In the second simulation, it put John and Suzy in the Expermiment group and Bob in the Control Group.

| Simulation | Group      | Visits (y0) | Purchase Amount Sum (y1) | Purchase Amount Sum of Squares (y2) |
| ------     | -------    | -------     | --------------------     | ------------------------------      |
| 0          | Experiment | 5           | 26                       | 226                                 |
| 0          | Control    | 0           | 0                        | 0                                   |
| 1          | Experiment | 3           | 9                        | 81                                  |
| 1          | Control    | 2           | 17                       | 145                                 |
| ...        | ...        | ...         | ...                      | ...                                 |

If we see that 9000 randomized simulations had a difference of less than $1.50, we can say that there is a 10% chance that our
$1.50 **observed difference** was due to randomness. Most of the time you'd describe that as having a p value of 90 or a
confidence level of 90%.

## Building

The default `make` target will build a binary named `simulate` in the root directory.

``` sh
CC=gcc make
```

## CLI Usage

The `simulate` binary takes a csv on `stdin` and outputs a resulting csv on `stdout`.

``` python
cat /path/to/samples.csv | (./simulate 10000 0.5 0.5) > /path/to/results.csv
```

#### Arguments

It accepts the number of simulations to run as the first positional argument. The following arguments describe the
weighting for each of your groups. 

You can pass weights as percentages or whole numbers. The following three variants are all equivalent:

``` python
./simulate 10000 2 3 5
./simulate 10000 0.2 0.3 0.5
./simulate 10000 400 600 10000
```

#### Input CSV

The input csv is assumed to have exactly four columns with no header row.

* `id`: Unique id
* `y0`: The number of samples 
* `y1`: The sum of the samples
* `y2`: The sum of squares of the samples

``` python
john,2,0.0,0.0
suzy,1,9.0,81.0
bob,2,17,145.0
```

#### Output

The result csv will contain 5 columns with no header row.

* `simulation`: Simulation index
* `group_id`: Group id
* `y0`: The number of samples in the group
* `y1`: The sum of the samples in the group
* `y2`: The sum of squares in the group

``` python
0,0,5.0,26.0,226.0
0,1,0.0,0.0,0.0
1,0,3.0,9.0,81.0
1,1,2.0,17.0,145.0
...
```

### Contributors

* Jeffrey Persch
* Chris Conley
* Gil Raphaelli

### Running Tests

``` sh
CC=gcc make test
```

### License

This project is released under the [MIT License](http://www.opensource.org/licenses/MIT).
