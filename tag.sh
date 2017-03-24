#! /bin/sh

DD=`date +'%Y%m%d%H%M%S'`
sed -i  configure.ac -e "s/1\.9\.0/1.9.0.$DD/"
cd website
sed -i  index.html -e "s/DATETIME/$DD/"
cd ..
