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

# Running script on quickerNES
${quickerNESExecutable} ${script} --hashOutputFile quickerNES.${script}.${mode}.hash ${testerArgs}

# Running script on quickNES
${quickNESExecutable} ${script} --hashOutputFile quickNES.${script}.${mode}.hash ${testerArgs}

# Comparing hashes
quickerNESHash=`cat quickerNES.hash`
 
# Comparing hashes
quickNESHash=`cat quickNES.hash`

# Compare hashes
if [ "${quickerNESHash}" = "${quickNESHash}" ]; then
 echo "[] Test Passed"
 exit 0
else
 echo "[] Test Failed"
 exit -1
fi
