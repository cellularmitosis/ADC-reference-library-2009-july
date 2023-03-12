#!/bin/sh

PLATFORM_DIRECTORY=`dirname "${MW_OUTPUT_FILE}"`

# first get the platform identifier
PLATFORM=`basename "${PLATFORM_DIRECTORY}" | tr "[:upper:]" "[:lower:]"`

# now find the main resource file and rename it with the platform identifier
cd "${PLATFORM_DIRECTORY}/../Resources/English.lproj"
mv "${MW_OUTPUT_NAME}.rsrc" "Localized-${PLATFORM}.rsrc"
touch "Localized.rsrc"