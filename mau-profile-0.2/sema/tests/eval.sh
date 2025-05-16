#!/usr/bin/env bash

function rm_prev(){
    if [[ "$DATA" =~ ^0[x,X].* ]]; then
        DATA=${DATA: 2}
    fi
    # echo "::DATA => $DATA"
}

BIN_LOADER=$1

DATA=$2
rm_prev $DATA
RETDATA=$DATA
RETSIZE=$[ ${#RETDATA} / 2 ]

# tx1
Tx1_CALLVALUE=$3
DATA=$4
rm_prev $DATA
Tx1_CALLDATA=$DATA

# tx2
Tx2_CALLVALUE=$5
DATA=$6
rm_prev $DATA
Tx2_CALLDATA=$DATA

# tx3
Tx3_CALLVALUE=$7
DATA=$8
rm_prev $DATA
Tx3_CALLDATA=$DATA


# echo $BIN_LOADER ./kernel.ptx $RETSIZE $Tx1_CALLVALUE $Tx1_CALLDATA $Tx2_CALLVALUE $Tx2_CALLDATA $Tx3_CALLVALUE $Tx3_CALLDATA
echo "${Tx1_CALLVALUE} ${Tx1_CALLDATA}" > .tmpSeed
echo "${Tx2_CALLVALUE} ${Tx2_CALLDATA}" >> .tmpSeed
echo "${Tx3_CALLVALUE} ${Tx3_CALLDATA}" >> .tmpSeed

OUTPUT=$(CUDA_DEVICE_ID=4 $BIN_LOADER ./kernel.ptx .tmpSeed $RETSIZE)
echo "$OUTPUT"

LASTLINE="${OUTPUT##*$'\n'}"
if [ "$LASTLINE" = "$RETDATA" ]
then
    exit 0
else
    exit 1
fi