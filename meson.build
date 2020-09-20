project('demo', 'c')

cc = meson.get_compiler('c')

deps = [
    dependency('threads'),
    cc.find_library('m', required : true)
]

lib = library(
    'citylife', 
    'src/libs/bigint/bigint.c',
    'src/libs/config/config.c',
    'src/libs/ctest/assert.c',
    'src/libs/ctest/test.c',
    'src/libs/hashmap/map.c',
    'src/libs/jasmine/jsmn.c',
    'src/libs/logger/logger.c',
    'src/libs/queue/queue.c',
    'src/libs/server/server.c',
    'src/libs/storage/append_list.c',
    'src/libs/workerpool/worker_pool.c',
    dependencies: deps,
    install: true
)

test_exes = [
    ['bigint', 'src/libs/bigint/bigint_tests.c', 'unit tests for bigint library'],
    ['config', 'src/libs/config/config_tests.c', 'unit tests for config library'],
    ['map', 'src/libs/hashmap/map_tests.c', 'unit tests for map library'],
    ['queue', 'src/libs/queue/queue_tests.c', 'unit tests for queue library'],
    ['server', 'src/libs/server/server_tests.c', 'unit tests for server library'],
    ['worker_pool', 'src/libs/workerpool/worker_pool_tests.c', 'unit tests for worker_pool library']
]


foreach p : test_exes
    exe = executable(p[0], p[1], link_with: lib, dependencies: deps)
    test(p[2], exe)
endforeach

progs = [
]

foreach p : progs
    executable(p[0], p[1], link_with: lib, dependencies: deps)
endforeach
