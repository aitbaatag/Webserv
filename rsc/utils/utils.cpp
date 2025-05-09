#include "../../Includes/utlis/utils.hpp"

std::string Logger::get_timestamp()
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", t);
    return std::string(buffer);
}

std::string Logger::get_time_only()
{
    time_t now = time(NULL);
    char buffer[9];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", localtime(&now));
    return std::string(buffer);
}

std::string Logger::info(const std::string &message)
{
    return Color::BLUE + Color::BOLD + "[INFO] " + Color::RESET + 
           Color::CYAN + " [" + get_timestamp() + "]" + Color::RESET + " " + 
           message + "\n";
}

void Logger::error(const std::string &message)
{
    std::cerr << Color::RED << Color::BOLD << "[ERROR]" << Color::RESET
              << Color::CYAN << " [" << get_timestamp() << "]" << Color::RESET << " "
              << Color::YELLOW << message << Color::RESET << std::endl;
}

std::string Logger::trace_http(const std::string &level, const std::string &ip,
        unsigned short port, const std::string &method, const std::string &path, const std::string &http_version,
                    std::string status_code,
                    std::string duration_ms)
{
    std::string level_str;
    if (level == "ERROR")
        level_str = Color::RED + Color::BOLD + "[" + level + "]" + Color::RESET;
    else if (level == "INFO")
        level_str = Color::BLUE + Color::BOLD + "[" + level + "]" + Color::RESET;
    else if (level == "WARN")
        level_str = Color::YELLOW + Color::BOLD + "[" + level + "]" + Color::RESET;
    else
        level_str = Color::BOLD + "[" + level + "]" + Color::RESET;
    
    std::string status_str;
    if (status_code.substr(0, 1) == "2")
        status_str = Color::GREEN + status_code + Color::RESET;
    else if (status_code.substr(0, 1) == "3")
        status_str = Color::CYAN + status_code + Color::RESET;
    else if (status_code.substr(0, 1) == "4")
        status_str = Color::YELLOW + status_code + Color::RESET;
    else if (status_code.substr(0, 1) == "5")
        status_str = Color::RED + status_code + Color::RESET;
    else
        status_str = status_code;
    
    std::string method_str;
    if (method == "GET")
        method_str = Color::GREEN + method + Color::RESET;
    else if (method == "POST")
        method_str = Color::YELLOW + method + Color::RESET;
    else if (method == "DELETE" || method == "UNKNOWN")
        method_str = Color::RED + method + Color::RESET;
    else if (method == "PUT")
        method_str = Color::YELLOW + method + Color::RESET;
    else
        method_str = method;
    
    std::ostringstream oss;
    oss << level_str << Color::CYAN << " [" << get_timestamp() << "]" << Color::RESET 
        << " " << Color::MAGENTA << ip << ":" << port << Color::RESET << " "
        << "\"" << method_str << " " << Color::BLUE << path << Color::RESET << " " << http_version << "\""
        << " → " << status_str << " " << Color::GRAY << "(" << duration_ms << "ms)" << Color::RESET;
    return oss.str() + "\n";
}

unsigned long long current_time_in_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<unsigned long long>(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}



EpollEventContext::EpollEventContext() : httpClient(NULL), serverSocket(NULL), fileDescriptor(-1)
{
    
}

EpollEventContext* EpollEventContext::createServerData(int fd, ServerSocket *server)
{
    EpollEventContext *data = new EpollEventContext();
    data->descriptorType = ServerSocketFd;
    data->serverSocket = server;
    data->fileDescriptor = fd;
	return data;
}

EpollEventContext* EpollEventContext::createClientData(int fd, HttpClient *client)
{
    EpollEventContext *data = new EpollEventContext();
    data->descriptorType = ClientSocketFd;
    data->httpClient = client;
    data->fileDescriptor = fd;
    return data;
}

EpollEventContext* EpollEventContext::createFileData(int fd, HttpClient *client)
{
    EpollEventContext *data = new EpollEventContext();
    data->descriptorType = ClientFileFd;
    data->httpClient = client;
    data->fileDescriptor = fd;
    return data;
}

void printServerBanner() {
    std::cout << "\033[2J\033[1;1H";
    std::string PEACH_COLOR = "\033[38;2;206;145;120m";
    std::cout << PEACH_COLOR << Color::BOLD << "\n\n" <<
        "     ██╗    ██╗███████╗██████╗ ███████╗███████╗██████╗ ██╗   ██╗\n" <<
        "     ██║    ██║██╔════╝██╔══██╗██╔════╝██╔════╝██╔══██╗██║   ██║\n" <<
        "     ██║ █╗ ██║█████╗  ██████╔╝███████╗█████╗  ██████╔╝██║   ██║\n" <<
        "     ██║███╗██║██╔══╝  ██╔══██╗╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝\n" <<
        "     ╚███╔███╔╝███████╗██████╔╝███████║███████╗██║  ██║ ╚████╔╝ \n" <<
        "      ╚══╝╚══╝ ╚══════╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  " << 
        Color::RESET << std::endl;
    std::cout << Color::GRAY << "                        HTTP Server Implementation" << Color::RESET << std::endl;
    std::cout << PEACH_COLOR << "                 Made by: alamaoui, kait-baa & orhaddao" << Color::RESET << std::endl;
    std::cout << std::endl;
}


void close_fd(int &fd)
{
    if (fd > 0)
    {
	    close(fd);
	    fd = -1;
    }
};

void delete_file(std::string &filename)
{
    if (!filename.empty())
    {
        if (std::remove(filename.c_str()) == 0)
        {
        }
        filename.clear();
    }
}
