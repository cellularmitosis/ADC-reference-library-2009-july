#!/bin/bash

set -e -o pipefail

# replace developer.apple.com links with relative links where possible.
# (with help from Gemini AI)

if test "$(uname -s)" = "Linux" ; then
    echo "Error: this script uses BSD syntax for the sed -i option." >&2
    exit 1
fi

find . -name '*.html' -print0 | while IFS= read -r -d $'\0' file; do
    printf '\e]2;%s\a' "$file"
    echo $file
    dir="$(dirname "$file")"
    if test -e "${dir}/ROOT" ; then
        prefix="./"
    else
        prefix="../"
        while ! test -e "${dir}/${prefix}ROOT" ; do
            prefix=${prefix}../
        done
    fi

    # target_paths="css/ documentation/ featuredarticles/ images/ index.html js/ media/ qa/ reference/ referencelibrary/ releasenotes/ samplecode/ style.css technicalnotes/ technicalqas/ technotes/"

    # replace all http://developer.apple.com/ links with relative links.
    sed -i '' \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/css/|href=\"${prefix}css/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/documentation/|href=\"${prefix}documentation/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/featuredarticles/|href=\"${prefix}featuredarticles/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/images/|href=\"${prefix}images/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/index.html|href=\"${prefix}index.html|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/js/|href=\"${prefix}js/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/media/|href=\"${prefix}media/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/qa/|href=\"${prefix}qa/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/reference/|href=\"${prefix}reference/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/referencelibrary/|href=\"${prefix}referencelibrary/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/releasenotes/|href=\"${prefix}releasenotes/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/samplecode/|href=\"${prefix}samplecode/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/style.css|href=\"${prefix}style.css|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/technicalnotes/|href=\"${prefix}technicalnotes/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/technicalqas/|href=\"${prefix}technicalqas/|g" \
        -e "s|[hH][rR][eE][fF]=\"https*://developer\.apple\.com/technotes/|href=\"${prefix}technotes/|g" \
        "$file"

    # replace all / links with relative links.
    sed -i '' \
        -e "s|[hH][rR][eE][fF]=\"/css/|href=\"${prefix}css/|g" \
        -e "s|[hH][rR][eE][fF]=\"/documentation/|href=\"${prefix}documentation/|g" \
        -e "s|[hH][rR][eE][fF]=\"/featuredarticles/|href=\"${prefix}featuredarticles/|g" \
        -e "s|[hH][rR][eE][fF]=\"/images/|href=\"${prefix}images/|g" \
        -e "s|[hH][rR][eE][fF]=\"/index.html|href=\"${prefix}index.html|g" \
        -e "s|[hH][rR][eE][fF]=\"/js/|href=\"${prefix}js/|g" \
        -e "s|[hH][rR][eE][fF]=\"/media/|href=\"${prefix}media/|g" \
        -e "s|[hH][rR][eE][fF]=\"/qa/|href=\"${prefix}qa/|g" \
        -e "s|[hH][rR][eE][fF]=\"/reference/|href=\"${prefix}reference/|g" \
        -e "s|[hH][rR][eE][fF]=\"/referencelibrary/|href=\"${prefix}referencelibrary/|g" \
        -e "s|[hH][rR][eE][fF]=\"/releasenotes/|href=\"${prefix}releasenotes/|g" \
        -e "s|[hH][rR][eE][fF]=\"/samplecode/|href=\"${prefix}samplecode/|g" \
        -e "s|[hH][rR][eE][fF]=\"/style.css|href=\"${prefix}style.css|g" \
        -e "s|[hH][rR][eE][fF]=\"/technicalnotes/|href=\"${prefix}technicalnotes/|g" \
        -e "s|[hH][rR][eE][fF]=\"/technicalqas/|href=\"${prefix}technicalqas/|g" \
        -e "s|[hH][rR][eE][fF]=\"/technotes/|href=\"${prefix}technotes/|g" \
        "$file"
done
