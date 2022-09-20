#!/bin/sh
echo "Deleting al backup files (tilde files!) ... "
find . -name *.*~ -delete
echo "Done."

