#include "../../Includes/Config/Config.hpp"
#include "../../Includes/server/server_socket.hpp"
class Logger;
// ----------------------------------- PARSING CONFIG --------------------------------- //

void ServerConfigParser::loadConfigFile(int argc, char *argv[])
{
	std::ostringstream errMsg;

	if (argc != 2)
	{
		errMsg << "invalid usage: ./" << argv[0] << "<config_file>";
		throw std::runtime_error(Logger::error(errMsg.str()));
	}

	std::string configFileName = argv[1];
	if (configFileName.size() < 5 || configFileName.substr(configFileName.size() - 5) != ".conf")
	{
		errMsg.str("");
		errMsg << "Configuration file '" << configFileName << "' must have '.conf' extension";
		throw std::runtime_error(Logger::error(errMsg.str()));
	}

	file_.open(argv[1]);
	if (!file_.is_open())
	{
		errMsg.str("");
		errMsg << "open() \"" << configFileName << "\" " << strerror(errno);
		throw std::runtime_error(Logger::error(errMsg.str()));
	}
}

std::ifstream& ServerConfigParser::getConfigFileStream() {
	return file_;
}

ServerConfigParser::ServerConfigParser() 
    : file_(),            
      lineCounter_(0),      
      currentLine_(""),       
      openingBraceCount_(0),      
      closingBraceCount_(0),
		insideServerBlock_(false),
		insideRouteBlock_(false)
{
}

ServerConfigParser::~ServerConfigParser() {
	if (file_.is_open()) {
		file_.close();
   }
}

void ServerConfigParser::parseServer(std::vector<std::string> &lineTokens, std::size_t &idx)
{
	insideServerBlock_ = true;
	if (lineTokens.size() > 1 && lineTokens[idx + 1] != "{")
		throw std::runtime_error(Logger::error("[Syntax] : 'server' block must start with '{' at line " + std::to_string(lineCounter_) + "."));
	servers_.push_back(ServerConfig());
}

void ServerConfigParser::parseListen(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : 'listen' directive requires value and semicolon at line " + std::to_string(lineCounter_) + "."));

	std::string portStr = lineTokens[idx + 1];
	if (portStr.empty() || portStr[0] == '0')
		throw std::runtime_error(Logger::error("[Syntax] : Invalid port: 'listen' Cannot be empty or start with 0 at line " + std::to_string(lineCounter_) + "."));
	for (std::size_t i = 0; i < portStr.length(); ++i)
	{
		if (portStr[i]< '0' || portStr[i] > '9')
			throw std::runtime_error(Logger::error("[Syntax] : Invalid port: 'listen' Must contain only digits at line " + std::to_string(lineCounter_) + "."));
	}
	std::size_t port = std::atoll(portStr.c_str());
	if (portStr.size() >= 6 || port < 1 || port > 65535)
		throw std::runtime_error(Logger::error("[Syntax] : Invalid port: 'listen' Must be in range 1-65535 at line " + std::to_string(lineCounter_) + "."));
	if (lineTokens[idx + 2] != ";")
		throw std::runtime_error(Logger::error("[Syntax] : 'listen' Missing ';' after port number at line " + std::to_string(lineCounter_) + "."));
	ServerConfig& current_server = servers_.back();
	if (current_server.Tracker.has_port == true)
		throw std::runtime_error(Logger::error("[Syntax] : Multiple 'listen' directives detected in configuration at line " + std::to_string(lineCounter_) + "."));
	current_server.port = port;
	current_server.Tracker.has_port = true;
	idx += 2;
}

void  ServerConfigParser::parseHost(std::vector<std::string> &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : 'host' directive requires value and semicolon at line " + std::to_string(lineCounter_) + "."));
	if (lineTokens[idx + 2] != ";")
		throw std::runtime_error(Logger::error("[Syntax] : 'host' Missing ';' after Host type at line ") + std::to_string(lineCounter_) + ".");	

	ServerConfig& current_server = servers_.back();
	if (current_server.Tracker.has_host == true)
		throw std::runtime_error(Logger::error("[Syntax] : Multiple 'host' directives detected in configuration at line " + std::to_string(lineCounter_) + "."));
	current_server.host = lineTokens[idx + 1];
	current_server.Tracker.has_host = true;
	idx += 2;
}

