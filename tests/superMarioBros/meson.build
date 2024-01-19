game = 'superMarioBros'

goal = 'warps'
test(goal, bash, workdir : meson.current_source_dir(), args : [ testCommands, goal + '.test', '--cycleType', 'Full'], suite: [ game, goal ] )

goal = 'warpless'
test(goal, bash, workdir : meson.current_source_dir(), args : [ testCommands, goal + '.test', '--cycleType', 'Full'], suite: [ game, goal ] )