#!/bin/bash

filesdir=$"empty"
searchstr=$"empty"


if [[ -z "$1" || -z "$2" ]]; then
    # Print an error message if the parameter is missing
    echo "Error: filesdir or searchstr are missing."
    # Exit with a return value of 1 (indicating error)
    exit 1
else
    filesdir=$1
    searchstr=$2
fi

#Check if filesdir represent a directory on the filesystem

if [[ -d $filesdir ]]; then
    X=$(find $filesdir -type f | wc -l)
    Y=$(grep -r -o $searchstr $filesdir | wc -l)
    echo "The number of files are $X and the number of matching lines are $Y"
else
    echo "Error: directory doesn't exists on the filesystem."
    exit 1
fi

