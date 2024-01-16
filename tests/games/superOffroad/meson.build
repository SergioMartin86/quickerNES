game = 'superOffroad'

goal = 'anyPercent'
test(goal + '-FullCycle', bash, workdir : meson.current_source_dir(), timeout: 60, args : [ testCommands, goal + '.test', '--fullCycle'], suite: [ game, goal ] )
test(goal, bash, workdir : meson.current_source_dir(), timeout: 60, args : [ testCommands, goal + '.test'], suite: [ game, goal ] )