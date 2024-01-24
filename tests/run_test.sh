#!/bin/bash

# Stop if anything fails
set -e

# Getting executable paths
baseExecutable=${1}
newExecutable=${2}

# Getting script name
script=${3}

# Getting additional arguments
testerArgs=${@:4}

# If running full cycle, add that to the hash file output
mode="normal"
if [ "${4}" = "--fullCycle" ]; then mode="fullCycle"; fi

# Getting current folder (game name)
folder=`basename $PWD`

# Getting pid (for uniqueness)
pid=$$

# Hash files
baseHashFile="/tmp/quickerNES.${folder}.${script}.${mode}.${pid}.hash"
newHashFile="/tmp/quickNES.${folder}.${script}.${mode}.${pid}.hash"

# Removing them if already present
rm -f ${baseHashFile}
rm -f ${newHashFile}

set -x
# Running script on quickerNES
${baseExecutable} ${script} --hashOutputFile ${baseHashFile} ${testerArgs}

# Running script on quickNES
${newExecutable} ${script} --hashOutputFile ${newHashFile} ${testerArgs}
set +x

# Comparing hashes
baseHash=`cat ${baseHashFile}`
 
# Comparing hashes
newHash=`cat ${newHashFile}`

# Removing temporary files
rm -f ${baseHashFile} ${newHashFile}

# Compare hashes
if [ "${baseHash}" = "${newHash}" ]; then
 echo "[] Test Passed"
 exit 0
else
 echo "[] Test Failed"
 exit -1
fi
