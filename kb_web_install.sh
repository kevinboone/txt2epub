#!/bin/bash
WEB=/home/kevin/docs/kzone5/
SOURCE=$WEB/source
TARGET=$WEB/target
make clean
(cd ..; tar cvfz $TARGET/txt2epub.tar.gz txt2epub)
cp README_txt2epub.html $SOURCE
(cd $WEB; ./make.pl txt2epub)
