#!/bin/bash
for ((i = 0; i < 24; i++)); do
	magick -size 100x100 -gravity center label:$i PNG32:$i.png
done
