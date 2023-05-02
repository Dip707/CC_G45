from sys import argv
from os import system
import subprocess

(dir, cases) = ('func', 23) if len(argv) >= 2 and argv[1] == 'f' else ('no_func', 19)

system('make compiler')
system('clang++ -c runtime/runtime_lib.cc -o obj/runtime_lib.o')
for i in range(1, cases + 1):
    name = 'tests/' + dir + '/test' + str(i) + '.be'

    print(f'===== Test {i} =====')

    if i in [17, 18, 19]:
        system(f'./bin/base {name} -p')
    else:
        system(f'./bin/base {name} -o bin/test.bc')
        #system('make program')
        system('llc -filetype=obj bin/test.bc -o obj/test.o')
        system('clang++ obj/test.o obj/runtime_lib.o -o bin/test')
        system('./bin/test')
        system('rm bin/test.bc obj/test.o bin/test 2> /dev/null')