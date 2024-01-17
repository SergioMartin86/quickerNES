game = 'arkanoid'

bash = find_program('bash')

goal = 'warpless'
test(goal + '-FullCycle', bash, workdir : meson.current_source_dir(), timeout: testTimeout, args : [ testCommands, goal + '.test', '--fullCycle'], suite: [ game, goal ] )

goal = 'warps'
test(goal + '-FullCycle', bash, workdir : meson.current_source_dir(), timeout: testTimeout, args : [ testCommands, goal + '.test', '--fullCycle'], suite: [ game, goal ] )