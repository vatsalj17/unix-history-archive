Root=RootPath
echo Setting up structure for personalized interpreter ...
BaseDir=`pwd`
rm -rf $BaseDir/h $BaseDir/std $BaseDir/pi $BaseDir/picont $BaseDir/pilink $BaseDir/piconx
HDir=$BaseDir/h
PiDir=$BaseDir/pi
StdDir=$BaseDir/std
mkdir $PiDir $HDir $StdDir
cp $Root/Pimakefile $PiDir/Makefile
echo Copying files ...
cp $Root/pi/itran pitran
cp $Root/pi/h/"*" $HDir
cp $Root/pi/iconx/"*".h $StdDir
for j in link/"*" iconx/"*".c icont/"*".c rtlib linklib
do
   cp $Root/pi/$j $StdDir
done
ed - $PiDir/Makefile <<EOF >/dev/null
/^Dir=/
s:^.*\$:Dir=$BaseDir:
w
q
EOF
$Root/Ranlib $StdDir
echo Personalized interpreter is complete.
