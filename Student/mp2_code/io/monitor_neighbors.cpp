#include "monitor.hpp"

// Yes, this is terrible. It's also terrible that, in Linux, a socket
// can't receive broadcast packets unless it's bound to INADDR_ANY,
// which we can't do in this assignment.
void hackyBroadcast(const char *buf, int length)
{
    int i;
    for (i = 0; i < 256; i++)
        if (i != globalMyID) //(although with a real broadcast you would also get the packet yourself)
            sendto(globalSocketUDP, buf, length, 0,
                   (struct sockaddr *)&globalNodeAddrs[i], sizeof(globalNodeAddrs[i]));
}
