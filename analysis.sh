#!/bin/bash
TIMEFORMAT=%R
ITERATION=10
BUFFER_SIZE=("32" "64" "128" "256" "512")
RAW_DATA="raw"
RESULT="result"
MODE_READ_WRITE=1
MODE_MMAP_WRITE=2
MODE_SENDFILE=3
FILE_READ_WRITE="read_write"
FILE_MMAP_WRITE="mmap_write"
FILE_SENDFILE="sendfile"
DESTINATION_FILE="destinaion_file"
START=1
END=$(( ${#BUFFER_SIZE[@]}*${ITERATION}*3 ))

function progress_bar
{
    let _progress=(${1}*100/${2}*100)/100
    let _done=(${_progress}*4)/10
    let _left=40-$_done
    _done=$(printf "%${_done}s")
    _left=$(printf "%${_left}s")

    printf "\rProgress : [${_done// /#}${_left// /-}] ${_progress}%%"
}

# @param mode file_name buffer_size
function do_file_copy
{
    sum=0
    for j in $(seq 1 $ITERATION)
    do
        progress_bar ${START} ${END}
        START=$((START+1))

        TIME=$( { time ./file_copy -m ${1} -b ${3}; } 2>&1 )
        sum=$(awk "BEGIN {print $sum+$TIME; exit}")
        echo ${TIME} >> ${RAW_DATA}/${2}${3}
        rm ${DESTINATION_FILE}
        sync
    done
    avg=$(echo "$sum / $ITERATION" | bc -l | awk '{printf "%0.3f", $0}')
    echo ${3} ${avg} >> ${RAW_DATA}/${2}
}

# prepare environment
echo -e "\e[33m[Prepare environment]\e[0m"
rm -rf ${RAW_DATA}
rm -rf ${RESULT}
rm ${DESTINATION_FILE}
mkdir ${RAW_DATA}
mkdir ${RESULT}

# test parameters
echo -e "\e[33m[Test parameters]\e[0m"
echo "Buffer size: ${BUFFER_SIZE[@]}"
echo "Iteration: ${ITERATION}"

# start test
echo -e "\e[33m[Start testing]\e[0m"

start=1
end=$(( ${#BUFFER_SIZE[@]}*${ITERATION}*3 ))
for((i=0; i<${#BUFFER_SIZE[@]}; i++))
do
    # read/write
    do_file_copy ${MODE_READ_WRITE} ${FILE_READ_WRITE} ${BUFFER_SIZE[i]}

    # mmap/write
    do_file_copy ${MODE_MMAP_WRITE} ${FILE_MMAP_WRITE} ${BUFFER_SIZE[i]}

    # sendfile
    do_file_copy ${MODE_SENDFILE} ${FILE_SENDFILE} ${BUFFER_SIZE[i]}
done

# analysis
echo -e "\e[33m\n[Analyzing]\e[0m"
gnuplot result.gp
mv *.png ${RESULT}

echo -e "\e[33m[Finished]\e[0m"
