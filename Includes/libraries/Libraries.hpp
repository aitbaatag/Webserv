#ifndef LIBRARIES_HPP
#define LIBRARIES_HPP

#define MAX_EVENTS 10
#define MAX_RECV 20 * 1024 // 20KB
#define TIMEOUT 10
#define EPOLL_TIMEOUT 100

#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <netinet/in.h>
#include <set>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#endif
