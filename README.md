# Link-state routing protocol in C++

## Background

This is a C++ implementation of the [link-state protocol](https://en.wikipedia.org/wiki/Link-state_routing_protocol), a protocol used to plan the shortest paths across a network. The protocol consists of two parts: reliable flooding algorithm and shortest paths computation.

Reliable flooding is a gossip algorithm used to share LSAs (Link State Advertisements) among routers. Each LSA contains information about a node's neighbors and the costs to them. This is done so that eventually every router will be aware of the link costs between arbitrary nodes. The reliable flooding is also performed every time a link cost changes or some node disconnected or reconnected into the network. In order to ensure each node only has the latest news, an LSA carries with it a sequence number that serves to keep track of its version.

Once each router obtains a global view of the entire network, it runs [Dijkstra's algorithm](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm) to compute the shortest paths to every other routers in the network. This allows a node to find the next hop router it should forward a packet to given that the packet is destined to certain router in the network.

## Running on the example topology

![testtopo](https://user-images.githubusercontent.com/72721378/139183748-183f3c2e-c712-42ce-86f0-e80668caee83.png)

1. Run the docker image detached: `docker run -d --rm --cap-add=NET_ADMIN --name link-state alvinyang68/linkstate` (download may take a few minutes).
2. Execute `docker exec -it link-state /bin/bash` to attach an interactive bash shell on the running container.
3. Execute `./manager_send <node_id1> send <node_id2> <message>` to send a message from one node to another. For example, `./manager_send 6 send 3 hello` will make node 6 sends "hello" to node 3.
4. Inspect the generated log file for a node by executing `cat log<node_id>`, e.g. `cat log1` to inspect the log file of node 1.
5. Change the link cost between two neighboring nodes via `./manager_send <node_id1> cost <node_id2> <new_cost>`.

To bring up/down a link between any two nodes while running the program, execute `./run_example.sh linkup <node_id1> <node_id2>` and `./run_example.sh linkdown <node_id1> <node_id2>`, respectively.
