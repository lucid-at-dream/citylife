project('citylife', 'c')
cc = meson.get_compiler('c')

### Build citylife-base

citylife_base_deps = [
    dependency('threads'),
    cc.find_library('m', required : true)
]

source_files = run_command('find', 'src/base-libs', '-name', '*.c').stdout().strip().split('\n')
include_dirs = run_command('find', 'src/base-libs', '-name', 'include').stdout().strip().split('\n')

citylife_base_lib = library(
    'citylife-base',
    source_files,
    dependencies: citylife_base_deps,
    include_directories: include_dirs,
    install: true
)

citylife_base_dep = declare_dependency(
    include_directories : include_dirs,
    link_with : citylife_base_lib
)

### Test citylife-base

test_files = run_command('find', 'test/base-libs', '-name', '*.c', '-printf', '%p %f\n').stdout().strip().split('\n')

foreach p : test_files
    filename = p.split(' ')[0]
    testname = p.split(' ')[1].split('.')[0]

    exe = executable(
        testname, filename, 
        link_with: citylife_base_lib,
        dependencies: [citylife_base_dep, citylife_base_deps]
    )

    test(testname, exe)
endforeach


### Build citylife-core

citylife_core_deps = []

source_files = run_command('find', 'src/core-libs', '-name', '*.c').stdout().strip().split('\n')
include_dirs = run_command('find', 'src/core-libs', '-name', 'include').stdout().strip().split('\n')

citylife_core_lib = library(
    'citylife-core',
    source_files,
    dependencies: citylife_core_deps,
    include_directories: include_dirs,
    install: true
)

citylife_core_dep = declare_dependency(
    include_directories : include_dirs,
    link_with : citylife_core_lib
)

### Test citylife-core

test_files = run_command('find', 'test/core-libs', '-name', '*.c', '-printf', '%p %f\n').stdout().strip().split('\n')

foreach p : test_files
    filename = p.split(' ')[0]
    testname = p.split(' ')[1].split('.')[0]

    exe = executable(
        testname, filename, 
        link_with: [citylife_base_lib, citylife_core_lib],
        dependencies: [citylife_base_dep, citylife_core_dep, citylife_core_deps]
    )

    test(testname, exe)
endforeach


