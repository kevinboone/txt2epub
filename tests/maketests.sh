#!/usr/bin/bash

../txt2epub -o markdown.epub markdown.txt
../txt2epub -f -o ch.epub ch1.txt ch2.txt 
../txt2epub -o xhtml.epub xhtml.xhtml 
../txt2epub -o longlines.epub longlines.txt 
../txt2epub -x -o longlines_nobreak.epub longlines_nobreak.txt 
../txt2epub -o ampersand.epub ampersand.txt 
../txt2epub --verbatim-marker 𐄁 -o mixed.epub mixed.txt 


