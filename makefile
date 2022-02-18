.PHONY: setup build test sonar clean

TEST=map_tests

default: build

setup:
	meson setup build
	meson configure -Db_coverage=true build

build: setup
	meson compile -C build

test: build
	meson test --wrap='valgrind --leak-check=full --error-exitcode=1' -C build
	gcovr -r src build --sonarqube -o build/coverage.xml
	sed -i 's|path="|path="src/|g' build/coverage.xml
	cat build/meson-logs/testlog-valgrind.txt

bench: build
	meson test --benchmark -C build --verbose

ci: build test bench

sonar: clean
	build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir bw-output make test
	sonar-scanner/bin/sonar-scanner -X

clean:
	rm -rf build || true
	rm -rf .scannerwork || true
	rm -rf bw-output || true

debug-test:
	meson test ${TEST} --gdb -C build

solo-test:
	meson test ${TEST} --print-errorlogs -C build
