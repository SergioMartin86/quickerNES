game = 'arkanoid'

bash = find_program('bash')

goal = 'warpless'
test(goal, bash, workdir : meson.current_source_dir(), timeout: testTimeout, args : [ testCommands, goal + '.test', '--cycleType', 'Full'], suite: [ game, goal ] )

goal = 'warps'
test(goal, bash, workdir : meson.current_source_dir(), timeout: testTimeout, args : [ testCommands, goal + '.test', '--cycleType', 'Full'], suite: [ game, goal ] )