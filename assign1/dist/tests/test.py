#!/usr/bin/env python3
from functools import wraps
import shutil
import os
import subprocess
import itertools
import pathlib
import string
import random

TEST_DIR = "ee309_test/"

def weight(score):
    def wrapper(clss):
        setattr(clss, 'weight', score)
        return clss
    return wrapper

class Test():
    def __init__(self, root_dir, bin_path, ref_bin_path):
        # setup test dir var
        self.test_dir = pathlib.Path(root_dir) / TEST_DIR

        # setup the test directory
        if os.path.isdir(self.test_dir):
            try:
                shutil.rmtree(self.test_dir)
            except OSError as e:
                print("failed to clear test dir : {}".format(e))
                exit(-1)

        try:
            os.makedirs(self.test_dir)
        except OSError as e:
            print("failed to create test dir : {}".format(e))
            exit(-1)

        # setup bin/ref bin path var
        self.bin_path = bin_path
        self.ref_bin_path = ref_bin_path

        return
    
    def random_string(self, length):
        charset = string.ascii_uppercase + string.ascii_lowercase + string.digits
        return ''.join(random.choice(charset) for _ in range(length))

    def random_bytes(self, length):
        return bytes([random.randint(0, 255) for _ in range(length)])

    def create_file(self, name, content):
        with open(self.test_dir / name, "wb") as f:
            f.write(content) 

    def create_dir(self, name):
        os.mkdir(self.test_dir / name)

    def create_file_with_dir(self, name, content, d):
        path = pathlib.Path(d) / name
        self.create_file(path, content)

    def create_dir_with_dir(self, name, d):
        path = pathlib.Path(d) / name
        self.create_dir(path)

    def create_symlink(self, src, dst):
        os.symlink(src, self.test_dir / dst)

    def compare_long(self, l1, l2):
        if len(l1) != len(l2):
            return False
        if len(l1) < 44:
            return l1 == l2
        
        l1_notime = l1[:44] + l1[57:]
        l2_notime = l2[:44] + l2[57:]

        return l1_notime == l2_notime

    def run_proc(self, args, test_cwd, is_long = False, capture_stderr = False):
        res = subprocess.run(args, check=False, capture_output=True, cwd=test_cwd)

        if capture_stderr:
            return res.stderr
        else:
            return res.stdout

    def run_procs_and_diff(self, args, test_cwd, is_long = False, capture_stderr = False, debug = False):
        res = []

        p1 = self.run_proc([str(self.bin_path)] + args, test_cwd, is_long, capture_stderr)
        p2 = self.run_proc([str(self.ref_bin_path)] + args, test_cwd, is_long, capture_stderr)

        if debug:
            print(p1.decode())
            print(p2.decode())

        for l1, l2 in itertools.zip_longest(p1.split(b"\n"), p2.split(b"\n"), fillvalue = None):
            if l1 == None or l2 == None:
                res.append((l1, l2))
                continue

            if (is_long and not self.compare_long(l1, l2)) or (not is_long and l1 != l2):
                res.append((l1, l2))
        return res


