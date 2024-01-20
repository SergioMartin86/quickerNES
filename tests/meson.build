nomalloc = environment({'MALLOC_PERTURB_': '0'})

bash = find_program('bash')
testCommands = ['run_test.sh', quickerNESTester.path(), quickNESTester.path() ]
testTimeout = 120

# Tests for copyrighted game roms (only for local testing or own CI runners)
protectedTestSet = [ 
  'arkanoid.warpless.test',
  'arkanoid.warps.test',
  'castlevania1.anyPercent.test',
  'castlevania1.pacifist.test',
  'galaga.anyPercent.test',
  'ironSword.anyPercent.test',
  'metroid.playaround.test',
  'nigelMansell.anyPercent.test',
  'ninjaGaiden.anyPercent.test',
  'ninjaGaiden.pacifist.test',
  'ninjaGaiden2.anyPercent.test',
  'ninjaGaiden2.pacifist.test',
  'novaTheSquirrel.anyPercent.test',
  'princeOfPersia.anyPercent.test',
  'saintSeiyaKanketsuHen.anyPercent.test',
  'saintSeiyaOugonDensetsu.anyPercent.test',
  'saiyuukiWorld.anyPercent.test',
  'saiyuukiWorld.lastHalf.test',
  'solarJetman.anyPercent.test',
  'sprilo.anyPercent.test',
  'superMarioBros.warpless.test',
  'superMarioBros.warps.test',
  'superMarioBros3.warps.test',
  'superOffroad.anyPercent.test',
  'tennis.anyPercent.test',
]

# Tests for open source free roms (for public cloud testing)
openSourceTestSet = [
  'novaTheSquirrel.anyPercent.test',
  'sprilo.anyPercent.test',
]

# Creating test set based on whether copyrighted roms are to be used
testSet = openSourceTestSet
if get_option('onlyOpenSource') == false
 testSet += protectedTestSet
endif

# Adding tests to the suite
foreach testFile : testSet
  testSuite = testFile.split('.')[0]
  testName = testFile.split('.')[1]
  test(testName,
       bash,
       workdir : meson.current_source_dir(),
       timeout: testTimeout,
       args : [ testCommands, testFile, '--cycleType', 'Full'],
       suite : [ testSuite ])
endforeach

