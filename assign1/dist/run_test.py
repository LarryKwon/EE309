#!/usr/bin/env python3
import pathlib
import importlib
import json
import sys

BIN_PATH = "ls309"
REF_BIN_PATH = "samplels309"
TESTS_ROOT_PATH = "tests"
TESTS_JSON_PATH = "tests.json"

class Grader():
    def __init__(self):
        self.bin_path = pathlib.Path(BIN_PATH).resolve()
        assert(self.bin_path.exists())
        self.ref_bin_path = pathlib.Path(REF_BIN_PATH).resolve()
        assert(self.ref_bin_path.exists())
        self.test_root_path = pathlib.Path(pathlib.Path(TESTS_ROOT_PATH).resolve())
        assert(self.test_root_path.exists())

        with open(self.test_root_path / TESTS_JSON_PATH, "r") as f:
            self.tests_json = json.loads(f.read())

        self.script_root_path = pathlib.Path(__file__).parent.absolute()
        assert(self.script_root_path.exists())

    def run(self):
        result_dict = {}
        for test in self.tests_json["tests"]:
            print("[*] Running test {} :".format(test["test_name"])) 
            result_dict[test["test_name"]] = {}
            for test_case in test["test_cases"]:
                print("    - Running test cases {}......".format(test_case["test_case_name"]), flush=True, end = "")
                try:
                    res = importlib.import_module("{}.{}.{}".format(
                        TESTS_ROOT_PATH, test["test_name"], test_case["test_case_name"]))
                    test_instance = res.Test(self.script_root_path, self.bin_path, self.ref_bin_path)
                    result_dict[test["test_name"]][test_case["test_case_name"]] = test_instance.run()
                    success, diff = result_dict[test["test_name"]][test_case["test_case_name"]]
                    print("O" if success else "X")
                except Exception as e:
                    result_dict[test["test_name"]][test_case["test_case_name"]] = (None, None)
                    print("? ({})".format(type(e).__name__))
                    if type(e).__name__ != "ModuleNotFoundError":
                        print(e)
                    continue
        print("")
        print("******************** Grade Result ********************")
        max_score = sum(map(lambda x:x["test_total_score"], self.tests_json["tests"]))
        total_score = 0
        for test in self.tests_json["tests"]:
            max_test_score = test["test_total_score"]
            total_test_score = 0
            total_test_weight = sum(map(lambda x:x["test_case_weight"], test["test_cases"]))
            test_result_dict = result_dict[test["test_name"]]

            for test_case in test["test_cases"]:
                test_frac = test_case["test_case_weight"] / total_test_weight
                success, _ = test_result_dict[test_case["test_case_name"]]
                total_test_score += (max_test_score * test_frac) if success else 0
            
            print("- {:15s} ({:d}/{:d})".format(test["test_name"], int(total_test_score), max_test_score)) 

            for test_case in test["test_cases"]:
                test_frac = test_case["test_case_weight"] / total_test_weight
                success, _ = test_result_dict[test_case["test_case_name"]]
                max_test_case_score = max_test_score * test_frac
                test_case_score = (max_test_score * test_frac) if success else 0
                result = "Error" if success == None else "Success" if success else "Fail"
                print("    - {:25s} : {} ({:.1f}/{:.1f})".format(test_case["test_case_name"], result,
                                                               test_case_score, max_test_case_score))
            total_score += total_test_score
        print("----------------------------------")
        print("- {:30s} {:d}/{:d}".format("total", int(total_score), max_score))
        print("******************************************************")

        print("*********************** Diffs ************************")
        for test in self.tests_json["tests"]:
            test_result_dict = result_dict[test["test_name"]]
            for test_case in test["test_cases"]:
                success, diff = test_result_dict[test_case["test_case_name"]]
                if success != None and not success:
                    print("Diff result of failed test '{}'".format(test_case["test_case_name"]))
                    for l1, l2 in diff:
                        print("+'{}'".format(l1.decode() if l1 != None else "None"))
                        print("-'{}'".format(l2.decode() if l2 != None else "None"))
                    print("")
        print("******************************************************")

if __name__ == "__main__":
    grader = Grader()
    grader.run()