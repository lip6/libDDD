uttk=`which uttk`
if [ $? -eq 1 ]
then
        echo "==============================================================================="
        echo "= Could not find uttk, please download it from http://uttk.org and install it ="
        echo "==============================================================================="
        exit 0
fi        
$uttk test.yml
# Juste pour un affichage sur mon site http://lrde.epita.fr/~charron/tests.html
# scp log.html ssh:~/www/tests.html
