#!/usr/bin/env python3
from tests.test import Test, weight
import random

TEST_DIR_NAME = "single-recursive-dir"
TEST_FILE_NAMES = ["third-recursive-file", "first-recursive-file", "second-recursive-file"]

@weight(15)
class Test(Test):
    def __init__(self, root_dir, bin_path, ref_bin_path):
        super().__init__(root_dir, bin_path, ref_bin_path)

        random.seed(13107)
        self.create_dir(TEST_DIR_NAME)
        for name in TEST_FILE_NAMES:
            size = random.randint(0x100, 0x1000)
            self.create_file_with_dir(name, self.random_bytes(size), TEST_DIR_NAME)
        return

    def run(self):
        result = self.run_procs_and_diff(["-R"], self.test_dir)
        return not result, result