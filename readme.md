# Lab work 9: Count words parallel using tbb high-level tools

Authors (team): [Shevchenko Ivan](https://github.com/ishevche)<br>
Additional tasks: implemented using both tbb::parallel_pipeline and tbb::flow_graph

## Prerequisites

- gcc
- cmake
- python3
- libboost-all-dev
- libtbb-dev
- libfmt-dev (optionally for pretty output)

### Compilation

```shell
./compile.sh
```

```
Usage: ./compile.sh [options]
  Options:
    -h      --help                  Show help message.
    -O      --optimize-build        Compile with optimization before executing (default).
    -o      --no-optimize-build     Compile without optimization before executing.
    -D      --debug-build           Compile with debug options.
    -d      --no-debug-build        Compile without debug options.
    -I      --install_prefix        Installation path.
    -R      --remove-build-dirs     Remove build dirs after the install.
```

### Installation

```shell
xargs sudo apt-get install < dependencies/apt.txt
pip install -r dependencies/requirements.txt
```

### Installation

```shell
xargs sudo apt-get install < dependencies/apt.txt
pip install -r dependencies/requirements.txt
```

### Usage

#### C++

```
Usage:
        countwords_par_tbb [-h|--help] <config>
Allowed options::
  -h [ --help ]         Show help message
  -c [ --conf ] arg     Path to configuration file
```

#### Python

- `src/evaluate.py` - is used for automatic launches of words counting multiple times

```
usage: evaluate.py [-h] [-c CONFIG] [-b EXEC] [-o OUT] times flush

positional arguments:
  times                 times to repeat evaluation
  flush                 buffers flushing 0 - no, 1 - yes

options:
  -h, --help            show this help message and exit
  -c CONFIG, --config CONFIG
                        path to a config file
  -b EXEC, --bin-executable EXEC
                        path to an executable
  -o OUT, --output OUT  output file for all evaluations
```

### Important!

All optional arguments in python script have default values
that are suitable for project configuration:

- Default path to a config file is `data/config.cfg`
- Default path to an executable is `bin/countwords_par_tbb`
- Default path to a result file is `data/times.csv`

Python script does not work for executable compiled in `DEBUG` mode.

Default compilation without using any flags will result
in program using `tbb::flow_graph` in its implementation. Adding
an option `-DCOUNT_WORDS_PIPELINE=ON` to cmake will cause the
program to use `tbb::parallel_pipeline` in its implementation.

The project can use the `fmt` library to format the result
files if CMake can find the package. Otherwise, `setw` will be
used, which can cause the wrong displaying of data, as some
Unicode characters require several bytes to be represented that
are not considered in `setw`.

Compilation via cmake will place the executable in a `bin` folder.

### Results

I have started my program using the config `data/config.cfg`. 
Results of the execution for both modes can be accessed
seen [here](https://drive.google.com/file/d/1HyjaR4LEQ794GwWnM6VtqLiIkYchII9V/view?usp=sharing) 
sorted by alphabet
and [here](https://drive.google.\com/file/d/1crg6x8gs8fTpOPn6zTdEhVzvHLTCEKDN/view?usp=sharing) 
sorted by number of occurrence (they are the same as previous lab, 
checked using `diff` util). Timings for launch using tbb::flow_graph are:

```
Total=1047504 (17.5 mins)
Finding=72363 (1.2 mins)
Reading=225172 (3.8 mins)
Writing=33221 (33.2 secs)
```

Timings for launch using tbb::parallel_pipeline are:

```
Total=1105124 (18.4 mins)
Finding=31007 (31.0 secs)
Reading=121199 (2.0 mins)
Writing=31806 (31.8 secs)
```

Worth to mention that the way of calculating finding and reading times has 
changed since last lab. Each function now calculates its execution time and 
add it to the global variable. Using this approach waiting time is not 
counted as it did in previous labs.

That is also noticeable that both implementation are faster than 
implementation using low-level concurrent queue. This is easily explainable, 
as using thread_queue we can only delegate some exact task for each thread, 
but using high-level tools it is up to them to dispatch task evenly across 
all available threads. As a result program that uses queue at some point 
not fully utilizes all the resources, but both flow_graph and 
parallel_pipeline do, resulting in faster execution.
