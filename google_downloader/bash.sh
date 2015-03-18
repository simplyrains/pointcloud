for i in `cat text.txt` ; do curl -O $i ; done
let "a = 0"
for old in *.*; do
	mv $old ${old##*.}.jpg
done
mv txt.jpg text.txt