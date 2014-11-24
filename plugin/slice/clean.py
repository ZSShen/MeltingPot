#!/usr/bin/python


import os;
import sys;
import argparse;
import shutil;


def main():

    parser = argparse.ArgumentParser();
    parser.add_argument("--rebuild", dest = "rebuild", action = "store_true");
    parser.add_argument("--no-rebuild", dest = "rebuild", action = "store_false");
    parser.set_defaults(rebuild = False);

    args = parser.parse_args();
    rebuild = args.rebuild;

    # Recursivly clean all the binary directories.
    list_build = list();
    list_lib = list();
    path_cur = os.getcwd();
    for path_dir, list_dir, list_file in os.walk(path_cur):
        if path_dir.endswith("build"):
            list_build.append(path_dir);
        elif path_dir.endswith("lib"):
            list_lib.append(path_dir);

    for path_build in list_build:
        shutil.rmtree(path_build);
    for path_lib in list_lib:
        shutil.rmtree(path_lib);

    # Create the build folder if necessary.
    if rebuild == False:
        return;
    for path_dir, list_dir, list_file in os.walk(path_cur):
        if path_dir.endswith("src"):
            path_build = path_dir.replace("src", "build");
            os.makedirs(path_build);
        elif path_dir.endswith("plugin"):
            path_build = os.path.join(path_dir, "build");
            os.makedirs(path_build);
        elif path_dir.endswith("engine"):
            path_build = os.path.join(path_dir, "build");
            os.makedirs(path_build);

    return;


if __name__ == "__main__":
    main();
