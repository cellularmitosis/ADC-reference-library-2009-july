#!/usr/bin/env sh

# main.command
# Find Images by Keyword

#  Created by Otto on 6/6/07.
#  Copyright 2007 Apple, Inc. All rights reserved.

while read fldrPath; do
	mdfind -onlyin "$fldrPath" "(kMDItemContentType == 'public.jpeg' || kMDItemContentType == 'public.tiff' || kMDItemContentType == 'public.png') && kMDItemKeywords == '${keywordString}'"
	echo
done