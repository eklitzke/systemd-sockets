#include <cstdio>
#include <cstring>

#include <iostream>
#include <string>
#include <vector>

#include <getopt.h>
#include <systemd/sd-daemon.h>

#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace {
const std::vector<int> need_ports{8000, 8001, 8002};
}

int main(int argc, char **argv) {
  int ports = 2;
  const std::string interface = "0.0.0.0";
  for (;;) {
    static struct option long_options[] = {{"ports", required_argument, 0, 'p'},
                                           {0, 0, 0, 0}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "p:", long_options, &option_index);
    if (c == -1) {
      break;
    }
    switch (c) {
    case 0:
      if (long_options[option_index].flag != 0) {
        // if the option set a flag, do nothing
        break;
      }
      break;
    case 'p':
      ports = static_cast<int>(std::strtol(optarg, nullptr, 10));
      break;
    case '?':
      // getopt_long should already have printed an error message
      break;
    default:
      abort();
    }
  }
  if (ports < 0) {
    std::cerr << "negative ports not allowed\n";
  }
  if (ports > 2) {
    std::cerr << "that's too many ports\n";
  }

  // If possible, use the systemd listen socket
  std::vector<int> have_ports;
  const int fd_count = sd_listen_fds(0);
  for (int i = 0; i < fd_count; i++) {
    const int fd = SD_LISTEN_FDS_START + i;
    sockaddr_in in_addr;
    socklen_t socklen;
    if (getsockname(fd, (sockaddr *)&in_addr, &socklen)) {
      perror("getsockname()");
      return 1;
    }
    const int host_port = static_cast<int>(ntohl(in_addr.sin_port));
    std::cout << "have port " << host_port << "\n";
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
    std::cout << "binding " << need << "\n";
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
      perror("socket()");
    }
    sockaddr_in in_addr;
    memset(&in_addr, 0, sizeof(in_addr));
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(need);
    in_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (sockaddr *)&in_addr, sizeof(in_addr))) {
      perror("bind()");
    }
  }

  fd_set fds;
  FD_ZERO(&fds);
  select(0, &fds, &fds, &fds, nullptr);
  return 0;
}
