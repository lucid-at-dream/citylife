#!/bin/bash

for i in {A..Z}; do
	cp SampleCode "Service$i.java"
	sed -i "s/AnotherService/Service$i/g" Service$i.java
done


