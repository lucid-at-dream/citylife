#!/bin/bash

mkdir -p build

for css in $(ls style)
do
        name=$(echo $css | cut -d'.' -f1)
        sassc style/$css build/$name.css
done
