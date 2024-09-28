#!/usr/bin/env python3
from tests.test import Test, weight
import random

TEST_DIR_NAMES = ["third-dir", "first-dir", "second-dir"]

@weight(15)
class Test(Test):
    def __init__(self, root_dir, bin_path, ref_bin_path):
        super().__init__(root_dir, bin_path, ref_bin_path)

        random.seed(13102)
        for name in TEST_DIR_NAMES:
            self.create_dir(name)
            file_cnt = random.randint(0, 0x10)
            file_tags = random.sample(range(0, 0x100), file_cnt)
            for tag in file_tags:
                size = random.randint(0x100, 0x1000)
                self.create_file_with_dir("file-{}".format(tag), self.random_bytes(size), name)
        return

    def run(self):
        result = self.run_procs_and_diff(TEST_DIR_NAMES, self.test_dir)
        return not result, result
