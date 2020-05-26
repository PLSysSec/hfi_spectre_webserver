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
                    if results[r][w][s][m] < 100:
                        print(f' & {results[r][w][s][m]:.2f}', end='\t')
                    elif results[r][w][s][m] < 1000:
                        print(f' & {results[r][w][s][m]:.1f}', end='\t')
                    else:
                        print(f' & {results[r][w][s][m]:.0f}', end='\t')
                else:
                    stddev = int(results[r][w][s][m])
                    avg = int(results[r][w][s]['average'])
                    rel_stddev = stddev/avg if avg is not 0 else 0
                    print(' & {:.1%}'.format(rel_stddev), end='\t')


        print('\\\\\\hline')

sys.exit(main(sys.argv))
