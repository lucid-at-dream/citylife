
default: test

test: clean
	make -C src compile test

sonar: clean
	build-wrapper-linux-x86-64 --out-dir bw-output make -C src compile test
	sonar-scanner -X

clean:
	rm -rf tests
	rm -rf bin
	rm -rf obj
	rm -rf *.gcov
	rm -rf bw-output
	make -C src clean
