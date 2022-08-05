// Copyright (c) 2022 shr-go. All rights reserved.
// Author: shr-go

#ifndef GG_LIB_SOCKETSHELPER_H
#define GG_LIB_SOCKETSHELPER_H

#include "gg_lib/copyable.h"
#include "gg_lib/noncopyable.h"
#include "gg_lib/Utils.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <endian.h>


struct tcp_info;

namespace gg_lib {
    namespace net {
        /// @brief Socket operation wrapper
        namespace sockets {
            int createNonblockingOrDie(sa_family_t family);

            int connect(int sockfd, const struct sockaddr *addr);

            void bindOrDie(int sockfd, const struct sockaddr *addr);

            void listenOrDie(int sockfd);

            int accept(int sockfd, struct sockaddr_in6 *addr);

            ssize_t read(int sockfd, void *buf, size_t count);

            ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);

            ssize_t write(int sockfd, const void *buf, size_t count);

            void close(int sockfd);

            void shutdownWrite(int sockfd);

            void toIpPort(char *buf, size_t size,
                          const struct sockaddr *addr);

            void toIp(char *buf, size_t size,
                      const struct sockaddr *addr);

            void fromIpPort(const char *ip, uint16_t port,
                            struct sockaddr_in *addr);

            void fromIpPort(const char *ip, uint16_t port,
                            struct sockaddr_in6 *addr);

            int getSocketError(int sockfd);

            const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr);

            const struct sockaddr *sockaddr_cast(const struct sockaddr_in6 *addr);

            struct sockaddr *sockaddr_cast(struct sockaddr_in6 *addr);

            const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr);

            const struct sockaddr_in6 *sockaddr_in6_cast(const struct sockaddr *addr);

            struct sockaddr_in6 getLocalAddr(int sockfd);

            struct sockaddr_in6 getPeerAddr(int sockfd);

            bool isSelfConnect(int sockfd);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"

            inline uint64_t hostToNetwork64(uint64_t host64) {
                return htobe64(host64);
            }

            inline uint32_t hostToNetwork32(uint32_t host32) {
                return htobe32(host32);
            }

            inline uint16_t hostToNetwork16(uint16_t host16) {
                return htobe16(host16);
            }

            inline uint64_t networkToHost64(uint64_t net64) {
                return be64toh(net64);
            }

            inline uint32_t networkToHost32(uint32_t net32) {
                return be32toh(net32);
            }

            inline uint16_t networkToHost16(uint16_t net16) {
                return be16toh(net16);
            }

#pragma GCC diagnostic pop
        }

        /// @brief Wrapper of sockaddr_in.
        class InetAddress : public copyable {
        public:
            explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);

            InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);

            explicit InetAddress(const struct sockaddr_in &addr)
                    : addr_(addr) {}

            explicit InetAddress(const struct sockaddr_in6 &addr)
                    : addr6_(addr) {}

            sa_family_t family() const { return addr_.sin_family; }

            string toIp() const;

            string toIpPort() const;

            uint16_t port() const;

            const struct sockaddr *getSockAddr() const { return sockets::sockaddr_cast(&addr6_); }

            void setSockAddrInet6(const struct sockaddr_in6 &addr6) { addr6_ = addr6; }

            uint32_t ipv4NetEndian() const;

            uint16_t portNetEndian() const { return addr_.sin_port; }

            static bool resolve(StringArg hostname, InetAddress *result);

            void setScopeId(uint32_t scope_id);

        private:
            union {
                struct sockaddr_in addr_;
                struct sockaddr_in6 addr6_;
            };
        };

        ///
        /// @brief Wrapper of socket file descriptor.
        ///
        class Socket : noncopyable {
        public:
            explicit Socket(int sockfd)
                    : sockfd_(sockfd) {}

            Socket(Socket &&rhs) noexcept {
                sockfd_ = rhs.sockfd_;
                rhs.sockfd_ = -1;
            }

            ~Socket();

            int fd() const { return sockfd_; }

            bool getTcpInfo(struct tcp_info *info) const;

            bool getTcpInfoString(char *buf, int len) const;

            /// abort if address in use
            void bindAddress(const InetAddress &localaddr) const;

            /// abort if address in use
            void listen() const;

            /// On success, returns a non-negative integer that is
            /// a descriptor for the accepted socket, which has been
            /// set to non-blocking and close-on-exec. *peeraddr is assigned.
            /// On error, -1 is returned, and *peeraddr is untouched.
            int accept(InetAddress *peeraddr) const;

            void close() const;

            void shutdownWrite() const;

            void setTcpNoDelay(bool on) const;

            void setTcpCork(bool on) const;

            void setReuseAddr(bool on) const;

            void setReusePort(bool on) const;

            void setKeepAlive(bool on) const;

        private:
            int sockfd_;
        };
    }
}

#endif //GG_LIB_SOCKETSHELPER_H
