gitBranch=`git branch --show-current`
gitCommit=`git rev-parse HEAD`

echo "Git revision: ${gitBranch}:${gitCommit}" 

echo "Getting system information"
lscpu
lsmem
echo "Running Tests sequentially..."

pushd arkanoid
tester warps.test
tester warpless.test
popd

pushd ninjaGaiden2
tester pacifist.test
tester anyPercent.test
popd

pushd superOffroad
tester anyPercent.test
popd

pushd nigelMansell
tester anyPercent.test
popd

pushd castlevania1
tester pacifist.test
tester anyPercent.test
popd 

pushd saintSeiyaKanketsuHen
tester anyPercent.test
popd

pushd tennis
tester anyPercent.test
popd

pushd superMarioBros
tester warps.test
tester warpless.test
popd 

pushd ninjaGaiden
tester pacifist.test
tester anyPercent.test
popd 

pushd ironSword
tester anyPercent.test
popd 

pushd saintSeiyaOugonDensetsu
tester anyPercent.test
popd 

pushd princeOfPersia
tester lvl7.test
popd

pushd solarJetman
tester anyPercent.test
popd 

pushd galaga
tester anyPercent.test
popd
