#!/usr/bin/env python3
from tests.test import Test, weight
import random

TEST_DIR_NAMES = ["third-dir", "first-dir", "second-dir"]
TEST_FILE_NAMES = ["1st_file", "2nd_file", "3rd_file"]

@weight(15)
class Test(Test):
    def __init__(self, root_dir, bin_path, ref_bin_path):
        super().__init__(root_dir, bin_path, ref_bin_path)

        random.seed(13103)
        for name in TEST_DIR_NAMES:
            self.create_dir(name)
            file_cnt = random.randint(0, 0x10)
            for _ in range(file_cnt):
                size = random.randint(0x100, 0x1000)
                self.create_file_with_dir(self.random_string(0x10), self.random_bytes(size), name)

        for name in TEST_FILE_NAMES:
            size = random.randint(0x100, 0x1000)
            self.create_file(name, self.random_bytes(size))

        return

    def run(self):
        random.seed(13104)
        args = TEST_DIR_NAMES + TEST_FILE_NAMES
        random.shuffle(args)
        result = self.run_procs_and_diff(args, self.test_dir)
        return not result, result