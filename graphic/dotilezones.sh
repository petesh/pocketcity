#! /bin/bash -p

typeset me=${0##*/}
typeset infile=
typeset headerfile=
typeset width=
typeset height=
typeset rwidth=
typeset rheight=
typeset sizx=24
typeset sizy=12
typeset id=0
typeset remove=
typeset cast_to=
typeset -a types=(bw grey color)

trap cleanup 0 1

PATH=/usr/local/bin:$PATH

function cleanup {
	rm -rf $remove
}
# check and convert a string of the form <width>x<height> into the passed
# variables.
# usage: checkwh <string> <width var> <height var>
# returns: pass(0) if it worked, fail(1) otherwise
function checkwh {
	typeset w=
	typeset h=
	if [ -z "$1" -o -z "$2" -o -z "$3" ]; then
		return 1
	fi
	eval `echo $1 | sed -n 's/\(.*\)x\(.*\)/w=\1;h=\2;/p'`
	[[ -z $w ]] || [[ -z $h ]] || [[ $w -le 0 ]] || [[ $h -le 0 ]] && \
		return 1
	eval "$2=$w;$3=$h;"
	return 0
}

# check for the existence of a file. If it doesn't exist then evaluate
# expression passed in
function checkfile {
	if [[ ! -f $1 ]]; then
		echo "File '$1' not found"
		eval "$2"
	fi
}

# get temporary directory
# it has the pattern passed and is guaranteed to be in /tmp or $TMP and not
# exist before the function is called.
function mktmpdir {
	typeset tmp=$TMP
	typeset l=$$
	[ ! -d "$tmp" ] && tmp=$TEMP
	[ ! -d "$tmp" ] && tmp=/tmp
	temp=$tmp/$1-$$
	while [ -d "$temp" ]; do
		l=$((l + 1))
		temp=$tmp/$1-$l
	done
	mkdir $temp
	[ $? -eq 0 ] && echo $temp
}

function print_define {
        typeset define=$1
        typeset value=$2

        if [ -z "$cast_to" ]; then
                printf "#define\t%s\t%s\n" $define $value >&4
        else
                printf "#define\t%s\t((%s)(%s))\n" $define $cast_to $value >&4
        fi

}

# slice an image file into a series of tiles, incrementing a slice counter
# as we go.
# uses: width, height, rwidth, rheight, slicenum
# usage: sliceimage [<h>] <image file> <dest dir> <variable> \
#   <start> <end> <indiv> <comment>
function sliceimage {
	typeset index=0
	typeset -a st_def=
	typeset -a en_def=
	typeset -a in_def=
	typeset h=
	if [ $1  = "h" ]; then
		h=1
		shift
	fi
	typeset file=$1
	typeset ext=${1##*.}
	typeset destdir=$2
	typeset variable=$3
	typeset iw=
	typeset ih=
	typeset ypos=0

	[ -f $file ] || return 0

	IFS=, st_def=($4)
	IFS=, en_def=($5)
	IFS=, in_def=($6)

	# starting images
	eval "val=\$$variable;"
	[ -n "$h" ] && echo "/*! \\brief $7 */" >&4
	for def in ${st_def[@]}; do
		[ -n "$h" ] && print_define $def ${val}
	done
	eval `identify -format "iw=%w;ih=%h;" $file`
	while [ $ypos -lt $ih ]; do
		typeset xpos=0
		while [ $xpos -lt $iw ]; do
			typeset newf="$destdir/`printf "%0.3d" $val`.$ext"
			cp $file $newf
			mogrify -crop "${width}x${height}+${xpos}+${ypos}" $newf
			if [[ -n "$rwidth" ]]; then
				mogrify -resize "${rwidth}x${rheight}" $newf
			fi
			if [ -n "$h" -a -n "${in_def[$index]}" ]; then
				print_define ${in_def[$index]} $val
			fi
			val=$((val + 1))
			index=$((index + 1))
			xpos=$((xpos + width))
		done
		ypos=$((ypos + height))
	done
	typeset lval=$((val - 1))
	for def in ${en_def[@]}; do
		[ -n "$h" ] && print_define $def $lval
	done
	[ -n "$h" ] && echo "/* image:$1 */" >&4
	eval "$variable=$val"
}

function usage {
	cat <<EOM
usage: $me [ -d <width>x<height> ] [ -r <width>x<height> ] [ -t <types> ]
        [ -c <cast> ] <input file> <output file> [ <headerfile> ]

Mashes the files referred to in the input file (bw,grey,color) into
the combined tile file for use in the game. It also creates the include
file that results from this process.

The program will generate one file for each of the 3 color types:
	bw - black and white
	grey - 2 bit grey
	color - 256 color

in *both* .png and .bmp formats, utilizing imagemagick's identify and mogrify
commands

Input files are expected to be of the .png file type, with transparency
fully supported on the output file where possible.

Options are:
	-s <width>x<height> - shape the tile format to this
	-d <width>x<height> - make the tiles of this x and y dimension
	-r <width>x<height> - resize tiles to this size (after the fact)
        -t <type> - types to do (color, bw, grey)
        -c <type> - cast all thed efines to this type
	<input file> - the input file for reading the information
	<output file> - the output file for writing the information
	<headerfile> - the name of the header file to receive the output,
		or it will be dumped out to the command line

	special items in names for the files:
		\${width} - uses the tile width
		\${height} - uses the tile height
		\${rwidth} - uses the reduced tile width
		\${rheight} - uses the reduced tile height
EOM
	exit $1
}

typeset o=
while getopts "c:d:r:s:t:h?" o; do
	case $o in
                c)
                cast_to=$OPTARG
                ;;
		d)
		checkwh "$OPTARG" width height
		if [ $? -ne 0 ]; then
			echo "malformed: '$OPTARG' isn't a <width>x<height>"
			bad=1
		fi
		;;
		r)
		checkwh "$OPTARG" rwidth rheight
		if [ $? -ne 0 ]; then
			echo "malformed: '$OPTARG' isn't a <width>x<height>"
			bad=1
		fi
		;;
		s)
		checkwh "$OPTARG" sizx sizy
		if [ $? -ne 0 ]; then
			echo "malformed: '$OPTARG' isn't a <width>x<height>"
			bad=1
		fi
		;;
                t)
                types=($OPTARG)
                ;;
		h|\?)
		usage 1;;
	esac
