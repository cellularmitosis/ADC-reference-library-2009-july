#!/bin/bash

set -e -o pipefail

find samplecode -name 'project.pbxproj' \
    | cut -d/ -f1-2 \
    | perl -pe 's|(.*)|[\1](\1)|' \
    | perl -pe 's|(\[.*?/)|- [|' \
    | sort -f \
    | uniq
