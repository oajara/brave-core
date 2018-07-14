#!/bin/bash
#
# Push string changes to Transifex
#
# Error Codes:
#   0 = Success
#   1 = USERNAME must be set
#   2 = PASSWORD must be set
#   3 = Failed to export strings from Xcode project
#   4 = Failed to cleanup strings
#   5 = Failed to push string changes to Transifex

report_error()
{
  echo $2
  cleanup
  exit $1
}

cleanup()
{
  echo "Cleaning up temporary files..."

  if [ -e transifex.log ] ; then
    cat transifex.log >> output.log
    rm transifex.log
  fi

  if [ -e en.xliff ] ; then
    rm en.xliff
  fi
}

if [ "${USERNAME}" = "" ] ; then
  report_error 1 "USERNAME environment variable must be set to \"api\" or your Transifex username"
fi

if [ "${PASSWORD}" = "" ] ; then
  if [[ $(tr "[:upper:]" "[:lower:]" <<<"$USERNAME") = \
  $(tr "[:upper:]" "[:lower:]" <<<"api") ]] ; then
    report_error 2 "PASSWORD environment variable must be set to your Transifex API token"
  else
    report_error 2 "PASSWORD environment variable must be set to your Transifex password"
  fi
fi

cd $(dirname "$0")

echo "Exporting strings from Xcode project..."
(cd ../ && xcodebuild -exportLocalizations) &> output.log
if [ $? != 0 ] ; then
  report_error 3 "ERROR: Failed to export strings from Xcode project, please see output.log"
fi

mv -f ../Client/en.xliff . >>output.log 2>&1
if [ $? != 0 ] ; then
  report_error 3 "ERROR: Failed to export strings from Xcode project, please see output.log"
fi

echo "Cleaning up strings..."
./xliff-cleanup.py en.xliff >>output.log 2>&1
if [ $? != 0 ] ; then
  report_error 4 "ERROR: Failed to cleanup strings, please see output.log"
fi

sed -i '' 's/Shared\/Supporting Files/brave/' en.xliff >>output.log 2>&1
if [ $? != 0 ] ; then
  report_error 4 "ERROR: Failed to cleanup strings, please see output.log"
fi

echo "Pushing string changes to Transifex..."
curl -# -L -o transifex.log --user ${USERNAME}:${PASSWORD} -F file=@en.xliff -X PUT \
https://www.transifex.com/api/2/project/brave-ios/resource/bravexliff/content/

if ! grep -q "strings_" transifex.log ; then
  report_error 5 "ERROR: Failed to push string changes to Transifex, please see output.log"
fi

cleanup
