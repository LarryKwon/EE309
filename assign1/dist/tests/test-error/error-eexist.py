#!/usr/bin/env python3
from tests.test import Test, weight

TEST_FILE_NAME = "not-exist-file"
ERROR_MSG = "No such file or directory"

@weight(15)
class Test(Test):
    def __init__(self, root_dir, bin_path, ref_bin_path):
        super().__init__(root_dir, bin_path, ref_bin_path)
        return

    def run(self):
        result = self.run_proc([self.bin_path, TEST_FILE_NAME], self.test_dir, capture_stderr = True)
        return ERROR_MSG in result.decode(), result
