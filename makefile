
sonar: clean
	make -C auth sonar
	make -C gateway sonar
	make -C eva sonar
	sonar-scanner -X

clean:
	rm -rf bw-output

