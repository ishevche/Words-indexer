#! /usr/bin/env python3
import argparse
import re
import subprocess
import sys

import numpy as np
import pandas as pd


def get_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("times", help="times to repeat evaluation", type=int)
    parser.add_argument("flush", help="buffers flushing 0 - no, 1 - yes", type=int)
    parser.add_argument("-c", "--config", help="path to a config file",
                        dest="config", default="data/config.cfg"),
    parser.add_argument("-b", "--bin-executable", help="path to an executable",
                        dest="exec", default="bin/countwords_par_tbb")
    parser.add_argument("-o", "--output", help="output file for all evaluations",
                        dest="out", default="data/times.csv")
    return parser.parse_args()


def main(executable, config_file, times_to_repeat, flush, out_path, verbose=True):
    words_file = get_words_file(config_file)
    count_times = []
    find_times = []
    read_times = []
    write_times = []
    answer = None
    for i in range(times_to_repeat):
        flush_buffers(flush)
        count_time, find_time, read_time, write_time, result = \
            run_method(executable, config_file, words_file)
        if answer is None:
            answer = result
        elif answer != result:
            print("Not all results are the same :(")
            return
        count_times.append(count_time)
        find_times.append(find_time)
        read_times.append(read_time)
        write_times.append(write_time)
    if verbose:
        output_times(count_times, find_times, read_times, write_times, out_path)
    return count_times, find_times, read_times, write_times


def get_words_file(config_file):
    """
    This function is created to parse config_file and get the result file
    where the word map is saved, but modules designed to parse TOML files
    could not parse config_file, because allowed extensions are not in
    brackets.
    """
    return "data/res_a.txt"


def flush_buffers(flush):
    if flush:
        subprocess.Popen(["sync"]).wait()
        subprocess.Popen(["sh -c '/bin/echo 3 > /proc/sys/vm/drop_caches'"], shell=True).wait()


def run_method(executable, config_file, words_file):
    subprocess_result = subprocess.run([executable, config_file], capture_output=True)
    if subprocess_result.returncode:
        print(subprocess_result.stderr.decode(), file=sys.stderr, end='')
        exit(subprocess_result.returncode)
    output = parse_output(subprocess_result.stdout.decode())
    words = read_words_map(words_file)
    return *output, words


def read_words_map(words_file):
    words = {}
    with open(words_file, encoding="utf-8") as subprocess_result:
        for line in subprocess_result.readlines():
            line_match = re.match(r'^(.+?)\s+(\d+)$', line)
            words[line_match.group(1)] = line_match.group(2)
    return words


def parse_output(subprocess_result):
    count_words_time, find_result_time, read_result_time, write_result_time = \
        subprocess_result.split()
    count_match = re.match(r'^Total=(\d+)$', count_words_time)
    find_match = re.match(r'^Finding=(\d+)$', find_result_time)
    read_match = re.match(r'^Reading=(\d+)$', read_result_time)
    write_match = re.match(r'^Writing=(\d+)$', write_result_time)
    if not count_match or not write_match:
        raise ValueError("Wrong output format")
    count_time = int(count_match.group(1))
    find_time = int(find_match.group(1))
    read_time = int(read_match.group(1))
    write_time = int(write_match.group(1))
    return count_time, find_time, read_time, write_time


def output_times(count_times, find_times, read_times, write_times, out_path):
    def print_stats(times):
        print(min(times))
        print(int(np.mean(times)))
        print(int(np.std(times, ddof=1)))
        print()

    print_stats(count_times)
    print_stats(find_times)
    print_stats(read_times)
    print_stats(write_times)
    df = pd.DataFrame.from_dict({'Count times': count_times,
                                 'Find times': find_times,
                                 'Read times': read_times,
                                 'Writing times': write_times})
    df.to_csv(out_path, index=False)


if __name__ == '__main__':
    args = get_arguments()
    main(args.exec, args.config, args.times, args.flush, args.out)
