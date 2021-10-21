#!/bin/bash
make clean
make all

# Rest iptables config
sudo iptables -F

perl make_topology.pl testtopo.txt

./ls_router 0 testinitcosts0 log0
./ls_router 1 testinitcosts1 log1
./ls_router 2 testinitcosts2 log2
./ls_router 3 testinitcosts3 log3
./ls_router 4 testinitcosts4 log4
./ls_router 5 testinitcosts5 log5
./ls_router 6 testinitcosts6 log6
./ls_router 7 testinitcosts7 log7
./ls_router 255 testinitcosts255 log255