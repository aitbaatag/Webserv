#ifndef CONFIG_PARSER_HPP
# define CONFIG_PARSER_HPP

#include <cstring>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ctime>
#include <cctype>
#include <iostream>
#include <vector>
#include <string>
#include <map>

void printTokens(const std::vector<std::string > &tokens, std::size_t i);

//emerg: System is unusable (e.g., "cannot open config file").
//alert: Immediate action needed (e.g., "disk full").
//crit: Critical condition (e.g., "database crashed").
//error: General error (e.g., "invalid directive").
//warn: Warning (e.g., "deprecated feature used").

enum LogLevel {
	EMERG,
	WARNING,
	ERROR,
	DEBUG
};

// Track the server 
struct ServerFoundDirective {
	bool has_port;
	bool has_host;
	bool has_is_default;
	bool has_max_body_size;
	ServerFoundDirective();
};

// Track the route
struct RouteFoundDirective {
	bool has_path;
	bool has_redirect;
	bool has_root_dir;
	bool has_directory_listing;
	bool has_default_file;
	bool has_cgi_extension;
	bool has_upload_dir;
	RouteFoundDirective();
};


// Represents an HTTP route
struct Route
{
	std::string path;
	std::string redirect;
	std::string root_dir;
	bool directory_listing;
	std::string default_file;
	std::string cgi_extension;
	std::string upload_dir;
	std::vector<std::string> accepted_methods;
	RouteFoundDirective Tracker;
	Route();
};




// Represents a server block
struct Server
{
	int port;
	std::string host;
	std::vector<std::string> server_names;
	bool is_default;
	std::map<std::string, std::string> error_page;
	std::size_t max_body_size;
	std::vector<Route> routes;
	ServerFoundDirective Tracker;
	Server();
};






class ServerConfigParser {
	private:
		std::ifstream file_;
		std::size_t lineCounter_;
		std::string currentLine_;
		int openingBraceCount_;
		int closingBraceCount_;
		bool insideServerBlock_;
		bool insideRouteBlock_;
		std::vector<Server> servers_;

		// std::size_t current_server_;
		// std::vector <Route> router_;
		// Server &current_server_;
		// Route &current_route_;
	public:
		
		// contructor & deconstructor & public func
		ServerConfigParser();
		~ServerConfigParser();
		std::string formatErrorMessage(LogLevel level, const std::string &message);

		//parsing tokenize
		void loadConfigFile(int argc, char* argv[]);
		std::vector<std::string > tokenizeConfigLine();
		std::ifstream& getConfigFileStream();
		void parseConfigLine();
		void parseDirective(std::vector<std::string> &lineTokens);
		void parseConfigFile(int argc, char* argv[]);
		
		// Server Block Part
		void parseServer(std::vector<std::string> &lineTokens, std::size_t &idx);              // Parse "server" block
		void parseListen(std::vector<std::string> &lineTokens, std::size_t &idx);              // Parse "listen" directive @server
		void parseHost(std::vector<std::string> &lineTokens, std::size_t &idx);                // Parse "host" directive @server
		void parseServerName(std::vector<std::string> &lineTokens, std::size_t &idx);          // Parse "server_name" directive @server
		void parseMaxBodySize(std::vector<std::string> &lineTokens, std::size_t &idx);         // Parse "max_body_size" directive @server
		void parseErrorPage(std::vector<std::string> &lineTokens, std::size_t &idx);           // Parse "error_page" directive @server

		// Route Block Part
		void parseRoute(std::vector<std::string> &lineTokens, std::size_t &idx);               // Parse "route" block 
		void parseMethods(std::vector<std::string> &lineTokens, std::size_t &idx);             // Parse "methods" directive @route
		void parseRoot(std::vector<std::string> &lineTokens, std::size_t &idx);                // Parse "root" directive @route
		void parseDirectoryListing(std::vector<std::string> &lineTokens, std::size_t &idx);    // Parse "directory_listing" directive @route
		void parseDefaultFile(std::vector<std::string> &lineTokens, std::size_t &idx);         // Parse "default_file" directive @route
		void parseCgi(std::vector<std::string> &lineTokens, std::size_t &idx);                 // Parse "cgi" directive @route
		void parseUploadDir(std::vector<std::string> &lineTokens, std::size_t &idx);           // Parse "upload_dir" directive @route
		void parseRedirect(std::vector<std::string> &lineTokens, std::size_t &idx);           // Parse "upload_dir" directive @route

		// Beautiful printer function
		void printServers() const;
};

#endif