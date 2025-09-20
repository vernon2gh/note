#!/bin/bash

if [ $# -eq 0 ]; then
	echo "$0 index"
	echo "   index range is 0~17, reference https://lore.kernel.org/lkml/_/text/mirror"
	exit
fi

export INDEX=$1
export MIRROR_DIR=$(pwd)/git/${INDEX}.git
export MBX_DIR=$(pwd)/mbx/$INDEX

function mbx_one() {
	COMMIT_SHA=$1
	BLOB_SHA=$(git ls-tree $COMMIT_SHA | awk '{print $3}')
	MBXFILE=$(git log --pretty="format:%ai_%an_%s" -1 $COMMIT_SHA |								\
		  sed 's/[[:space:]]\{1,\}/_/g; s/\[//g; s/\]//g; s/\//_/g; s/(//g; s/)//g; s/+//g; s/-//g; s/://g; s/,//g' |	\
		  tr -cd '\000-\177')
	MBXFILE=${MBXFILE:0:200}.mbx

	if [ ! -e $MBX_DIR/$MBXFILE ]; then
		git cat-file blob $BLOB_SHA > $MBX_DIR/$MBXFILE
	fi
}
export -f mbx_one

if [ ! -d "$MIRROR_DIR" ]; then
	git clone --mirror https://lore.kernel.org/lkml/$INDEX $MIRROR_DIR
fi
mkdir -p $MBX_DIR

cd $MIRROR_DIR
git fetch origin
git rev-list --all | xargs -n 1 -P $(nproc) bash -c 'mbx_one $1' _
cd -
