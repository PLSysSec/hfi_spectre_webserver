#!/usr/bin/env python3
import sys
import os
import argparse
import math

ARGPARSER = argparse.ArgumentParser()
ARGPARSER.add_argument('-folders',
    nargs="+",
    help='folders containing .txt results files to import')

configurations = ["stock", "spectre_sfi_aslr", "spectre_sfi_full", "spectre_cet_aslr", "spectre_cet_full"]
num_configurations = len(configurations)
metrics = ["avg_lat_microseconds", "tail_lat_microseconds", "throughput"]
num_metrics = len(metrics)

def parse_results(folders):
    results = {config: {} for config in configurations}
    for folder in folders:
        for file in os.listdir(folder):
            for config in configurations:
                suffix = "_" + config + ".txt"
                if file.endswith(suffix):
                    workload = file[:file.rfind(suffix)]
                    results[config][workload] = parse_result_from_file(os.path.join(folder, file))
    return results

def parse_result_from_file(f):
    result = {}
    with open(f, 'r') as f:
        f.readline()  # ignore first line
        for metric in metrics:
            result[metric] = float(f.readline())
    return result

def format_num(num, levels_deep=0):
    if num < 10:
        return f'{num:.2f}', 0
    elif num < 100:
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

    results = parse_results(args.folders)

    workloads = [w for w in results[configurations[0]]]
    num_workloads = len(workloads)
    num_metrics = len(results[configurations[0]][workloads[0]])

    # print workloads
    print('\\multirow{2}{1cm}{Protection} ')
    for w in workloads:
        if w == "msghash_check_c":
            w = "\\cdnHash"
        elif w == "html_template":
            w = "\\cdnTemplatedHTML"
        elif w == "xml_to_json":
            w = "\\cdnXMLtoJSON"
        elif w == "jpeg_resize_c":
            w = "\\cdnJpgQuality"
        elif w == "tflite":
            w = "\\cdnML"
        print(' & \\multicolumn{' + str(num_metrics) + '}{c|}{' + w.replace('_', '\_') + '}')
    print('\\\\\\cline{2-'+ str(num_workloads * num_metrics + 1) + '}\n')
    # print metric statistics
    for w in workloads:
        for m in metrics:
            if m == "avg_lat_microseconds":
                print(' & Avg Lat ', end='\t')
            elif m == "tail_lat_microseconds":
                print(' & Tail Lat ', end='\t')
            elif m == "throughput":
                print(' & Thru-put ', end='\t')
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
            for m in metrics:
                lres = float(results[r][w][m])
                form, dep = format_num(lres)
                latency_suffixes = ['us', 'ms', 's']
                thruput_suffixes = ['', 'k', 'm']

                if m == "avg_lat_microseconds" or m == "tail_lat_microseconds":
                    print_padded(f' & {form} {latency_suffixes[dep]}', 15)
                elif m == "throughput":
                    print_padded(f' & {form}{thruput_suffixes[dep]}', 15)
                else:
                    raise ValueError("unknown unit for metric " ++ m)
            print('')
        print('\\\\\\hline\n')

sys.exit(main(sys.argv))
