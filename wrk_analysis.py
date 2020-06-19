#!/usr/bin/env python3
import sys
import os
import argparse
import math

ARGPARSER = argparse.ArgumentParser()
ARGPARSER.add_argument('-folders',
    nargs="+",
    help='folders containing .txt results files to import')
ARGPARSER.add_argument('-sofolder',
    required=True,
    help='folder containing .so files corresponding to the results files')

configurations = ["stock", "spectre_sfi_aslr", "spectre_sfi_full", "spectre_cet_aslr", "spectre_cet_full"]
num_configurations = len(configurations)
metrics = ["avg_lat_microseconds", "tail_lat_microseconds", "throughput", "bin_size"]
num_metrics = len(metrics)

# If workloads with these names are found, they go in a separate table below the main table
workloads_to_go_in_bottom_table = ["jpeg_resize_c", "tflite"]

# given a config name, return the string to put in the LaTeX for it
def latex_name_of_config(config):
    if config == "stock":
        return "Stock --- unsafe"
    elif config == "spectre_sfi_aslr":
        return "\\sysDesignOne ASLR"
    elif config == "spectre_sfi_full":
        return "\\sysDesignOne Det"
    elif config == "spectre_cet_aslr":
        return "\\sysDesignTwo ASLR"
    elif config == "spectre_cet_full":
        return "\\sysDesignTwo Det"
    else:
        return config.replace('_', '\_')

# given a workload name, return the string to put in the LaTeX for it
def latex_name_of_workload(workload):
    if workload == "msghash_check_c":
        return "\\cdnHash"
    elif workload == "html_template":
        return "\\cdnTemplatedHTML"
    elif workload == "xml_to_json":
        return "\\cdnXMLtoJSON"
    elif workload == "jpeg_resize_c":
        return "\\cdnJpgQuality"
    elif workload == "tflite":
        return "\\cdnML"
    elif workload == "echo_server":
        return "\\cdnEcho"
    else:
        return workload.replace('_', '\_')

# given a metric name, return the string to put in the LaTeX for it
def latex_name_of_metric(metric):
    if metric == "avg_lat_microseconds":
        return 'Avg Lat'
    elif metric == "tail_lat_microseconds":
        return 'Tail Lat'
    elif metric == "throughput":
        return 'Thru-put'
    elif metric == "bin_size":
        return 'Bin size'
    else:
        return metric.replace('_', '\_')

def parse_results(folders, so_folder):
    results = {config: {} for config in configurations}
    for folder in folders:
        for file in os.listdir(folder):
            for config in configurations:
                suffix = "_" + config + ".txt"
                if file.endswith(suffix):
                    workload = file[:file.rfind(suffix)]
                    results[config][workload] = parse_result_from_file(so_folder, os.path.join(folder, file))
    return results

def parse_result_from_file(so_folder, filename):
    result = {}
    with open(filename, 'r') as f:
        f.readline()  # ignore first line
        for metric in metrics:
            if metric == "bin_size":
                # bin_size is special
                result[metric] = get_bin_size_bytes(so_folder, filename)
            else:
                result[metric] = float(f.readline())
    return result

def get_bin_size_bytes(so_folder, results_filename):
    (stem, ext) = os.path.splitext(os.path.basename(results_filename))
    assert(ext == ".txt")
    so_filename = stem + ".so"
    so_path = os.path.join(so_folder, so_filename)
    return os.stat(so_path).st_size

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

def print_table(results, workloads):
    num_workloads = len(workloads)
    if num_configurations == 0:
        raise ValueError("no configurations")
    if num_workloads == 0:
        raise ValueError("no workloads")
    if num_metrics == 0:
        raise ValueError("no metrics")

    # print table header
    cell_size = '0.55cm'
    print('\\footnotesize')
    print('\\begin{tabular}{p{2.2cm}}')
    for _ in range(num_workloads):
        print('  |', end='')
        for _ in range(num_metrics):
            print('p{' + cell_size + '}', end='')
        print('')
    print('')

    # print workloads
    print('\\multirow{2}{1cm}{Protection} ')
    for w in workloads:
        print(' & \\multicolumn{' + str(num_metrics) + '}{c|}{' + latex_name_of_workload(w) + '}')
    print('\\\\\\cline{2-'+ str(num_workloads * num_metrics + 1) + '}\n')

    # print metric headers
    for w in workloads:
        for m in metrics:
            print(' & ' + latex_name_of_metric(m), end='\t')
        print('')
    print('\\\\\\hline\n')

    # print data
    for r in configurations:
        print(latex_name_of_config(r))
        for w in workloads:
            for m in metrics:
                lres = float(results[r][w][m])
                form, dep = format_num(lres)
                latency_suffixes = ['us', 'ms', 's']
                thruput_suffixes = ['', 'k', 'm']
                binsize_suffixes = ['B', 'KB', 'MB', 'GB']

                if m == "avg_lat_microseconds" or m == "tail_lat_microseconds":
                    print_padded(f' & {form}{latency_suffixes[dep]}', 15)
                elif m == "throughput":
                    print_padded(f' & {form}{thruput_suffixes[dep]}', 15)
                elif m == "bin_size":
                    print_padded(f' & {form}{binsize_suffixes[dep]}', 15)
                else:
                    raise ValueError("unknown unit for metric " + m)
            print('')
        print('\\\\\\hline\n')

    # print footer
    print('\\end{tabular}')

def main(args):
    args = ARGPARSER.parse_args()

    results = parse_results(args.folders, args.sofolder)

    workloads = [w for w in results[configurations[0]]]

    bottom_workloads = [w for w in workloads if w in workloads_to_go_in_bottom_table]
    top_workloads = [w for w in workloads if w not in workloads_to_go_in_bottom_table]

    print_table(results, top_workloads)
    if len(bottom_workloads) > 0:
        print('\n\\vspace{1em}\n')
        print_table(results, bottom_workloads)

sys.exit(main(sys.argv))
