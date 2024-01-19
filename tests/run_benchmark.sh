#!/bin/bash

# Finding all test scripts
testScriptList=`find . -type f -name *.test`

# Iterating over the scripts
for script in ${testScriptList}; do
  
  # Getting base folder
  folder=`dirname ${script}`

  # Getting filename
  fileName=`basename ${script}`
 
  # Going to folder
  pushd ${folder}

  # Running script on quickerNES
  quickerNESTester ${fileName} --cycleType Rerecord

  # Running script on quickerNES
  quickNESTester ${fileName} --cycleType Rerecord

  # Coming back
  popd
done

