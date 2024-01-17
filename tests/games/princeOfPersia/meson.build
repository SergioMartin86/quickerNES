game = 'princeOfPersia'

goal = 'anyPercent'
test(goal + '-FullCycle', bash, workdir : meson.current_source_dir(), timeout: testTimeout, args : [ testCommands, goal + '.test', '--fullCycle'], suite: [ game, goal ] )