#!/bin/bash

# Correct all links to http://developer.apple.com/... to http://web.archive.org/web/20080831224435/http://developer.apple.com/...

time find . -name '*.html' -exec sed -i '' -e 's|href="http://developer.apple.com|href="http://web.archive.org/web/20080831224435/http://developer.apple.com|g' {} \;
