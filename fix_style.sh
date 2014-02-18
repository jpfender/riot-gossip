#!/bin/sh
cd $(dirname $0)
find . -name "*.[c|h]" | xargs astyle --options=.astylerc
find . -name "*.[c|h]" | xargs sed -i -f keyword_space.sed
