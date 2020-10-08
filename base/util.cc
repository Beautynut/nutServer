
#include "util.h"
#include "Logging.h"



namespace nut{
    const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr)
    {
        return static_cast<const struct sockaddr*>(boost::implicit_cast<const void*>(addr));
    }

    struct sockaddr* sockaddr_cast(struct sockaddr_in* addr)
    {
        return static_cast<struct sockaddr*>(boost::implicit_cast<void*>(addr));
    }

    void toHostPort(char* buf, size_t size,
                         const struct sockaddr_in& addr)
    {
        char host[INET_ADDRSTRLEN] = "INVALID";
        ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof host);
        uint16_t port = ntohs(addr.sin_port);
        snprintf(buf, size, "%s:%u", host, port);
    }

    void fromHostPort(const char* ip, uint16_t port,
                           struct sockaddr_in* addr)
    {
        addr->sin_family = AF_INET;
        addr->sin_port = htons(port);
        if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
        {
            LOG << "sockets::fromHostPort";
        }
    }

    struct sockaddr_in getLocalAddr(int sockfd)
    {
        struct sockaddr_in localaddr;
        bzero(&localaddr, sizeof localaddr);
        socklen_t addrlen = sizeof(localaddr);
        if (::getsockname(sockfd, nut::sockaddr_cast(&localaddr), &addrlen) < 0)
        {
            LOG << "sockets::getLocalAddr";
        }
        return localaddr;
    }

    int getSocketError(int sockfd)
    {
        int optval;
        socklen_t optlen = sizeof optval;

        if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
        {
            return errno;
        }
        else
        {
            return optval;
        }
    }
}
