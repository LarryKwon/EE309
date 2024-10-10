#!/usr/bin/env python3

import os
import itertools
import random
import subprocess

# 사용할 옵션들
options = ['-a', '-l', '-R']

# 랜덤 시드 설정
def set_random_seed(seed_value=42):
    random.seed(seed_value)

# 현재 디렉터리에서 모든 파일 또는 디렉터리를 수집하는 함수
def get_all_paths(base_path, num_samples=20):
    all_entries = []
    
    # base_path에서 모든 파일과 디렉터리를 수집
    for root, dirs, files in os.walk(base_path):
        for name in dirs + files:
            all_entries.append(os.path.join(root, name))
    
    # 경로를 임의로 num_samples개 선택
    return random.sample(all_entries, min(num_samples, len(all_entries)))

# 파일/디렉터리와 옵션의 모든 가능한 조합을 생성하는 함수
def get_all_command_combinations(paths, max_length=10, num_samples=10):
    combinations = []
    for length in range(1, max_length + 1):
        for combo in itertools.combinations(paths, length):
            combo_list = list(combo)
            # 옵션을 조합에 추가하고 위치를 랜덤하게 삽입
            for _ in range(num_samples):
                random_options = random.sample(options, random.randint(1, len(options)))
                combined = combo_list + random_options
                random.shuffle(combined)
                combinations.append(combined)
    return combinations

# 두 프로그램을 실행하고 출력 비교하는 함수
def compare_programs(combinations):
    print(len(combinations))
    for combination in combinations:
        try:
            # 명령어와 인자를 조합
            command_ls209 = ['./ls309'] + combination
            command_sample309 = ['./samplels309'] + combination
            
            print(f"path, {' '.join(combination)}")
            # ls209 실행
            result_ls209 = subprocess.run(command_ls209, capture_output=True, text=True)
            
            # sample309 실행
            result_sample309 = subprocess.run(command_sample309, capture_output=True, text=True)
            
            # 출력 결과 비교
            if result_ls209.stdout == result_sample309.stdout:
                print(f"✅ 동일한 출력: {' '.join(combination)}")
            else:
                print(f"❌ 출력이 다름: {' '.join(combination)}")
                print("ls309 출력:", flush=True)
                print(result_ls209.stdout, flush=True)
                print("sample309 출력:", flush=True)
                print(result_sample309.stdout, flush=True)
                
            
            # 메모리 누수 검사 (ls209)
            print(f"🔍 {' '.join(combination)} 에서 ls309 메모리 누수 검사:")
            memcheck_ls209 = subprocess.run(['valgrind', '--leak-check=full', '--error-exitcode=1', './ls309'] + combination, capture_output=True, text=True)
            if "definitely lost" in memcheck_ls209.stderr:
                print("❗ 메모리 누수가 발견되었습니다.\n", flush=True)
                print(memcheck_ls209.stderr, flush=True)
                break
            else:
                print("✅ 메모리 누수가 발견되지 않았습니다.\n", flush=True)
        
        except Exception as e:
            print(f"에러 발생: {e}", flush=True)

if __name__ == "__main__":
    # 랜덤 시드 설정
    set_random_seed()
    
    # base_path는 비교할 파일 및 디렉터리를 찾을 기준 경로
    base_path = '/'  # 현재 디렉터리에서 찾는 경우
    random_paths = get_all_paths(base_path)
    all_combinations = get_all_command_combinations(random_paths)
    compare_programs(all_combinations)