#! /bin/bash -p

# generate the simcity-??[_hd] files based on the options

function usage
{
	cat <<EOM
$0: - generate bitmap rcp files [ -d <density> ] [ -c <colors> ]
	[ -D ] <textfile> [<outputfile>]
options:
	d - a density to use, they are cumulative, always has single
		[ single = 72, onehalf = 108, double = 144, merged = 108 144 ]
	c - a depth to use, they are cumulative, you must specify
		at least one depth
		[ bw = 1, twobit = 2, grey = 4, color = 8, 16 = hicolor,
	          merged=1 2 4 8 16 ]
	D - debug mode
you must specify a source file of the rcp files, you do not need to
specify a destination. If you don't specify the destination output is
to stdout.
EOM
}

densities=72
colors=
gtid=
while getopts "c:d:DHh?" opt; do
	case $opt in
	d)
		trydens=
		case $OPTARG in
			single) trydens= ;; # single density is mandatory
			onehalf) trydens=108 ;;
			double) trydens=144;;
			all) trydens="108 144";;
			1) trydens=;;
			1.5) trydens=108;;
			2) trydens=144;;
                        *)
        			echo "error: density $OPTARG unknown"
        			usage
        			exit 1
                                ;;
		esac
		densities="$densities $trydens"
		;;
	c)
		trid=
		case $OPTARG in
			bw) trid=1;;
			twobit) trid=2;;
			grey) trid=4;;
			color) trid=8;;
			hicolor) trid=16;;
			merged)
				trid="1 2 4 8 16"
				gtid=1
				;;
		esac
		if [ -z "$trid" ]; then
			echo "error: color depth $OPTARG unknown"
			usage
			exit 1
		fi
		[ -n "$colors" ] && gtid=1

		colors="$colors $trid"
		;;
	D)
		debug=1
		;;
	H|h|?)
		usage
		exit 0
		;;
	esac
done

[ -z "$colors" ] && colors=1

[ -n "$debug" ] && {
	echo "densities= $densities"
	echo "colordepths= $colors"
	set -x
}

shift $((OPTIND - 1))

inputfile=$1
outputfile=$2

if [ -z "$inputfile" ]; then
	echo "error: input file not specified"
	usage
	exit 1
fi

if [ ! -r "$inputfile" ]; then
	echo "error: input file '$inputfile' does not exist"
	usage
	exit 1
fi

if [ -n "$outputfile" ]; then
	exec 1>&$outputfile
fi

cat <<EOM
// This file is automatically generated. Changes will be discarded
// modify the file $inputfile instead.

#include "simcity_resconsts.h"

EOM

sed '/^#/d' $inputfile | while read inputline; do
	OIFS="$IFS"
	IFS="|"
	set -- $inputline
	IFS="$OIFS"

	echo BITMAPFAMILYEX ID $1 COMPRESS
	echo BEGIN
	for density in $densities; do
		if [ -n "$gtid" ]; then
			for color in $colors; do
				cbmp=
				# 1 = $2, 2 = $3, 4 = $4, 8 = $5, 16 = $6
				[ $color -eq 16 ] && cbmp=$6
				[ $color -eq 8 ] && cbmp=$5
				[ $color -eq 4 ] && cbmp=$4
				[ $color -eq 2 ] && cbmp=$3
				[ $color -eq 1 ] && cbmp=$2
				[ -n "$cbmp" ] && printf "\tBITMAP \"%s\" BPP %d DENSITY %d\n" "$cbmp" $color $density
			done
		else
			for color in $colors; do
				cbmp=
				# 1 = $2, 2 = $3, 4 = $4, 8 = $5, 16 = $6
				[ $color -eq 16 ] && cbmp=$6
				[ -z "$cbmp" -o $color -eq 8 ] && cbmp=$5
				[ -z "$cbmp" -o $color -eq 4 ] && cbmp=$4
				[ -z "$cbmp" -o $color -eq 2 ] && cbmp=$3
				[ -z "$cbmp" -o $color -eq 1 ] && cbmp=$2
				printf "\tBITMAP \"%s\" BPP %d DENSITY %d\n" \
					"$cbmp" $color $density
			done
		fi
	done
	echo END
done
