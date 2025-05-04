#ifndef LIBRARIES_HPP
#define LIBRARIES_HPP

#define MAX_EVENTS 25
#define MAX_RECV 1024 * 1024 * 5
#define MAX_SEND 8192
#define TIMEOUT 20
#define EPOLL_TIMEOUT 0

#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <fcntl.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <set>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include <ctime>
#include <iostream>
#include <string>

#include <ctime>
#include <sstream>
#include <string>

namespace Color {
const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string GRAY = "\033[90m";
const std::string BOLD = "\033[1m";
} // namespace Color

#endif
