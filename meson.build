project('citylife', 'c')

cc = meson.get_compiler('c')
add_global_arguments('-Wall', '-g', language : 'c')

### Copy test resources to build directory

run_command('mkdir', '-p', meson.build_root() + '/test')
run_command('cp', '-r', 'test/_resources/', meson.build_root() + '/test')

### Build citylife-base

citylife_base_deps = [
    dependency('threads'),
    cc.find_library('m', required : true)
]

base_source_files = run_command('find', 'src/base-libs', '-name', '*.c').stdout().strip().split('\n')
base_include_dirs = run_command('find', 'src/base-libs', '-name', 'include').stdout().strip().split('\n')

citylife_base_lib = library(
    'citylife-base',
    base_source_files,
    dependencies: citylife_base_deps,
    include_directories: base_include_dirs,
    install: true
)

citylife_base_dep = declare_dependency(
    include_directories : base_include_dirs,
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

core_source_files = run_command('find', 'src/core-libs', '-name', '*.c').stdout().strip().split('\n')
core_include_dirs = run_command('find', 'src/core-libs', '-name', 'include').stdout().strip().split('\n')

citylife_core_lib = library(
    'citylife-core',
    core_source_files,
    dependencies: citylife_core_deps,
    link_with : citylife_base_lib,
    include_directories: [base_include_dirs, core_include_dirs],
    install: true
)

citylife_core_dep = declare_dependency(
    include_directories : core_include_dirs,
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


### Build gateway service

gateway_svc_deps = []

gateway_svc_source_files = run_command('find', 'src/services/gateway', '-name', '*.c').stdout().strip().split('\n')
gateway_svc_include_dirs = run_command('find', 'src/services/gateway', '-name', 'include').stdout().strip().split('\n')

gateway_svc_lib = library(
    'gateway-svc',
    gateway_svc_source_files,
    dependencies: gateway_svc_deps,
    link_with : [citylife_base_lib, citylife_core_lib],
    include_directories: [base_include_dirs, core_include_dirs, gateway_svc_include_dirs],
    install: true
)

gateway_svc_dep = declare_dependency(
    include_directories : gateway_svc_include_dirs,
    link_with : gateway_svc_lib
)

exe = executable(
    'gateway', gateway_svc_source_files,
    link_with: [citylife_base_lib, citylife_core_lib, gateway_svc_lib],
    dependencies: [citylife_base_dep, citylife_core_dep, gateway_svc_dep]
)

### Test gateway service

test_files = run_command('find', 'test/services/gateway', '-name', '*.c', '-printf', '%p %f\n').stdout().strip().split('\n')

foreach p : test_files
    filename = p.split(' ')[0]
    testname = p.split(' ')[1].split('.')[0]

    exe = executable(
        testname, filename, 
        link_with: [citylife_base_lib, citylife_core_lib, gateway_svc_lib],
        dependencies: [citylife_base_dep, citylife_core_dep, gateway_svc_dep]
    )

    test(testname, exe)
endforeach
