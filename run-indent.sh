#!/bin/sh
if [ -z $1 ]; then
	echo "Indent all codebase ... "
	find . -name "*.[c,h]" -execdir indent -linux -i8 -l100 {} \;
	find . -name "*.[c,h]~" -delete
else
	echo "Indent ${1} ... "
	indent -linux -i8 -l100 $1 -o $1
fi
echo "Done."

