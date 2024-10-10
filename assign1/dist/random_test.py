#!/usr/bin/env python3

import os
import random
import subprocess

# 랜덤으로 선택할 파일 또는 디렉터리의 개수
num_samples = 1000000

# 현재 디렉터리에서 랜덤하게 파일 또는 디렉터리를 선택하는 함수
def get_random_paths(base_path, num_samples=5):
    all_entries = []
    
    # base_path에서 모든 파일과 디렉터리를 수집
    for root, dirs, files in os.walk(base_path):
        for name in dirs + files:
            all_entries.append(os.path.join(root, name))
    
    # 무작위로 num_samples 개의 파일/디렉터리를 선택
    return random.sample(all_entries, min(num_samples, len(all_entries)))

# 두 프로그램을 실행하고 출력 비교하는 함수
def compare_programs(paths):
    for path in paths:
        try:
            # ls209 실행
            result_ls209 = subprocess.run(['./ls309', '-alR', path], capture_output=True, text=True)
            
            # sample309 실행
            result_sample309 = subprocess.run(['./samplels309', '-alR', path], capture_output=True, text=True)
            
            # 출력 결과 비교
            if result_ls209.stdout == result_sample309.stdout:
                print(f"✅ 동일한 출력: {path}")
            else:
                print(f"❌ 출력이 다름: {path}")
                print("ls309 출력:")
                print(result_ls209.stdout)
                print("sample309 출력:")
                print(result_sample309.stdout)
                break
            
            # 메모리 누수 검사 (ls209)
            # print(f"\n🔍 {path} 에서 ls309 메모리 누수 검사:")
            # memcheck_ls209 = subprocess.run(['valgrind', '--leak-check=full', '--error-exitcode=1', './ls209', '-alR', path], capture_output=True, text=True)
            # if "definitely lost" in memcheck_ls209.stderr:
            #     print("❗ 메모리 누수가 발견되었습니다.")
            #     print(memcheck_ls209.stderr)
            #     break
            # else:
            #     print("✅ 메모리 누수가 발견되지 않았습니다.")
        
        except Exception as e:
            print(f"에러 발생: {e}")

if __name__ == "__main__":
    # base_path는 비교할 파일 및 디렉터리를 찾을 기준 경로
    base_path = '/'  # 현재 디렉터리에서 찾는 경우
    random_paths = get_random_paths(base_path, num_samples)
    compare_programs(random_paths)