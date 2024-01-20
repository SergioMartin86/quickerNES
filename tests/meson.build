nomalloc = environment({'MALLOC_PERTURB_': '0'})

bash = find_program('bash')
testCommands = ['../run_test.sh', quickerNESTester.path(), quickNESTester.path() ]
testTimeout = 120

if get_option('onlyOpenSource') == false

subdir('arkanoid')
subdir('castlevania1')
subdir('superOffroad')
subdir('princeOfPersia')
subdir('ninjaGaiden')
subdir('ninjaGaiden2')
subdir('ironSword')
subdir('solarJetman')
subdir('tennis')
subdir('metroid')
subdir('nigelMansell')
subdir('galaga')
subdir('saintSeiyaOugonDensetsu')
subdir('saintSeiyaKanketsuHen')
subdir('superMarioBros')
subdir('superMarioBros3')
subdir('saiyuukiWorld')

endif

# Open source games
subdir('sprilo')
subdir('novaTheSquirrel')