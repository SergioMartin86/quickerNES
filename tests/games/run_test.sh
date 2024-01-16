#!/bin/bash

# Stop if anything fails
set -e

# Getting executable paths
quickerNESExecutable=${1}
quickNESExecutable=${2}

# Getting script name
script=${3}

# Getting additional arguments
testerArgs=${@:4}

# If running full cycle, add that to the hash file output
mode="normal"
if [ "${4}" = "--fullCycle" ]; then mode="fullCycle"; fi

# Hash files
quickerNESHashFile="quickerNES.${script}.${mode}.hash"
quickNESHashFile="quickNES.${script}.${mode}.hash"

# Removing them if already present
rm -f ${quickerNESHashFile}
rm -f ${quickNESHashFile}

# Running script on quickerNES
${quickerNESExecutable} ${script} --hashOutputFile ${quickerNESHashFile} ${testerArgs}

# Running script on quickNES
${quickNESExecutable} ${script} --hashOutputFile ${quickNESHashFile} ${testerArgs}

# Comparing hashes
quickerNESHash=`cat ${quickerNESHashFile}`
 
# Comparing hashes
quickNESHash=`cat ${quickNESHashFile}`

# Compare hashes
if [ "${quickerNESHash}" = "${quickNESHash}" ]; then
 echo "[] Test Passed"
 exit 0
else
 echo "[] Test Failed"
 exit -1
fi
