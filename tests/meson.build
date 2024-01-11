nomalloc = environment({'MALLOC_PERTURB_': '0'})

# Adding game ending checks
endingChecker = executable('endingChecker',
 files(['endingChecker.cpp']),
 dependencies: [ quickerNESCoreDependency ]
 )

testSuite = [ 'gameEndings' ]
test('castlevania1AnyPercent', endingChecker, workdir : meson.current_source_dir(), args : ['roms/Castlevania (U) (PRG0) [!].nes'], suite: testSuite)
