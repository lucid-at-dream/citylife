.PHONY: setup build test sonar clean

default: test

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

sonar: clean
	build-wrapper-linux-x86-64 --out-dir bw-output make test
	sonar-scanner -X

clean:
	rm -rf build || true
	rm -rf .scannerwork || true
	rm -rf bw-output || true
