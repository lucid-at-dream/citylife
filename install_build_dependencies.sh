#/bin/bash

sudo apt-get install -y python3 python3-pip python3-setuptools python3-wheel ninja-build valgrind gcovr

sudo pip3 install meson

sudo apt-get install -y libpqxx-dev libcairomm-1.0-dev libgtkmm-3.0-dev bison flex

wget https://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip

unzip build-wrapper-linux-x86.zip

export PATH="$PATH:$(pwd)/build-wrapper-linux-x86"

wget https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-3.2.0.1227.zip

unzip sonar-scanner-cli-3.2.0.1227.zip

export PATH="$PATH:$(pwd)/sonar-scanner-3.2.0.1227/bin"

