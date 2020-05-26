#!/usr/bin/env python3
import sys
import os
import json
import argparse
import math

ARGPARSER = argparse.ArgumentParser()
ARGPARSER.add_argument('-file',
    nargs='+',
    help='Json file(s) to be imported')
ARGPARSER.add_argument('-statistic',
    nargs='*',
    help='Values requests, latency, throughput',
    default=['throughput'])
ARGPARSER.add_argument('-metric',
    nargs='*',
    help='Values include average, mean, stddev, min, max, percentile (e.g. p90, p99_9)',
    default=['average'])

def parse_results(files):
    results = {}
    for file in files:
        with open(file, 'r') as f:
            ac_result = json.load(f)

        # remove first order of json mumbo
        results.update(ac_result)

    return results


def format_num(num):
    if num < 10:
        return f'{num:.2f}', 0
    elif num < 100:
        return f'{num:.1f}', 0
    elif num < 1000:
        return f'{num:.0f}', 0
    else:
        res, dep = format_num(num/1000)
        return res, int(dep) + 1

def main(args):
    args = ARGPARSER.parse_args()

    if(len(args.metric) != len(args.statistic)):
        ARGPARSER.print_usage()
        print('Error: please provide the same number of arguments to statistic and metric')
        sys.exit(1)

    results = parse_results(args.file)

    configurations = [c for c in results]
    num_configurations = len(configurations)
    workloads = [w for w in results[configurations[0]]]
    num_workloads = len(workloads)

    # print workloads
    print('Configuration ', end='')
    for w in workloads:
        print(' & \\multicolumn{' + str(len(args.metric)) + '}{c}{' + w + '}', end='')
    print('\\\\\\hline')
    # print metric statistics
    for w in workloads:
        for s, m in zip(args.statistic, args.metric):
            print(f' & {s}({m})', end='\t')
    print('\\\\\\hline')

    for r in configurations:
        print(f'{r} ', end='\t')
        for w in workloads:
            for s, m in zip(args.statistic, args.metric):
                if m != 'stddev':
                    lres = float(results[r][w][s][m])
                    form, dep = format_num(lres)
                    k_names = ['', 'k', 'm']
                    print(f' & {form}{k_names[dep]}', end='\t')
                else:
                    stddev = int(results[r][w][s][m])
                    avg = int(results[r][w][s]['average'])
                    rel_stddev = stddev/avg if avg is not 0 else 0
                    print(' & {:.1%}'.format(rel_stddev), end='\t')


        print('\\\\\\hline')

sys.exit(main(sys.argv))
