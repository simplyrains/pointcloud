cd ../build/Debug/$1
FILES=*.txt
for f in $FILES
do
  	echo "Processing $f file..."
	split(){ arr=($f); }
	IFS=','$'.' split
	LAT=${arr[0]}.${arr[1]}
	LNG=${arr[2]}.${arr[3]}
	HEADING=${arr[4]}.${arr[5]}
	DIS=${arr[6]}.${arr[7]}
	echo '   - 'LAT = $LAT
	echo '   - 'LNG = $LNG
	DIR_NAME=$LAT','$LNG','$HEADING','$DIS
	mkdir $DIR_NAME
	cd $DIR_NAME
	for i in `cat ../$f` ; do curl -O $i ; done
	for old in *.*; do
		mv $old ${old##*.}.jpg
	done
	cd ..
done