void ServerConfigParser::parseServerName(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	std::vector<std::string> server_names;

	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : 'server_name' needs value and semicolon at line " + std::to_string(lineCounter_) + "."));

	idx++;
	while (idx < lineTokens.size() && lineTokens[idx] != ";")
	{
		const std::string &host = lineTokens[idx];

		if (host == "{" || host == "}")
			throw std::runtime_error(Logger::error("[Syntax] : 'server_name' Unexpected '{' or '}' in server_name at line " + std::to_string(lineCounter_) + "."));
		bool hasColon = false;
		bool numberAllowed = false;
		bool hasNumberAfterColon = false;
		for (std::size_t i = 0; i < host.length(); ++i)
		{
			char c = host[i];
			if (c == ':')
			{
				if (i == 0 || i == host.length() - 1)
					throw std::runtime_error(Logger::error("[Syntax] : ':' cannot be at start or end of server_name at line " + std::to_string(lineCounter_) + "."));
				if (hasColon)
					throw std::runtime_error(Logger::error("[Syntax] : Multiple ':' in server_name at line " + std::to_string(lineCounter_) + "."));
				hasColon = true;
				numberAllowed = true;
				continue;
			}
			if (c >= '0' && c <= '9')
			{
				if (!numberAllowed)
					throw std::runtime_error(Logger::error("[Syntax] : Numbers only allowed after ':' in server_name at line " + std::to_string(lineCounter_) + "."));
				hasNumberAfterColon = true;
				continue;
			}
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '.' || c == '-')
				continue;

			throw std::runtime_error(Logger::error("[Syntax] : Invalid character in server_name at line " + std::to_string(lineCounter_) + "."));
		}

		if (hasColon && !hasNumberAfterColon)
			throw std::runtime_error(Logger::error("[Syntax] : Number required after ':' in server_name at line " + std::to_string(lineCounter_) + "."));

		server_names.push_back(host);
		idx++;
	}
	if (idx >= lineTokens.size() || lineTokens[idx] != ";")
		throw std::runtime_error(Logger::error("[Syntax] : Missing ';' in server_name at line " + std::to_string(lineCounter_) + "."));
	
	ServerConfig& current_server = servers_.back();
	if (!current_server.server_names.empty())
		throw std::runtime_error(Logger::error("[Syntax] : Multiple 'server_name' directives detected in configuration at line " + std::to_string(lineCounter_) + "."));
	for (int i = 0; i < server_names.size(); i++)
		current_server.server_names.push_back(server_names[i]);
}

