# Monte Carlo Simulator [![Build Status][travis-image]][travis-link]

[travis-image]: https://secure.travis-ci.org/monetate/monte-carlo-simulator.png?branch=master
[travis-link]: https://travis-ci.org/monetate/monte-carlo-simulator

A command line binary written in C for performing fast Monte Carlo Permutation Tests for validating A/B test results.

Uses [Double precision SIMD-oriented Fast Mersenne Twister
(dSFMT)](http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/) for wicked fast pseudo-randomization.

## Why?

Our clients at [Monetate](http://monetate.com) run thousands of A/B tests each year. We've recently updated our
statistical models to calculate significance of these campaigns' goals (Conversion Rate, Revenue Per Visit, etc) and wanted
a way to:

* Validate the accuracy of these statistical models
* Create a feedback loop to further improve the models

We chose Monte Carlo to accomplish this, but quickly found that running simulations in our language of choice, Python, on A/B Tests with several million
visitors wasn't going to cut it.

## Monte Carlo Testing Intro

*Jump right to [Installation](#installation), [CLI Usage](#cli-usage) or straight to the
[code](/simulate.c) if you're already familiar with Monte Carlo Testing.*

Say you're running an A/B Test on a site to see if the experiment variant had a significant effect on Revenue Per Visit. 
To simplify things a bit, let's begin by looking at just three of these visitors.
Our first visitor, John, visited twice but did not buy anything. Suzy visited once and made one $9 purchase. Bob visited
twice and made two purchases at $8 and $9.

*In this case, a visitor may have multiple visits but the A/B Test randomizes based on visitor to give everyone a
consistent experience.*

| User   | Group      | Visits (y0) | Purchase Amount Sum (y1) | Purchase Amount Sum of Squares (y2) |
| ------ | -------    | -------     | --------------------     | ------------------------------      |
| john   | Experiment | 2           | 0                        | 0                                   |
| suzy   | Control    | 1           | 9                        | 81                                  |
| bob    | Experiment | 2           | 17                       | 145                                 |

From storing the info this way, we can compute our **observed difference** with statistical significance in revenue per visit between the two groups.

To verify the computed significance, we can also send this data through a Monte Carlo Simulator to determine how likely
the difference was due to randomness or not.

The simulator performs multiple permutations. On each interation the simulator will randomly assign visitors to a
theoretical Experiment or Control group and sum up the y0, y1, y2 for all visitors in the group.

We can see that in the table below, two simulations were performed. The first simulation put all three
visitors in the Experiment group and none in the Control group. In the second simulation,
it put John and Suzy in the Expermiment group and Bob in the Control Group.

| Simulation | Group      | Visits (y0) | Purchase Amount Sum (y1) | Purchase Amount Sum of Squares (y2) |
| ---------- | ---------- | ----------- | ------------------------ | ----------------------------------- |
| 0          | Experiment | 5           | 26                       | 226                                 |
| 0          | Control    | 0           | 0                        | 0                                   |
| 1          | Experiment | 3           | 9                        | 81                                  |
| 1          | Control    | 2           | 17                       | 145                                 |

#### Now What?

Let's now assume we had 2 million visitors split evenly into Experiment and Control groups. We observed a difference in
revenue per visit of $1.50 with a p-value of 11% in our two-tailed t-test.

Now we run the Monte Carlo simulator with 10,000 iterations. We can then calculate the difference
between the two groups for each of the ten thousand simulations. Most of these differences will be near zero because we randomly distributed the visitors
between the two groups, but some may lay outside of our $1.50 **observed difference**.

| Simulation | Group      | Visits (y0) | Purchase Amount Sum (y1) | Purchase Amount Sum of Squares (y2) |
| ---------- | ---------- | ----------- | ------------------------ | ----------------------------------- |
| 0          | Experiment | 1000129     | 124124                   | 9193930                             |
| 0          | Control    | 999871      | 111123                   | 10003234                            |
| 1          | Experiment | 999976      | 154320                   | 8100857                             |
| 1          | Control    | 1000024     | 82394                    | 7231043                             |
| ...        | ...        | ...         | ...                      | ...                                 |
| 9999       | Experiment | 993429      | 100001                   | 9534543                             |
| 9999       | Control    | 1006571     | 129993                   | 8738439                             |

If we see that 1000 of the 10,000 random iterations had a difference of more than $1.50, we can say that there is a 10% chance that our
$1.50 **observed difference** was due to randomness.

Although technically not a direct comparison, we can compare our computed p-value of 11% to our simulated 10% result to
determine whether or not the model is accurate enough.

## Installation

#### Downloading a Release

You can grab a pre-compiled binary for your OS and architecture from a [Github Release](/releases):

``` python
wget https://github.com/monetate/monte-carlo-simulator/releases/download/v0.1.0/monte-carlo-simulator-v0.1.0-Linux-i386.tar.gz \
  -O monte-carlo-simulator-v0.1.0-Linux-i386.tar.gz
tar -zxvf monte-carlo-simulator-v0.1.0-Linux-i386.tar.gz
cd monte-carlo-simulator-v0.1.0-Linux-i386
```

#### Building From Source

*Currently, the simulator will only build on a machine with a CPU with Intel's SSE2 instructions and a C compiler which
supports these features.*

It's known to work on:

* Amazon's EC2 instances with gcc 4.1.2
* Travis CI Bluebox workers with gcc 4.6 and clang 3.3

The default `make` target will build a binary named `simulate` in the project's root directory.

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
./simulate 10000 400 600 1000
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
* Austin Rochford

### Running Tests

``` sh
CC=gcc make test
```

### License

This project is released under the [MIT License](http://www.opensource.org/licenses/MIT).

[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/monetate/monte-carlo-simulator/trend.png)](https://bitdeli.com/free "Bitdeli Badge")
