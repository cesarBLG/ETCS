#!/bin/bash

# find all .po files recursively in the current directory and its subdirectories
for po_file in $(find . -name "*.po"); do
    # compile the .po file into a .mo file with the same name in the same directory
    msgfmt -o "${po_file%.po}.mo" "$po_file"
done