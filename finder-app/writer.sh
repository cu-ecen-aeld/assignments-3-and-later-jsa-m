#!/bin/bash

writefile="empty"
writestr="empty"

if [[ -z "$1" || -z "$2" ]]; then
    # Print an error message if the parameter is missing
    echo "Error: writefile or writestr are missing."
    # Exit with a return value of 1 (indicating error)
    exit 1
else
    writefile=$1
    writestr=$2
fi

dir=$(dirname "$writefile")

# Ensure the directory exists
dir=$(dirname "$writefile")
if [[ ! -d "$dir" ]]; then
    mkdir -p "$dir"  # Create the directory if it doesn't exist
fi

echo "$writestr" > $writefile
if [[ $? -ne 0 ]]; then
    echo "Error: Unable to create or write $filesdir"
    exit 1
fi

