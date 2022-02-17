#!/bin/sh
echo "Indent all codebase ... "
find . -name *.[c,h] -execdir indent -linux -i8 -l100 {} \;
find . -name *.[c,h]~ -delete
echo "Done."
