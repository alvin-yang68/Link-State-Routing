# Link-state routing protocol in C++

This is a C++ implementation of the [link-state protocol](https://en.wikipedia.org/wiki/Link-state_routing_protocol), a protocol used to plan the shortest paths across a network. The protocol consists of two parts: reliable flooding algorithm and shortest paths computation. 

Reliable flooding is a gossip algorithm used to share LSAs (Link State Advertisements) among routers. Each LSA contains information about a node's neighbors and the costs to them. This is done so that eventually every router will be aware of the link costs between arbitrary nodes. The reliable flooding is also performed every time a link cost changes or some node disconnected or reconnected into the network. In order to ensure each node only has the latest news, an LSA carries with it a sequence number that serves to keep track of its version.

Once each router obtains a global view of the entire network, it runs [Dijkstra's algorithm](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm) to compute the shortest paths to every other routers in the network. This allows a node to find the next hop router it should forward a packet to given that the packet is destined to certain router in the network.

## Running on the example topology

![testtopo](https://user-images.githubusercontent.com/72721378/139183748-183f3c2e-c712-42ce-86f0-e80668caee83.png)

1. `cd` to the `example/` directory
2. Build the Dockerfile and run the image detached: `docker build -t <image_name> .` and `docker run -d --rm <image_name>`
3. Run `docker exec -it <container name> /bin/bash` to attach an interactive bash shell on the running container.
4. Execute `./manager_send <node_id1> send <node_id2> <message>` to send a message from node 1 to node 2.
5. Inspect the generated log files for node 1 and node 2 by running `cat log1` and `cat log2`, respectively.

To bring up/down a link between any two nodes while running the program, execute `./sudo run_example.sh linkup <node_id1> <node_id2>` and `./sudo run_example.sh linkdown <node_id1> <node_id2>`, respectively.
