#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <string>
#include <vector>

#include <systemd/sd-daemon.h>

#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace {
const std::vector<int> need_ports{8000, 8001, 8002};
}

int main(int argc, char **argv) {
  // If possible, use the systemd listen socket
  std::cout << "in main" << std::endl;
  std::vector<int> have_ports;
  const int fd_count = sd_listen_fds(0);
  std::cout << "fd_count = " << fd_count << std::endl;
  for (int i = 0; i < fd_count; i++) {
    const int fd = SD_LISTEN_FDS_START + i;
    sockaddr_in in_addr;
    socklen_t socklen = sizeof(in_addr);
    if (getsockname(fd, (sockaddr *)&in_addr, &socklen)) {
      std::cerr << "getsockname(): " << strerror(errno) << std::endl;
      return 1;
    }
    const int host_port = static_cast<int>(ntohs(in_addr.sin_port));
    have_ports.push_back(host_port);
  }
  for (const auto &need : need_ports) {
    bool have = false;
    for (const auto &port : have_ports) {
      if (need == port) {
        have = true;
        break;
      }
    }
    if (have) {
      continue;
    }
    std::cout << "binding " << need << std::endl;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
      std::cerr << "socket(): " << strerror(errno) << std::endl;
      return 1;
    }
    sockaddr_in in_addr;
    memset(&in_addr, 0, sizeof(in_addr));
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(need);
    in_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (sockaddr *)&in_addr, sizeof(in_addr))) {
      std::cerr << "bind(): " << strerror(errno) << std::endl;
      return 1;
    }
  }

  fd_set fds;
  FD_ZERO(&fds);
  select(0, &fds, &fds, &fds, nullptr);
  return 0;
}
