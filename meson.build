project('citylife', 'c')

cc = meson.get_compiler('c')
add_global_arguments('-g', '-Og', language : 'c')

### Copy test resources to build directory

run_command('mkdir', '-p', meson.build_root() + '/test')
run_command('cp', '-r', 'test/_resources/', meson.build_root() + '/test')




## Build citylife-base

base_deps = [
    dependency('threads'),
    cc.find_library('m', required : true)
]

# Flex source generation
flex = find_program('flex')

flex_src = custom_target(
    'flex_wkt',
    output : ['wkt.lex.yy.c'],
    input : 'src/base-libs/geometry/wkt_parse/well_known_text.l',
    command : [flex, '-o', '@OUTPUT@', '@INPUT@']
)

# Bison source generation
bison = find_program('yacc')

bison_src = custom_target(
    'bison_wkt',
    output : ['wkt.y.tab.c', 'wkt.y.tab.h'],
    input : 'src/base-libs/geometry/wkt_parse/well_known_text.y',
    command : [bison, '@INPUT@', '-o', '@OUTPUT0@', '--defines=@OUTPUT1@']
)


# Find "base-libs" source files
base_source_files = run_command(
    'find', 'src/base-libs', '-name', '*.c', '-and', '-not', '-path', '*/test/*', '-and', '-not', '-path', '*/perf/*'
).stdout().strip().split('\n')

# Find "base-libs" include directories
base_include_dirs = run_command(
    'find', 'src/base-libs', '-name', 'include'
).stdout().strip().split('\n')

base_lib = library(
    'citylife-base',
    base_source_files, flex_src, bison_src,
    dependencies: base_deps,
    include_directories: base_include_dirs,
    install: true
)

base_dep = declare_dependency(
    sources: [bison_src, flex_src],
    include_directories : base_include_dirs,
    link_with : base_lib
)

warning('Base lib built')

## Test citylife-base
test_files = run_command(
    'find', 'src/base-libs', '-name', '*.c', '-and', '-path', '*/test/*', '-printf', '%p %f\n'
).stdout().strip().split('\n')

foreach p : test_files
    filename = p.split(' ')[0]
    testname = p.split(' ')[1].split('.')[0]

    exe = executable(
        testname, filename,
        link_with: base_lib,
        dependencies: [base_dep, base_deps]
    )

    test(testname, exe)
endforeach

warning('Tests built')

## Benchmark citylife-base
test_files = run_command(
    'find', 'src/base-libs', '-name', '*.c', '-and', '-path', '*/perf/*', '-printf', '%p %f\n'
).stdout().strip().split('\n')

foreach p : test_files
    filename = p.split(' ')[0]
    testname = p.split(' ')[1].split('.')[0]

    exe = executable(
        testname, filename, 
        link_with: base_lib,
        dependencies: [base_dep, base_deps]
    )

    benchmark(testname, exe)
endforeach

warning('Benchmarks built')


### Build citylife-core

core_deps = []

core_source_files = run_command('find', 'src/core-libs', '-name', '*.c').stdout().strip().split('\n')
core_include_dirs = run_command('find', 'src/core-libs', '-name', 'include').stdout().strip().split('\n')

core_lib = library(
    'citylife-core',
    core_source_files,
    dependencies: core_deps,
    link_with : base_lib,
    include_directories: [base_include_dirs, core_include_dirs],
    install: true
)

core_dep = declare_dependency(
    include_directories : core_include_dirs,
    link_with : core_lib
)

warning('Core lib built')

### Test citylife-core

test_files = run_command('find', 'test/core-libs', '-name', '*.c', '-printf', '%p %f\n').stdout().strip().split('\n')

foreach p : test_files
    filename = p.split(' ')[0]
    testname = p.split(' ')[1].split('.')[0]

    exe = executable(
        testname, filename, 
        link_with: [base_lib, core_lib],
        dependencies: [base_dep, core_dep, core_deps]
    )

    test(testname, exe)
endforeach

warning('Core tests built')


### Build gateway service

gateway_svc_deps = []

gateway_svc_source_files = run_command('find', 'src/services/gateway', '-name', '*.c').stdout().strip().split('\n')
gateway_svc_include_dirs = run_command('find', 'src/services/gateway', '-name', 'include').stdout().strip().split('\n')

gateway_svc_lib = library(
    'gateway-svc',
    gateway_svc_source_files,
    dependencies: gateway_svc_deps,
    link_with : [base_lib, core_lib],
    include_directories: [base_include_dirs, core_include_dirs, gateway_svc_include_dirs],
    install: true
)

gateway_svc_dep = declare_dependency(
    include_directories : gateway_svc_include_dirs,
    link_with : gateway_svc_lib
)

exe = executable(
    'gateway', gateway_svc_source_files,
    link_with: [base_lib, core_lib, gateway_svc_lib],
    dependencies: [base_dep, core_dep, gateway_svc_dep]
)

warning('Gateway service built')

### Test gateway service

test_files = run_command('find', 'test/services/gateway', '-name', '*.c', '-printf', '%p %f\n').stdout().strip().split('\n')

foreach p : test_files
    filename = p.split(' ')[0]
    testname = p.split(' ')[1].split('.')[0]

    exe = executable(
        testname, filename, 
        link_with: [base_lib, core_lib, gateway_svc_lib],
        dependencies: [base_dep, core_dep, gateway_svc_dep]
    )

    test(testname, exe)
endforeach

warning('Gateway service tests built')