#!/bin/bash

# nodes=(0 1 2 3 4 5 6 7 255) # all node ids
nodes=(1)

init () {
    # reset iptables config
    sudo iptables -F

    perl make_topology.pl testtopo.txt
}

# loop through each node id, call `ls_router` and make it a background process
start () {
    rm -f ./log*   # clean up old log files

    local pids=() # all process ids

    for id in ${nodes[@]}; do
        ./ls_router $id "testinitcosts${id}" "log${id}" &   # run `ls_router` in background
        pids+=($!)    # save the pid of most recently executed background process
    done

    echo "pids: ${pids[@]}"
}

if [[ $1 == "init" ]]; then
    init
fi

if [[ $1 == "start" ]]; then
    start
fi
