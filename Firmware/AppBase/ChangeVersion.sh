#!/bin/bash

# Getting previous commit hash
commitHash=`git rev-parse --short HEAD`
echo "commitHash : $commitHash"

# Getting lineNumber to change
lineNumber=`grep -n COMMIT_NUMBER Version.h | cut -d: -f1`
# echo "lineNumber : $lineNumber"

# Replacing old hash with new one
cat Version.h | sed -e "${lineNumber}s/.*/#define COMMIT_NUMBER \"$commitHash\"/" > Version.tmp

# Changing file
rm Version.h
mv Version.tmp Version.h

exit 0