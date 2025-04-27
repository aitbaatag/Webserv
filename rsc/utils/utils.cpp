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
    return Color::BLUE + Color::BOLD + "[INFO]" + Color::RESET + 
           Color::CYAN + " [" + get_time_only() + "]" + Color::RESET + " " + 
           message + "\n";
}

std::string Logger::error(const std::string &message)
{
    return Color::RED + Color::BOLD + "[ERROR]" + Color::RESET + 
           Color::CYAN + " [" + get_time_only() + "]" + Color::RESET + " " + 
           Color::YELLOW + message + Color::RESET + "\n";
}

std::string Logger::trace_http(const std::string &level, const std::string &ip,
        unsigned short port, const std::string &method, const std::string &path, const std::string &http_version,
                    std::string status_code,
                    std::string duration_ms)
{
    // Level coloring
    std::string level_str;
    if (level == "ERROR")
        level_str = Color::RED + Color::BOLD + "[" + level + "]" + Color::RESET;
    else if (level == "INFO")
        level_str = Color::BLUE + Color::BOLD + "[" + level + "]" + Color::RESET;
    else if (level == "WARN")
        level_str = Color::YELLOW + Color::BOLD + "[" + level + "]" + Color::RESET;
    else
        level_str = Color::BOLD + "[" + level + "]" + Color::RESET;
    
    // Status code coloring
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
    
    // HTTP method coloring
    std::string method_str;
    if (method == "GET")
        method_str = Color::GREEN + method + Color::RESET;
    else if (method == "POST")
        method_str = Color::YELLOW + method + Color::RESET;
    else if (method == "DELETE")
        method_str = Color::RED + method + Color::RESET;
    else if (method == "PUT")
        method_str = Color::YELLOW + method + Color::RESET;
    else
        method_str = method;
    
    // Professional layout with moderate color
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