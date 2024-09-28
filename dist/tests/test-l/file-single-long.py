#!/usr/bin/env python3
from tests.test import Test, weight

TEST_FILE_NAME = "single-long-file"

@weight(15)
class Test(Test):
    def __init__(self, root_dir, bin_path, ref_bin_path):
        super().__init__(root_dir, bin_path, ref_bin_path)
        self.create_file(TEST_FILE_NAME, "This is the test long file".encode())
        return

    def run(self):
        result = self.run_procs_and_diff(["-l", TEST_FILE_NAME], self.test_dir, is_long = True)
        return not result, result