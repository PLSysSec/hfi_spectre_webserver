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
    default=['latency', 'throughput'])
ARGPARSER.add_argument('-metric',
    nargs='*',
    help='Values include average, mean, stddev, min, max, percentile (e.g. p90, p99_9)',
    default=['p99', 'average'])

def parse_results(files):
    results = {}
    for file in files:
        with open(file, 'r') as f:
            ac_result = json.load(f)

        # remove first order of json mumbo
        results.update(ac_result)

    return results


def format_num(num, levels_deep=0):
    if num < 10 and levels_deep != 0:
        return f'{num:.2f}', 0
    elif num < 100 and levels_deep != 0:
        return f'{num:.1f}', 0
    elif num < 1000:
        return f'{num:.0f}', 0
    else:
        res, dep = format_num(num/1000, levels_deep+1)
        return res, int(dep) + 1

def print_padded(msg, padded_len):
    print(msg.ljust(padded_len), end='')

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
    print('\\multirow{2}{1cm}{Protection} ')
    for w in workloads:
        print(' & \\multicolumn{' + str(len(args.metric)) + '}{c|}{' + w.replace('_', '\_') + '}')
    print('\\\\\\cline{2-'+ str(num_workloads * len(args.metric) + 1) + '}\n')
    # print metric statistics
    for w in workloads:
        for s, m in zip(args.statistic, args.metric):
            if s == "latency" and m == "p99":
                print(' & Tail Lat ', end='\t')
            elif s == "throughput" and (m == "average" or m == "mean"):
                print(' & Thruput ', end='\t')
            else:
                print(f' & {s}({m})'.replace('_', '\_'), end='\t')
        print('')
    print('\\\\\\hline\n')

    for r in configurations:
        if r == "stock":
            print("Stock --- unsafe")
        elif r == "spectre_sfi_aslr":
            print("\\sysDesignOne with ASLR")
        elif r == "spectre_sfi_full":
            print("\\sysDesignOne deterministic (CBP-to-BTB)")
        elif r == "spectre_cet_aslr":
            print("\\sysDesignTwo with ASLR")
        elif r == "spectre_cet_full":
            print("\\sysDesignTwo deterministic (Interlock)")
        else:
            print(f'{r} '.replace('_', '\_'))

        for w in workloads:
            for s, m in zip(args.statistic, args.metric):
                if m != 'stddev':
                    lres = float(results[r][w][s][m])
                    form, dep = format_num(lres)
                    k_names = ['', 'k', 'm']
                    unit = ' ms' if s == "latency" else ''
                    print_padded(f' & {form}{k_names[dep]}{unit}', 15)
                else:
                    stddev = float(results[r][w][s][m])
                    avg = float(results[r][w][s]['average'])
                    rel_stddev = stddev/avg if avg != 0 else 0
                    print_padded(' & {:.1%}'.format(rel_stddev).replace('%', '\%'), 15)
            print('')
        print('\\\\\\hline\n')

sys.exit(main(sys.argv))
