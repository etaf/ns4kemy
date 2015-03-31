#!/bin/bash
monitor_obj=$1
if [ "$#" -lt 1 ]; then
    echo "Usage: ./kemy-monitor.sh monitor_obj"
    echo "only $# provided"
    exit 1
fi

${monitor_obj}
#echo $cnt
while :
do
    cnt=`ps aux | grep "$monitor_obj" | wc -l`
    if [ "$cnt" -lt 4 ]; then
        #"stoped"
        echo "stoped"
        latest_of=`ls -t result | head -n 1`
        monitor_obj="./kemy if=result/$latest_of of=result/$latest_of"
        echo $monitor_obj
        ./kemy if=result/$latest_of of=result/$latest_of
    fi
    sleep 60
done