void ServerConfigParser::parseMaxBodySize(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	std::size_t result ;
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : 'max_body_size' directive requires a value and a semicolon at line " + std::to_string(lineCounter_) + "."));
	idx++;
	std::string numStr = lineTokens[idx];
	if (numStr.length() > 20)
		throw std::runtime_error(Logger::error("[Syntax] : 'max_body_size' value is too large at line " + std::to_string(lineCounter_) + "."));
	unsigned long long multiplier = 1;
	char lastChar = numStr[numStr.length() - 1];

	if (lastChar == 'm' || lastChar == 'M')
	{
		multiplier = 1048576ULL;
		numStr = numStr.substr(0, numStr.length() - 1);
	}
	else if (lastChar == 'k' || lastChar == 'K')
	{
		multiplier = 1024ULL;
		numStr = numStr.substr(0, numStr.length() - 1);
	}
	else if (lastChar == 'g' || lastChar == 'G')
	{
		multiplier = 1073741824ULL;
		numStr = numStr.substr(0, numStr.length() - 1);
	}
	if (numStr.empty())
		throw std::runtime_error(Logger::error("[Syntax] : 'max_body_size' value cannot be empty at line " + std::to_string(lineCounter_) + "."));
	for (std::size_t i = 0; i < numStr.length(); ++i)
	{
		if (numStr[i]<'0' || numStr[i] > '9')
			throw std::runtime_error(Logger::error("[Syntax] : 'max_body_size' must be a valid number with an optional 'm', 'k', or 'g' suffix at line " + std::to_string(lineCounter_) + "."));
	}
	unsigned long long value = std::atol(numStr.c_str());
	if (value > std::numeric_limits<std::size_t>::max() / multiplier)
		result = std::numeric_limits<std::size_t>::max();
	else
		result = static_cast<std::size_t> (value * multiplier);
	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error(Logger::error("[Syntax] : Missing ';' in 'max_body_size' directive at line " + std::to_string(lineCounter_) + "."));
	ServerConfig &current_server = servers_.back();
	if (current_server.Tracker.has_max_body_size)
		throw std::runtime_error(Logger::error("[Syntax] : Multiple 'max_body_size' directives detected in configuration at line " + std::to_string(lineCounter_) + "."));
	current_server.max_body_size = result;
	current_server.Tracker.has_max_body_size = true;
}


