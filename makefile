
sonar:
	make -C auth sonar
	make -C gateway sonar
	sonar-scanner -X