done

shift $((OPTIND - 1))

if [ -z "$1" ]; then
	echo "error: Missing input file"
	bad=1
else
	infile=$1
fi

checkfile $infile "bad=1"

if [ -z "$2" ]; then
	echo "error: Missing output file"
	bad=1
else
	outfile=$2
fi

if [ -n "$bad" ]; then
	echo
	usage 1
fi

# redirect output of fd 4
if [ ! -z "$3" ]; then
	exec 4>$3
	if [ $? -ne 0 ]; then
		echo "Could not write to $3"
	fi
else
	exec 4>&1
fi

typeset OIFS=
typeset slicebw=0
typeset slicegrey=0
typeset slicecolor=0
typeset dirbw=
typeset dirgrey=
typeset dircolor=
typeset col=

for dir in ${types[@]}; do
	td=`mktmpdir $dir`
	if [ -z "$td" ]; then
		echo "could not create temp directory"
	else
		remove="$remove $td"
	fi
	eval "dir${dir}=$td"
done

# dirt:Z_DIRT:::Dirt Zone
sed -e '/^#/d' -e '/^$/d' $infile | while read inputline; do
	OIFS="$IFS"
	IFS=":"
	set -- $inputline
	IFS="$OIFS"
	typeset file=$1
	for col in ${types[@]}; do
		dir=dir${col}
		eval "dir=\$$dir"
		eval "counter=slice${col}"
		h=
		[[ $col = "color" ]] && h=h
		if [ ! -f "$col/$file.png" ]; then
			echo "could not open $col/$file.png"
			continue
		fi
		sliceimage $h "$col/$file.png" "$dir" "$counter" \
			"$2" "$3" "$4" "$5"
	done
done

if [[ -z $rwidth ]]; then
	rwidth=$width
	rheight=$height
fi
readonly nx=$sizx
readonly ny=$sizy
readonly shape=$((nx * ny))
readonly rx=$((rwidth * nx))
readonly ry=$((rheight * ny))

for col in ${types[@]}; do
        typeset cols=
        typeset typ=
        case $col in
        bw)
                cols=2
                typ=bilevel
                ;;
        grey)
                cols=16
                typ=grayscale
                ;;
        color)
                cols=98
                typ=Optimize
                ;;
        esac
	dir=dir${col}
	eval "dir=\$$dir"
	eval "outfile=$outfile"
	# create the file
	typeset dfile=$outfile-$col
	typeset -a ifiles=(`/bin/ls $dir/*.png 2>/dev/null`)
	typeset count=0
	typeset printed=0
	typeset xpos=0
	typeset ypos=0
	typeset compositesize=${rx}x${ry}
	for i in ${ifiles[@]}; do
		count=$((count + 1))
	done
	if [ $count -eq 0 ]; then
		continue
	fi
	# initial force creation of blank
        typeset bag=
        typeset tap=
        if [ $cols -gt 2 ]; then
                bag="-background #ffffff"
                tap="-transparent #ffffff"
        else
                bag=""
                tap=""
        fi
	montage $bag -geometry ${rwidth}x${rheight}+0+0 \
		-tile ${nx}x${ny} -adjoin ${ifiles[@]} $dfile.miff
	convert -type Optimize $tap $dfile.miff \
                -colors $cols $dfile.png
	convert $dfile.miff +compress -type $typ -colors $cols $dfile.bmp
	rm $dfile.miff
done
