#ifndef UTILS_HPP
# define UTILS_HPP

#include "../libraries/Libraries.hpp"
#include "../server/server_socket.hpp"



class Logger
{
	public:
		static std::string get_timestamp();
		static std::string get_time_only();
		static std::string info(const std::string &message);
		static std::string error(const std::string &message);
		static std::string trace_http(const std::string &level, const std::string &ip,
                                    	unsigned short port, const std::string &method,
                                    	const std::string &path, const std::string &http_version,
						            	std::string status_code, std::string duration_ms);
};


unsigned long long current_time_in_ms();

template <typename T>
std::string to_string(const T& value)
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}



enum FileDescriptorType
{
	ServerSocketFd,
	ClientSocketFd,
	ClientFileFd
};

struct EpollEventContext
{
	FileDescriptorType	descriptorType;
	HttpClient			*httpClient;
	ServerSocket		*serverSocket;
	int					fileDescriptor;

	EpollEventContext();

	static EpollEventContext* createServerData(int	fd, ServerSocket	*server);
	static EpollEventContext* createClientData(int	fd, HttpClient		*client);
	static EpollEventContext* createFileData(int	fd, HttpClient		*client);
};


void printServerBanner();

void close_fd(int &fd);
void delete_file(std::string &filename);


#endif