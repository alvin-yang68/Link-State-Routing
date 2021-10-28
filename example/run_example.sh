#!/bin/bash

nodes=(0 1 2 3 4 5 6 7 255) # all node ids

init () {
    # reset iptables config
    sudo iptables -F
    perl make_topology.pl testtopo.txt
}

clean () {
    rm -f ./log*   # clean up old log files
}

# loop through each node id, call `ls_router` and make it a background process
start () {
    local pids=() # all process ids

    for id in ${nodes[@]}; do
        ./ls_router $id "testinitcosts${id}" "log${id}" &   # run `ls_router` in background
        pids+=($!)    # save the pid of most recently executed background process
    done

    echo $$

    trap "kill ${pids[*]} && exit 0" SIGINT   # upon receiving ctrl+C, terminate background processes and exit the script

    while :; do sleep 2073600; done
}

linkup () {
    iptables -I OUTPUT -s 10.1.1.$1 -d 10.1.1.$2 -j ACCEPT
    iptables -I OUTPUT -s 10.1.1.$2 -d 10.1.1.$1 -j ACCEPT
}

linkdown () {
    iptables -D OUTPUT -s 10.1.1.$1 -d 10.1.1.$2 -j ACCEPT
    iptables -D OUTPUT -s 10.1.1.$2 -d 10.1.1.$1 -j ACCEPT
}

for argv in $@; do

    if [[ $argv == "linkup" ]]; then
        linkup $2 $3
        exit 0
    fi

    if [[ $argv == "linkdown" ]]; then
        linkdown $2 $3
        exit 0
    fi

    if [[ $argv == "init" ]]; then
        init
    fi

    if [[ $argv == "clean" ]]; then
        clean
    fi

    if [[ $argv == "start" ]]; then
        clean
        start
    fi

done