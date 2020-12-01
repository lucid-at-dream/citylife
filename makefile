.PHONY: test setup compile sonar clean

default: test

test: setup
	meson test --wrap='valgrind' -C build
	gcovr -r src build --sonarqube -o build/coverage.xml
	sed -i 's|path="|path="src/|g' build/coverage.xml
	cat build/meson-logs/testlog-valgrind.txt

compile: setup
	meson compile -C build

setup:
	meson setup build
	meson configure -Db_coverage=true build

sonar:
	build-wrapper-linux-x86-64 --out-dir bw-output make test
	sonar-scanner -X

clean:
	rm -rf build || true
	rm -rf .scannerwork || true
	rm -rf bw-output || true
