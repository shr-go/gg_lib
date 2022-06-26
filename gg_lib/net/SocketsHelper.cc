// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#include "gg_lib/net/SocketsHelper.h"
#include "gg_lib/Logging.h"

#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/tcp.h>

static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

using namespace gg_lib;
using namespace gg_lib::net;

int sockets::createNonblockingOrDie(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }
    return sockfd;
}

int sockets::connect(int sockfd, const struct sockaddr *addr) {
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

void sockets::bindOrDie(int sockfd, const struct sockaddr *addr) {
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        LOG_SYSFATAL << "sockets::bindOrDie";
    }
}

void sockets::listenOrDie(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        LOG_SYSFATAL << "sockets::listenOrDie";
    }
}

int sockets::accept(int sockfd, struct sockaddr_in6 *addr) {
    auto addrlen = static_cast<socklen_t>(sizeof *addr);
    int connfd = ::accept4(sockfd, sockaddr_cast(addr),
                           &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
        int savedErrno = errno;
        LOG_SYSERR << "Socket::accept";
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_FATAL << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept " << savedErrno;
                break;
        }
    }
    return connfd;
}

ssize_t sockets::read(int sockfd, void *buf, size_t count) {
    return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt) {
    return ::readv(sockfd, iov, iovcnt);
}

ssize_t sockets::write(int sockfd, const void *buf, size_t count) {
    return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd) {
    if (::close(sockfd) < 0) {
        LOG_SYSERR << "sockets::close";
    }
}

void sockets::shutdownWrite(int sockfd) {
    if (::shutdown(sockfd, SHUT_WR) < 0) {
        LOG_SYSERR << "sockets::shutdownWrite";
    }
}

void sockets::toIpPort(char *buf, size_t size, const struct sockaddr *addr) {
    if (addr->sa_family == AF_INET6) {
        buf[0] = '[';
        toIp(buf + 1, size - 1, addr);
        size_t end = ::strlen(buf);
        const struct sockaddr_in6 *addr6 = sockaddr_in6_cast(addr);
        uint16_t port = sockets::networkToHost16(addr6->sin6_port);
        assert(size > end);
        snprintf(buf + end, size - end, "]:%u", port);
    } else {
        toIp(buf, size, addr);
        size_t end = ::strlen(buf);
        const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
        uint16_t port = sockets::networkToHost16(addr4->sin_port);
        assert(size > end);
        snprintf(buf + end, size - end, ":%u", port);
    }
}

void sockets::toIp(char *buf, size_t size,
                   const struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    } else if (addr->sa_family == AF_INET6) {
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6 *addr6 = sockaddr_in6_cast(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

void sockets::fromIpPort(const char *ip, uint16_t port, struct sockaddr_in *addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
        LOG_SYSERR << "sockets::fromIpPort";
    }
}

void sockets::fromIpPort(const char *ip, uint16_t port,
                         struct sockaddr_in6 *addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = hostToNetwork16(port);
    if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
        LOG_SYSERR << "sockets::fromIpPort";
    }
}

int sockets::getSocketError(int sockfd) {
    int optval;
    auto optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

const struct sockaddr *sockets::sockaddr_cast(const struct sockaddr_in *addr) {
    return reinterpret_cast<const struct sockaddr *>(addr);
}

const struct sockaddr *sockets::sockaddr_cast(const struct sockaddr_in6 *addr) {
    return reinterpret_cast<const struct sockaddr *>(addr);
}

struct sockaddr *sockets::sockaddr_cast(struct sockaddr_in6 *addr) {
    return reinterpret_cast<struct sockaddr *>(addr);
}

const struct sockaddr_in *sockets::sockaddr_in_cast(const struct sockaddr *addr) {
    return reinterpret_cast<const struct sockaddr_in *>(addr);
}

const struct sockaddr_in6 *sockets::sockaddr_in6_cast(const struct sockaddr *addr) {
    return reinterpret_cast<const struct sockaddr_in6 *>(addr);
}

struct sockaddr_in6 sockets::getLocalAddr(int sockfd) {
    struct sockaddr_in6 localaddr{};
    auto addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
        LOG_SYSERR << "sockets::getLocalAddr";
    }
    return localaddr;
}

struct sockaddr_in6 sockets::getPeerAddr(int sockfd) {
    struct sockaddr_in6 peeraddr{};
    auto addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0) {
        LOG_SYSERR << "sockets::getPeerAddr";
    }
    return peeraddr;
}

