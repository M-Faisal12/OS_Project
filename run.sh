#!/bin/bash
trap 'echo "Bye"; cleanup' INT
trap 'echo "Bye"; cleanup' EXIT
########################################################
########################################################
output="report"
thread=2
size=10
data="data"
while getopts "D:T:S:O:" opt; do
  case $opt in
    D) data="$OPTARG" ;;
    T) thread="$OPTARG" ;;
    S) size="$OPTARG" ;;
    O) output="$OPTARG" ;;
    ?) echo "Invalid option"; exit 1 ;;
  esac
done
##################################
PID_FILE="dispatcher.pid"
cleanup() {
    make clean
    if [[ -f "$PID_FILE" ]]; then
        PID=$(cat "$PID_FILE")
        if kill -0 "$PID" 2>/dev/null; then
            echo "Killing process $PID"
            kill "$PID"
            sleep 1
            kill -9 "$PID" 2>/dev/null
        fi
        rm -f "$PID_FILE"
    fi
}
##################################
check1(){
  c=$(g++ --version | grep "command not found" | wc -l)
  if [[ $c != 0 ]]; then
  echo "g++ is not installed !!"
  echo "Stopping process !!!!"
  exit 1
  fi
  c=$(make --version | grep "command not found" | wc -l)
  if [[ $c != 0 ]]; then
  echo "Make is not installed !!"
  echo "Stopping process !!!!"
  exit 1
  fi
}
##################################
check2(){
folders=$(ls | grep "$output")
if [ "$folders" = "$output" ]; then
  echo "Found output folder"
else
  echo "Folder not found"
  mkdir $output
  echo "Folder created for output reports"
fi

if [ "$thread" -le 0 ]; then
  echo "Thread count set to default value of 2"
  thread=2
fi
if [ "$size" -le 0 ]; then
  echo "Queue size set to default value of 10"
  size=10
fi

folders=$(ls | grep "$data")
if [ "$folders" = "$data" ]; then
  echo "Found data folder"
else
  echo "Folder not found"
  exit 1
fi
}
########################################################
check1
check2
########################################################

count=$(ls ./"$data" | grep '.csv' | wc -l)
if [[ $count -eq 0 ]]; then
  echo "No CSV files found in $data folder"
  echo "Stopping process !!!!"
  exit 1  
else
  files=$(ls ./"$data" | grep '.csv' | tr '\n' ',')
  make main Folder="$data" Files="$files" Threads="$thread" Size="$size" Output="$output" 
  if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
  fi
  echo "Build successful!"

  start=$(date +%s.%N)

  ./dispatcher "$data" "$files" "$thread" "$size" "$output" 
  
  echo $! > dispatcher.pid
  status=$?
  end=$(date +%s.%N)
  runtime=$(echo "$end - $start" | bc)

  records=$(wc -l < "$output"/Report.csv)
  echo "Total runtime: ${runtime} seconds"
  echo "Records processed: ${records}"
  echo "Exit status: ${status}"
# wait
fi

