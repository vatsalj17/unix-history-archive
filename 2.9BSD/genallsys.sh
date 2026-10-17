#
cp unix rlunix
cp unix rpunix
cp unix rkunix
cp unix hkunix
cp unix xpunix
adb -w rlunix << 'EOF'
swapdev?w 04000
rootdev?w 04000
pipedev?w 04000
swplo?W 8000
nswap?w 2240
'EOF'
adb -w rpunix << 'EOF'
0254?w rpio
swapdev?w 0401
rootdev?w 0400
pipedev?w 0400
nswap?w 5200
'EOF'
adb -w hkunix << 'EOF'
swapdev?w 02001
rootdev?w 02000
pipedev?w 02000
nswap?w 2376
'EOF'
adb -w xpunix << 'EOF'
0254?w xpio
swapdev?w 03001
rootdev?w 03000
pipedev?w 03000
nswap?w 4800
'EOF'
adb -w rkunix << 'EOF'
swapdev?w 0
rootdev?w 0
pipedev?w 0
swplo?W 4000
nswap?w 872
'EOF'
