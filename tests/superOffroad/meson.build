game = 'superOffroad'

goal = 'anyPercent'
test(goal, bash, workdir : meson.current_source_dir(), timeout: testTimeout, args : [ testCommands, goal + '.test', '--cycleType', 'Full'], suite: [ game, goal ] )