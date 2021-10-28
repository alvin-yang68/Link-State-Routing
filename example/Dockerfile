FROM ubuntu:20.04 AS focal

RUN apt update && apt-get install -y build-essential python python3 zip net-tools iptables sudo curl

COPY . /usr/src/ls_router
WORKDIR /usr/src/ls_router

# set up a virtual network topology on eth0 network interface
RUN ./run_example.sh init clean

CMD ['./run_example.sh', 'start']