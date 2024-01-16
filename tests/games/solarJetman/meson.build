game = 'solarJetman'

goal = 'anyPercent'
test(goal + '-FullCycle', bash, workdir : meson.current_source_dir(), args : [ testCommands, goal + '.test', '--fullCycle'], suite: [ game, goal ] )
test(goal, bash, workdir : meson.current_source_dir(), args : [ testCommands, goal + '.test'], suite: [ game, goal ] )