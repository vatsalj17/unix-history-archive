tset -e -s -r -d \?adm3a > /tmp/tset.$$
source /tmp/tset.$$
rm /tmp/tset.$$
stty newcrt
set path = (~/bin $path)
echo "not even you can prevent head crashes"
