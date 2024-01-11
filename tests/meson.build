nomalloc = environment({'MALLOC_PERTURB_': '0'})

# Adding game ending checks
endingChecker = executable('endingChecker', files(['endingChecker.cpp']), dependencies: [ quickerNESCoreDependency ] )
testSuite = [ 'gameEndings' ]
test('castlevania1AnyPercent', endingChecker, args : ['castlevania1AnyPercent.test'], suite: testSuite)
