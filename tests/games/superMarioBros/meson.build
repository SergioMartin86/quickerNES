game = 'superMarioBros'

goal = 'warps'
test(goal + '-FullCycle', bash, workdir : meson.current_source_dir(), args : [ testCommands, goal + '.test', '--fullCycle'], suite: [ game, goal ] )

goal = 'warpless'
test(goal + '-FullCycle', bash, workdir : meson.current_source_dir(), args : [ testCommands, goal + '.test', '--fullCycle'], suite: [ game, goal ] )