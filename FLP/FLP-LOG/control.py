import os
import subprocess

def run_prolog_on_files():
    input_folder = 'tests'
    file_extension = '.in'
    prolog_executable = './flp22-log'
    
    for file in os.listdir(input_folder):
        if file.endswith(file_extension):
            input_file_path = os.path.join(input_folder, file)
            temp_file_path = os.path.splitext(input_file_path)[0] + '.temp'
            out_file_path = os.path.splitext(input_file_path)[0] + '.out'

            with open(input_file_path, 'r') as in_file, open(temp_file_path, 'w') as temp_file:
                subprocess.run([prolog_executable], stdin=in_file, stdout=temp_file)

            with open(temp_file_path, 'r') as temp_file, open(out_file_path, 'r') as out_file:
                temp_lines = temp_file.readlines()
                out_lines = out_file.readlines()

                if temp_lines == out_lines:
                    print(f'Test {input_file_path}: OK')
                else:
                    print(f'Test {input_file_path}: FAILED')

if __name__ == '__main__':
    run_prolog_on_files()
