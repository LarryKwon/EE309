#!/usr/bin/env python3
from tests.test import Test, weight
import random
import string

TEST_PLAIN_FILE_NAMES = ["plain-file-1", "plain-file-2", "plain-file-3"]
TEST_BIN_FILE_NAMES = ["bin-file-1", "bin-file-2"]

@weight(15)
class Test(Test):
    def __init__(self, root_dir, bin_path, ref_bin_path):
        super().__init__(root_dir, bin_path, ref_bin_path)

        random.seed(13100)
        for name in TEST_PLAIN_FILE_NAMES:
            size = random.randint(0x100, 0x1000)
            plain_payload = self.random_string(size)
            self.create_file(name, plain_payload.encode())
        for name in TEST_BIN_FILE_NAMES:
            size = random.randint(0x100, 0x1000)
            self.create_file(name, self.random_bytes(size))
        return

    def run(self):
        result = self.run_procs_and_diff(TEST_PLAIN_FILE_NAMES + TEST_BIN_FILE_NAMES, self.test_dir)
        return not result, result