bool sockets::isSelfConnect(int sockfd) {
    struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
    if (localaddr.sin6_family == AF_INET) {
        const struct sockaddr_in *laddr4 = reinterpret_cast<struct sockaddr_in *>(&localaddr);
        const struct sockaddr_in *raddr4 = reinterpret_cast<struct sockaddr_in *>(&peeraddr);
        return laddr4->sin_port == raddr4->sin_port
               && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    } else if (localaddr.sin6_family == AF_INET6) {
        return localaddr.sin6_port == peeraddr.sin6_port
               && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
    } else {
        return false;
    }
}


InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6) {
    if (ipv6) {
        bzero(&addr6_, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = sockets::hostToNetwork16(port);
    } else {
        bzero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
        addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
        addr_.sin_port = sockets::hostToNetwork16(port);
    }
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6) {
    if (ipv6 || strchr(ip.c_str(), ':')) {
        bzero(&addr6_, sizeof addr6_);
        sockets::fromIpPort(ip.c_str(), port, &addr6_);
    } else {
        bzero(&addr_, sizeof addr_);
        sockets::fromIpPort(ip.c_str(), port, &addr_);
    }
}

string InetAddress::toIp() const {
    char buf[64] = "";
    sockets::toIp(buf, sizeof buf, getSockAddr());
    return buf;
}

string InetAddress::toIpPort() const {
    char buf[64] = "";
    sockets::toIpPort(buf, sizeof buf, getSockAddr());
    return buf;
}

uint16_t InetAddress::port() const {
    return sockets::networkToHost16(portNetEndian());
}

uint32_t InetAddress::ipv4NetEndian() const {
    assert(family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

static __thread char t_resolveBuffer[64 * 1024];

bool InetAddress::resolve(StringArg hostname, InetAddress *result) {
    assert(result != nullptr);
    struct hostent hent{};
    struct hostent *he = nullptr;
    int herrno = 0;

    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer,
                              sizeof t_resolveBuffer, &he, &herrno);
    if (ret == 0 && he != nullptr) {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        result->addr_.sin_addr = *reinterpret_cast<struct in_addr *>(he->h_addr);
        return true;
    } else {
        if (ret) {
            LOG_SYSERR << "InetAddress::resolve";
        }
        return false;
    }
}

void InetAddress::setScopeId(uint32_t scope_id) {
    if (family() == AF_INET6) {
        addr6_.sin6_scope_id = scope_id;
    }
}

Socket::~Socket() {
    if (sockfd_ >= 0) {
        sockets::close(sockfd_);
    }
}

bool Socket::getTcpInfo(struct tcp_info *info) const {
    socklen_t len = sizeof(*info);
    bzero(info, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, info, &len) == 0;
}

bool Socket::getTcpInfoString(char *buf, int len) const {
    struct tcp_info info;
    bool ok = getTcpInfo(&info);
    if (ok) {
        snprintf(buf, len, "unrecovered=%u rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                           "lost=%u retrans=%u rtt=%u rttvar=%u sshthresh=%u cwnd=%u total_retrans=%u",
                 info.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
                 info.tcpi_rto,          // Retransmit timeout in usec
                 info.tcpi_ato,          // Predicted tick of soft clock in usec
                 info.tcpi_snd_mss,
                 info.tcpi_rcv_mss,
                 info.tcpi_lost,         // Lost packets
                 info.tcpi_retrans,      // Retransmitted packets out
                 info.tcpi_rtt,          // Smoothed round trip time in usec
                 info.tcpi_rttvar,       // Medium deviation
                 info.tcpi_snd_ssthresh,
                 info.tcpi_snd_cwnd,
                 info.tcpi_total_retrans);  // Total retransmits for entire connection
    }
    return ok;
}

void Socket::bindAddress(const InetAddress &localaddr) const {
    sockets::bindOrDie(sockfd_, localaddr.getSockAddr());
}

void Socket::listen() const {
    sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress *peeraddr) const {
    struct sockaddr_in6 addr{};
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0) {
        peeraddr->setSockAddrInet6(addr);
    }
    return connfd;
}

void Socket::close() const {
    sockets::close(sockfd_);
}

void Socket::shutdownWrite() const {
    sockets::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on) const {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReuseAddr(bool on) const {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReusePort(bool on) const {
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    }
#else
    if (on) {
        LOG_ERROR << "SO_REUSEPORT is not supported.";
    }
#endif
}

void Socket::setKeepAlive(bool on) const {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                 &optval, static_cast<socklen_t>(sizeof optval));
}
