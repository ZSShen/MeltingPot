#!/usr/bin/python

import sys;
import os;
import argparse;


OPT_PATH_INPUT = "input";
HELP_PATH_INPUT = "The root pathname of pattern folder.";
WARN_PATH_INPUT = "The root pathname of pattern folder is not specified.";

SUFFIX_CONTRIBUTE = "Contributed By:";
DECLARE_OFFSET = "Relative Offset";


def main():

    parser = argparse.ArgumentParser();
    parser.add_argument("--%s" % (OPT_PATH_INPUT), help = HELP_PATH_INPUT);
    args = parser.parse_args();
    args = vars(args);

    path_input = args[OPT_PATH_INPUT];
    if path_input == None:
        print WARN_PATH_INPUT;
        return;

    dict_profile = dict();
    collect_contribution_info(path_input, dict_profile);
    print_contribution_info(dict_profile);

    return;


def collect_contribution_info(path_input, dict_profile):

    for path_dir, name_dir, names_file in os.walk(path_input):
        for name_file in names_file:
            path_pattern = os.path.join(path_dir, name_file);
            path_pattern = os.path.abspath(path_pattern);
            if path_pattern not in dict_profile:
                dict_profile[path_pattern] = set();

            h_pattern = open(path_pattern);
            while True:
                line = h_pattern.readline();
                if line == '':
                    break;
                line = line[:-1];
                # Spot the contribution declaration.
                if line.endswith(SUFFIX_CONTRIBUTE):
                    # Collect the contributors.
                    while True:
                        line = h_pattern.readline();
                        if line == '\n':
                            break;
                        if line.find(DECLARE_OFFSET) != -1:
                            continue;
                        line = line[:-1];
                        line = line.replace(" ", "");
                        dict_profile[path_pattern].add(line);

            h_pattern.close();

    return;


def print_contribution_info(dict_profile):

    for path_pattern in dict_profile:
        set_contributor = dict_profile[path_pattern];
        line = "\"%s\",%d" % (path_pattern, len(set_contributor));
        for name_contributor in set_contributor:
            line += ",\"%s\"" % (name_contributor);
        print(line);

    return;


if __name__ == "__main__":
    main();