void ServerConfigParser::parseErrorPage(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	if (idx + 3 >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : 'error_page' directive requires a status code, a file path, and a semicolon at line " + std::to_string(lineCounter_) + "."));
	idx++;
	std::string statusCodeStr = lineTokens[idx];
	if (statusCodeStr.empty() || statusCodeStr[0] == '0')
		throw std::runtime_error(Logger::error("[Syntax] : Invalid status code in 'error_page': Cannot be empty or start with 0 at line " + std::to_string(lineCounter_) + "."));

	for (std::size_t i = 0; i < statusCodeStr.length(); ++i)
	{
		if (statusCodeStr[i]<'0' || statusCodeStr[i] > '9')
			throw std::runtime_error(Logger::error("[Syntax] : Invalid status code in 'error_page': Must contain only digits at line " + std::to_string(lineCounter_) + "."));
	}

	std::size_t statusCode = std::atoll(statusCodeStr.c_str());
	if ((statusCodeStr.size() > 3) || statusCode < 100 || statusCode > 599)
		throw std::runtime_error(Logger::error("[Syntax] : Invalid status code in 'error_page': Must be in range 100-599 at line " + std::to_string(lineCounter_) + "."));

	idx++;

	std::string filePath = lineTokens[idx];
	if (filePath.empty())
		throw std::runtime_error(Logger::error("[Syntax] : Invalid file path in 'error_page': Cannot be empty at line " + std::to_string(lineCounter_) + "."));
	if (filePath[0] != '/')
		throw std::runtime_error(Logger::error("[Syntax] : Invalid file path in 'error_page': Must start with '/' at line " + std::to_string(lineCounter_) + "."));

	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error(Logger::error("[Syntax] : 'error_page' directive must end with a semicolon at line " + std::to_string(lineCounter_) + "."));
	ServerConfig& current_server = servers_.back();
	current_server.error_page[std::to_string(statusCode)] = 	filePath;
}

void ServerConfigParser::parseRoute(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	insideRouteBlock_ = true;
	idx++;

	if (idx >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : No route specified after 'route' keyword at line " + std::to_string(lineCounter_) + "."));

	if (((idx + 1) < lineTokens.size() && lineTokens[idx + 1] != "{"))
		throw std::runtime_error(Logger::error("[Syntax] : Multiple routes defined at line " + std::to_string(lineCounter_) + "."));
	std::string route = lineTokens[idx];
	if (route.empty() || route[0] != '/')
		throw std::runtime_error(Logger::error("[Syntax] : Route must be start with '/' at line " + std::to_string(lineCounter_) + "."));

	if (route != "default" && route != "/")
	{
		for (std::size_t i = 1; i < route.length(); i++)
		{
			if (route[i] != '/' && !std::isalpha(static_cast <unsigned char> (route[i])))
				throw std::runtime_error(Logger::error("[Syntax] : 'route' Only alphabetic characters allowed between slashes at line " + std::to_string(lineCounter_) + "."));
		}
	}
	servers_.back().routes.push_back(Route());
	servers_.back().routes.back().path = route;
}

void ServerConfigParser::parseMethods(std::vector<std::string> &lineTokens, std::size_t &idx)
{
	std::vector<std::string> Methods;

	bool isGet = false;
	bool isPost = false;
	bool isDelete = false;
	int i = 0;

	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : 'methods' directive requires at least one value at line " + std::to_string(lineCounter_) + "."));
	idx++;
	while (idx < lineTokens.size() && lineTokens[idx] != ";" && lineTokens[idx] != "{")
	{
		if (lineTokens[idx] == "GET")
		{
			if (isGet)
				throw std::runtime_error(Logger::error("[Syntax] : Duplicate method 'GET' found at line " + std::to_string(lineCounter_) + "."));
			isGet = true;
			Methods.push_back("GET");

		}
		else if (lineTokens[idx] == "POST")
		{
			if (isPost)
				throw std::runtime_error(Logger::error("[Syntax] : Duplicate method 'POST' found at line " + std::to_string(lineCounter_) + "."));
			isPost = true;
			Methods.push_back("POST");


		}
		else if (lineTokens[idx] == "DELETE")
		{
			if (isDelete)
				throw std::runtime_error(Logger::error("[Syntax] : Duplicate method 'DELETE' found at line " + std::to_string(lineCounter_) + "."));
			isDelete = true;
			Methods.push_back("DELETE");
		}
		else
			throw std::runtime_error(Logger::error("[Syntax] : Unsupported method '" + lineTokens[idx] + "' at line " + std::to_string(lineCounter_) + "."));
		idx++;
	}
	if (lineTokens[idx] != ";")
		throw std::runtime_error(Logger::error("[Syntax] : 'methods' directive must end with a semicolon at line " + std::to_string(lineCounter_) + "."));

	
	
	Route &current_route = servers_.back().routes.back();
	if (!current_route.accepted_methods.empty())
		throw std::runtime_error(Logger::error("[Syntax] : Multiple 'methods' directives detected in configuration at line " + std::to_string(lineCounter_) + "."));
	for (int i = 0; i < Methods.size(); i++)
		current_route.accepted_methods.push_back(Methods[i]);
}

void ServerConfigParser::parseRoot(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : 'root' requires a path and semicolon at line " + std::to_string(lineCounter_) + "."));
	idx++;
	std::string path = lineTokens[idx];

	for (std::string::size_type i = 0; i < path.size(); ++i)
	{
		if (	!std::isalpha(static_cast<unsigned char> (path[i])) &&
				!std::isdigit(static_cast< unsigned char > (path[i])) &&
				path[i] != '/')
			throw std::runtime_error(Logger::error("[Syntax] : 'root' path contains invalid character at line " + std::to_string(lineCounter_) + "."));
	}

	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error(Logger::error("[Syntax] : 'root' directive must end with a semicolon at line " + std::to_string(lineCounter_) + "."));
	Route &current_route = servers_.back().routes.back();
	if (current_route.Tracker.has_root_dir == true)
		throw std::runtime_error(Logger::error("[Syntax] : Multiple 'root' directives detected in configuration at line " + std::to_string(lineCounter_) + "."));
	current_route.root_dir = path;
	current_route.Tracker.has_root_dir = true;
}

void ServerConfigParser::parseDirectoryListing(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : 'DirectoryListing' needs value and semicolon at line " + std::to_string(lineCounter_) + "."));
	idx++;
	if (lineTokens[idx] != "on" && lineTokens[idx] != "off")
		throw std::runtime_error(Logger::error("[Syntax] : 'DirectoryListing' must be on|off at line " + std::to_string(lineCounter_) + "."));
	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error(Logger::error("[Syntax] : 'DirectoryListing' directive must end with a semicolon at line " + std::to_string(lineCounter_) + "."));
	Route &current_route = servers_.back().routes.back();
	if (current_route.Tracker.has_directory_listing == true)
		throw std::runtime_error(Logger::error("[Syntax] : Multiple 'has_directory_listing' directives detected in configuration at line " + std::to_string(lineCounter_) + "."));
	if (lineTokens[idx - 1] == "on")
		current_route.directory_listing = true;
	else
		current_route.directory_listing = false;
	current_route.Tracker.has_root_dir = true;
}

void ServerConfigParser::parseDefaultFile(std::vector<std::string> &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : 'default_file' needs value and semicolon at line " + std::to_string(lineCounter_) + "."));
	idx++;
	std::string file = lineTokens[idx];
	for (std::string::size_type i = 0; i < file.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(file[i]);
      if (!std::isalnum(c) && c != '/' && c != '.' && c != '-' && c != '_')
			throw std::runtime_error(Logger::error("[Syntax] : 'default_file' path contains invalid character at line " + std::to_string(lineCounter_) + "."));
	}
	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error(Logger::error("[Syntax] : 'default_file' directive must end with a semicolon at line " + std::to_string(lineCounter_) + "."));
	Route &current_route = servers_.back().routes.back();
	if (current_route.Tracker.has_default_file == true)
		throw std::runtime_error(Logger::error("[Syntax] : Multiple 'default_file' directives detected in configuration at line " + std::to_string(lineCounter_) + "."));
	current_route.default_file = lineTokens[idx - 1];
	current_route.Tracker.has_default_file = true;
}

bool isValidChar(unsigned char c)
{
	return std::isalnum(c) || c == '/' || c == '.' || c == '-' || c == '_';
}
void ServerConfigParser::parseCgi(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	if (idx + 3 >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : 'cgi' needs extension, path and semicolon at line " + std::to_string(lineCounter_) + "."));
	idx++;
	const std::string &extension = lineTokens[idx];
	for (std::string::size_type i = 0; i < extension.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char> (extension[i]);
		if (!isValidChar(c))
			throw std::runtime_error(Logger::error("[Syntax] : 'cgi' extension contains invalid character at line " + std::to_string(lineCounter_) + "."));
	}
	idx++;
	const std::string &path = lineTokens[idx];
	for (std::string::size_type i = 0; i < path.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char> (path[i]);
		if (!isValidChar(c))
			throw std::runtime_error(Logger::error("[Syntax] : 'cgi' path contains invalid character at line " + std::to_string(lineCounter_) + "."));
	}
	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error(Logger::error("[Syntax] : 'cgi' directive must end with a semicolon at line " + std::to_string(lineCounter_) + "."));

	if (servers_.empty() || servers_.back().routes.empty())
		throw std::runtime_error("Internal Error: No current route available");

	Route &currentRoute = servers_.back().routes.back();
	currentRoute.cgi_extension[extension] = path;
}

void ServerConfigParser::parseUploadDir(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : 'upload_directory' needs value and semicolon at line " + std::to_string(lineCounter_) + "."));
	idx++;
	std::string path = lineTokens[idx];
	for (std::string::size_type i = 0; i < path.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char> (path[i]);
		if (!std::isalnum(c) && c != '/' && c != '.' && c != '-' && c != '_')
			throw std::runtime_error(Logger::error("[Syntax] : 'upload_directory' path contains invalid character at line " + std::to_string(lineCounter_) + "."));
	}
	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error(Logger::error("[Syntax] : 'upload_directory' directive must end with a semicolon at line " + std::to_string(lineCounter_) + "."));
	Route &current_route = servers_.back().routes.back();
	if (current_route.Tracker.has_upload_dir == true)
		throw std::runtime_error(Logger::error("[Syntax] : Multiple 'upload_directory' directives detected in configuration at line " + std::to_string(lineCounter_) + "."));
	current_route.upload_dir = path;
	current_route.Tracker.has_upload_dir = true;
}

void ServerConfigParser::parseRedirect(std::vector<std::string> &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error(Logger::error("[Syntax] : 'redirect' needs value and semicolon at line " + std::to_string(lineCounter_) + "."));
	idx += 2;
	if (lineTokens[idx] != ";")
		throw std::runtime_error(Logger::error("[Syntax] : 'redirect' directive must end with a semicolon at line " + std::to_string(lineCounter_) + "."));
	Route &current_route = servers_.back().routes.back();
	if (current_route.Tracker.has_redirect == true)
		throw std::runtime_error(Logger::error("[Syntax] : Multiple 'redirect' directives detected in configuration at line " + std::to_string(lineCounter_) + "."));
	current_route.redirect = lineTokens[idx - 1];
	current_route.Tracker.has_redirect = true;
}

void ServerConfigParser::parseDirective(std::vector<std::string > &lineTokens)
{
	for (std::size_t idx = 0; idx < lineTokens.size(); ++idx)
	{
		if (insideServerBlock_)
		{
			if (lineTokens[idx] == "{")
				openingBraceCount_++;
			else if (lineTokens[idx] == "}")
			{
				closingBraceCount_++;
				if (openingBraceCount_ == closingBraceCount_)
					insideServerBlock_ = false;
				else if (insideRouteBlock_ && openingBraceCount_ - closingBraceCount_ == 1)
					insideRouteBlock_ = false;
			}
			if (lineTokens[idx] == "{" || lineTokens[idx] == "}")
				continue;
		}
		
		// Server block directives
		if (!insideRouteBlock_ && insideServerBlock_)
		{
			if (lineTokens[idx] == "listen")
				parseListen(lineTokens, idx);
			else if (lineTokens[idx] == "host")
				parseHost(lineTokens, idx);
			else if (lineTokens[idx] == "server_name")
				parseServerName(lineTokens, idx);
			else if (lineTokens[idx] == "max_body_size")
				parseMaxBodySize(lineTokens, idx);
			else if (lineTokens[idx] == "error_page")
				parseErrorPage(lineTokens, idx);
			else if (lineTokens[idx] == "methods")
				parseMethods(lineTokens, idx);
			else if (lineTokens[idx] == "route")
				parseRoute(lineTokens, idx);
			else
				throw std::runtime_error(Logger::error("[Syntax] : Invalid Directive in server block at line " + std::to_string(lineCounter_) + "."));
		}

		// Route block directives
		else if (insideRouteBlock_ && insideServerBlock_)
		{
			
			if (lineTokens[idx] == "methods")
				parseMethods(lineTokens, idx);
			else if (lineTokens[idx] == "root")
				parseRoot(lineTokens, idx);
			else if (lineTokens[idx] == "directory_listing")
				parseDirectoryListing(lineTokens, idx);
			else if (lineTokens[idx] == "default_file")
				parseDefaultFile(lineTokens, idx);
			else if (lineTokens[idx] == "cgi")
				parseCgi(lineTokens, idx);
			else if (lineTokens[idx] == "redirect")
				parseRedirect(lineTokens, idx);
			else if (lineTokens[idx] == "upload_directory")
				parseUploadDir(lineTokens, idx);
			else
				throw std::runtime_error(Logger::error("[Syntax] : Invalid Directive in route block at line " + std::to_string(lineCounter_) + "."));
		}
		// Handle server directive outside blocks
		else if (lineTokens[idx] == "server" && !insideServerBlock_)
			parseServer(lineTokens, idx);
		// Error for invalid context
		else if (!insideServerBlock_)
			throw std::runtime_error(Logger::error("[Syntax] : Directive outside server block at line " + std::to_string(lineCounter_) + "."));
	}
}

std::vector<std::string > ServerConfigParser::tokenizeConfigLine()
{
	std::vector<std::string> tokens;
	std::string currentToken;
	std::size_t index = 0;

	// remove comment
	std::string::size_type commentPos = currentLine_.find('#');
	if (commentPos != std::string::npos)
		currentLine_.erase(commentPos);
	while (index < currentLine_.size())
	{
		char character = currentLine_[index];
		if (character == ' ' || character == '\t')
		{
			if (!currentToken.empty())
			{
				tokens.push_back(currentToken);
				currentToken.clear();
			}
			while (index + 1 < currentLine_.size() && (currentLine_[index + 1] == ' ' || currentLine_[index + 1] == '\t'))
				++index;
		}
		else if (character == ';' || character == '{' || character == '}')
		{
			if (!currentToken.empty())
			{
				tokens.push_back(currentToken);
				currentToken.clear();
			}
			tokens.push_back(std::string(1, character));
		}
		else
			currentToken += character;
		++index;
	}
	if (!currentToken.empty())
		tokens.push_back(currentToken);
	return tokens;
}

void ServerConfigParser::parseConfigLine()
{
	std::vector<std::string> lineTokens;

	while (std::getline(file_, currentLine_))
	{
		lineCounter_++;
		lineTokens = tokenizeConfigLine();
		if (lineTokens.size())
			parseDirective(lineTokens);
	}
}

void ServerConfigParser::parseConfigFile(int argc, char* argv[])
{

	loadConfigFile(argc, argv);
	parseConfigLine();
	validate_config();
}
// ----------------------------------- TRACK DIRECTIVE ------------------------ //

RouteFoundDirective::RouteFoundDirective() : has_path(false),
has_redirect(false),
has_root_dir(false),
has_directory_listing(false),
has_default_file(false),
has_cgi_extension(false),
has_upload_dir(false) 
{}

ServerFoundDirective::ServerFoundDirective() : has_port(false),
has_host(false),
has_is_default(false),
has_max_body_size(false) 
{}


// Validate config
void ServerConfigParser::validate_config()
{
	std::map<int, std::vector<std::string>> server_name_tracker;
	std::ostringstream oss_error;

	for (int i = 0; i<static_cast<int> (servers_.size()); ++i)
	{
		// Check if port exists
		if (!servers_[i].Tracker.has_port)
			throw std::runtime_error(Logger::error("missing port for server number: " + std::to_string(i)));
		int port = servers_[i].port;
		// Loop through server names for each server
		for (std::vector<std::string>::iterator it =
			servers_[i].server_names.begin(); it != servers_[i].server_names.end(); ++it)
		{
			std::string &server_name = *it;
			if (server_name_tracker.find(port) != server_name_tracker.end())
			{
				std::vector<std::string>::iterator server_it =
					std::find(server_name_tracker[port].begin(),
						server_name_tracker[port].end(), server_name);
				if (server_it != server_name_tracker[port].end())
				{
					oss_error << "Duplicate server name '" << server_name <<
						"' found for port " << port << " in server " << i;
					throw std::runtime_error(Logger::error(oss_error.str()));
				}
				else
				{
					server_name_tracker[port].push_back(server_name);
				}
			}
			else
			{
				server_name_tracker[port].push_back(server_name);
			}
		}
		// Check for conflicting route configurations
		for (size_t j = 0; j < servers_[i].routes.size(); ++j)
		{
			const Route &route = servers_[i].routes[j];
			if (!route.redirect.empty() && route.Tracker.has_root_dir)
			{
				oss_error << "Server " << i << " route " << j <<
					" has both redirect and root_dir specified" << std::endl;
				throw std::runtime_error(Logger::error(oss_error.str()));
			}
		}
	}
}

// ----------------------------------- SET DEFAULT  PARAMETER  --------------------------------- //

Route::Route()
{
	path = "";
	redirect = "";
	root_dir = "";
	directory_listing = false;
	default_file = "";
	upload_dir = "";

}

ServerConfig::ServerConfig()
{
	port = -1;
	host = "";
	is_default = false;
	max_body_size = 1000000;
}