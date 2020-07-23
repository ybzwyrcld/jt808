// MIT License
//
// Copyright (c) 2020 Yuming Meng
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// @File    :  socket_util.h
// @Version :  1.0
// @Time    :  2020/07/17 10:13:15
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#ifndef JT808_SOCKET_UTIL_H_
#define JT808_SOCKET_UTIL_H_

#if defined(__linux__)
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#elif defined(_WIN32) || defined(__WIN64)
#include <winsock2.h>
#include <windows.h>
#endif


namespace libjt808 {

// connect.
template<typename T>
inline int Connect(T s, const struct sockaddr* name, int namelen) {
  return -1;
}
#if defined(__linux__)
inline int Connect(int fd, const struct sockaddr* name, int namelen) {
  return connect(fd, name, static_cast<socklen_t>(namelen));
}
#elif defined(_WIN32)
template<>
inline int Connect(SOCKET s, const struct sockaddr* name, int namelen) {
  return connect(s, name, namelen);
}
#endif

// bind.
template<typename T>
inline int Bind(T s, const struct sockaddr* name, int namelen) {
  return -1;
}
#if defined(__linux__)
inline int Bind(int fd, const struct sockaddr* name, int namelen) {
  return bind(fd, name, static_cast<socklen_t>(namelen));
}
#elif defined(_WIN32)
template<>
inline int Bind(SOCKET s, const struct sockaddr* name, int namelen) {
  return bind(s, name, namelen);
}
#endif

// listen.
template<typename T>
inline int Listen(T s, int backlog) {
  return -1;
}
#if defined(__linux__)
inline int Listen(int fd, int backlog) {
  return listen(fd, backlog);
}
#elif defined(_WIN32)
template<>
inline int Listen(SOCKET s, int backlog) {
  return listen(s, backlog);
}
#endif

// accept.
template<typename T>
inline int Accept(T s, struct sockaddr *addr, int *addrlen) {
  return -1;
}
#if defined(__linux__)
inline int Accept(int fd, struct sockaddr *addr, int *addrlen) {
  return accept(fd, addr, reinterpret_cast<socklen_t*>(addrlen));
}
#elif defined(_WIN32)
template<>
inline int Accept(SOCKET s, struct sockaddr *addr, int *addrlen) {
  return accept(s, addr, addrlen);
}
#endif

// send.
template<typename T>
inline int Send(T s, const char* buf, int len, int flags) {
  return -1;
}
#if defined(__linux__)
inline int Send(int fd, const char* buf, int len, int flags) {
  return send(fd, buf, len, flags);
}
#elif defined(_WIN32)
template<>
inline int Send(SOCKET s, const char* buf, int len, int flags) {
  return send(s, buf, len, flags);
}
#endif

// recv.
template<typename T>
inline int Recv(T s, char* buf, int len, int flags) {
  return -1;
}
#if defined(__linux__)
inline int Recv(int fd, char* buf, int len, int flags) {
  return recv(fd, buf, len, flags);
}
#elif defined(_WIN32)
template<>
inline int Recv(SOCKET s, char* buf, int len, int flags) {
  return recv(s, buf, len, flags);
}
#endif

// close.
template<typename T>
inline int Close(T s) {
  return -1;
}
#if defined(__linux__)
template<>
inline int Close(int fd) {
  return close(fd);
}
#elif defined(_WIN32)
template<>
inline int Close(SOCKET s) {
  return closesocket(s);
}
#endif

}  // namespace libjt808

#endif  // JT808_SOCKET_UTIL_H_
