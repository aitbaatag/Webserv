#include "config_parser.hpp"

// ----------------------------------- PARSING CONFIG --------------------------------- //
std::string ServerConfigParser::formatErrorMessage(LogLevel level, const std::string &message)
{
	std::string errMsg;

	time_t now = time(NULL);
	char timeBuf[20];
	strftime(timeBuf, sizeof(timeBuf), "%Y/%m/%d %H:%M:%S", localtime(&now));
	errMsg += timeBuf;

	std::string levelStr;
	switch (level)
	{
		case WARNING:
			levelStr = "warning";
			break;
		case ERROR:
			levelStr = "error";
			break;
		case DEBUG:
			levelStr = "debug";
			break;
		default:
			levelStr = "unknown";
			break;
	}

	errMsg += " [";
	errMsg += levelStr;
	errMsg += "] ";
	errMsg += message;

	return errMsg;
}


void ServerConfigParser::loadConfigFile(int argc, char *argv[])
{
	std::string errMsg;

	if (argc != 2)
	{
		errMsg = "invalid usage: ";
		errMsg += argv[0];
		errMsg += " <config_file>";
		errMsg = formatErrorMessage(ERROR, errMsg);
		throw std::runtime_error(errMsg);
	}

	std::string configFileName = argv[1];
	if (configFileName.size() < 5 || configFileName.substr(configFileName.size() - 5) != ".conf") {
		errMsg = formatErrorMessage(ERROR, "Configuration file '");
		errMsg += configFileName;
		errMsg += "' must have a .conf extension";
		throw std::runtime_error(errMsg);
  }

	file_.open(argv[1]);
	if (!file_.is_open())
	{

		errMsg = formatErrorMessage(ERROR, "open() \"");
		errMsg += configFileName;
		errMsg += "\" ";
		errMsg += strerror(errno);
		throw std::runtime_error(errMsg);
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
		throw std::runtime_error("Syntax Error: 'server' block must start with '{'.");
}

void ServerConfigParser::parseListen(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error("Syntax Error: 'listen' directive requires value and semicolon");

	std::string portStr = lineTokens[idx + 1];
	if (portStr.empty() || portStr[0] == '0')
		throw std::runtime_error("Syntax Error: Invalid port: 'listen' Cannot be empty or start with 0.");
	for (std::size_t i = 0; i < portStr.length(); ++i)
	{
		if (portStr[i]< '0' || portStr[i] > '9')
			throw std::runtime_error("Syntax Error: Invalid port: 'listen' Must contain only digits.");
	}
	int port = std::atoi(portStr.c_str());
	if (portStr.size() <= 6 && (port < 1 || port > 65535))
		throw std::runtime_error("Syntax Error: Invalid port: 'listen' Must be in range 1-65535.");
	if (lineTokens[idx + 2] != ";")
		throw std::runtime_error("Syntax Error: 'listen' Missing ';' after port number.");
	idx += 2;
}

void  ServerConfigParser::parseHost(std::vector<std::string> &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error("Syntax Error: 'host' directive requires value and semicolon");
	
	if (lineTokens[idx + 1] != "localhost" && lineTokens[idx + 1] != "127.0.0.1" && lineTokens[idx + 1] != "0.0.0.0" )
		throw std::runtime_error("Syntax Error: Invalid host value: 'host' - must be 'localhost', '127.0.0.1', or '0.0.0.0'");

	if (lineTokens[idx + 2] != ";")
		throw std::runtime_error("Syntax Error: 'host' Missing ';' after Host type.");

	idx += 2;
}

void ServerConfigParser::parseServerName(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	std::vector<std::string> server_names;

	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error("Syntax Error: 'server_name' needs value and semicolon");

	idx++;
	while (idx < lineTokens.size() && lineTokens[idx] != ";")
	{
		const std::string &host = lineTokens[idx];

		if (host == "{" || host == "}")
			throw std::runtime_error("Syntax Error: 'server_name' Unexpected '{' or '}' in server_name");
		bool hasColon = false;
		bool numberAllowed = false;
		bool hasNumberAfterColon = false;
		for (std::size_t i = 0; i < host.length(); ++i)
		{
			char c = host[i];
			if (c == ':')
			{
				if (i == 0 || i == host.length() - 1)
					throw std::runtime_error("Syntax Error: ':' cannot be at start or end of server_name");
				if (hasColon)
					throw std::runtime_error("Syntax Error: Multiple ':' in server_name");
				hasColon = true;
				numberAllowed = true;
				continue;
			}
			if (c >= '0' && c <= '9')
			{
				if (!numberAllowed)
					throw std::runtime_error("Syntax Error: Numbers only allowed after ':' in server_name");
				hasNumberAfterColon = true;
				continue;
			}
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '.' || c == '-')
				continue;

			throw std::runtime_error("Syntax Error: Invalid character in server_name");
		}

		if (hasColon && !hasNumberAfterColon)
			throw std::runtime_error("Syntax Error: Number required after ':' in server_name");

		server_names.push_back(host);
		idx++;
	}
	

	if (idx >= lineTokens.size() || lineTokens[idx] != ";")
		throw std::runtime_error("Syntax Error: Missing ';' in server_name");
}


void ServerConfigParser::parseMaxBodySize(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error("Syntax Error: 'max_body_size' directive requires a value and a semicolon.");
	idx++;
	std::string numStr = lineTokens[idx];
	if (numStr.length() > 10)
		throw std::runtime_error("Syntax Error: 'max_body_size' value is too large.");
	bool hasM = false;
	if (numStr[numStr.length() - 1] == 'm')
	{
		hasM = true;
		numStr = numStr.substr(0, numStr.length() - 1);
	}
	for (std::size_t i = 0; i < numStr.length(); ++i)
	{
		if (numStr[i]<'0' || numStr[i] > '9')
			throw std::runtime_error("Syntax Error: 'max_body_size' must be a number with an optional 'm' at the end.");
	}
	unsigned long value = std::strtoul(numStr.c_str(), 0, 10);
	if (!hasM)
		value /= 1048576;
	if (value > 2147483647UL)
		throw std::runtime_error("Syntax Error: 'max_body_size' value is too large.");
	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error("Syntax Error: Missing ';' in 'max_body_size' directive.");
}


void ServerConfigParser::parseErrorPage(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	if (idx + 3 >= lineTokens.size())
		throw std::runtime_error("Syntax Error: 'error_page' directive requires a status code, a file path, and a semicolon.");
	idx++;
	std::string statusCodeStr = lineTokens[idx];
	if (statusCodeStr.empty() || statusCodeStr[0] == '0')
		throw std::runtime_error("Syntax Error: Invalid status code in 'error_page': Cannot be empty or start with 0.");

	for (std::size_t i = 0; i < statusCodeStr.length(); ++i)
	{
		if (statusCodeStr[i]<'0' || statusCodeStr[i] > '9')
			throw std::runtime_error("Syntax Error: Invalid status code in 'error_page': Must contain only digits.");
	}

	int statusCode = std::atoi(statusCodeStr.c_str());
	if (statusCode < 100 || statusCode > 599)
		throw std::runtime_error("Syntax Error: Invalid status code in 'error_page': Must be in range 100-599.");

	idx++;

	std::string filePath = lineTokens[idx];
	if (filePath.empty())
		throw std::runtime_error("Syntax Error: Invalid file path in 'error_page': Cannot be empty.");
	if (filePath[0] != '/')
		throw std::runtime_error("Syntax Error: Invalid file path in 'error_page': Must start with '/'.");

	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error("Syntax Error: 'error_page' directive must end with a semicolon.");
	// errorPages[statusCode] = filePath;
}

void ServerConfigParser::parseRoute(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	// Servers.push_back({});
	insideRouteBlock_ = true;
	idx++;

	if (idx >= lineTokens.size())
		throw std::runtime_error("Syntax Error: No route specified after 'route' keyword");

	if (((idx + 1) < lineTokens.size() && lineTokens[idx + 1] != "{"))
		throw std::runtime_error("Syntax Error: Multiple routes defined");
	std::string route = lineTokens[idx];
	if (route.empty() || (route != "default" && route[0] != '/'))
		throw std::runtime_error("Syntax Error: Route must be 'default' or start with '/'");

	if (route != "default" && route != "/")
	{
		for (std::size_t i = 1; i < route.length(); i++)
		{
			if (route[i] != '/' && !std::isalpha(static_cast <unsigned char> (route[i])))
				throw std::runtime_error("Syntax Error: 'route' Only alphabetic characters allowed between slashes");
		}
	}
}

void ServerConfigParser::parseMethods(std::vector<std::string> &lineTokens, std::size_t &idx)
{
	std::vector<std::string> Methods;

	bool isGet = false;
	bool isPost = false;
	bool isDelete = false;
	int i = 0;

	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error("Syntax Error: 'methods' directive requires at least one value");
	idx++;
	while (idx < lineTokens.size() && lineTokens[idx] != ";" && lineTokens[idx] != "{")
	{
		if (lineTokens[idx] == "GET")
		{
			if (isGet)
				throw std::runtime_error("Syntax Error: Duplicate method 'GET' found");
			isGet = true;
			Methods.push_back("GET");

		}
		else if (lineTokens[idx] == "POST")
		{
			if (isPost)
				throw std::runtime_error("Syntax Error: Duplicate method 'POST' found");
			isPost = true;
			Methods.push_back("POST");


		}
		else if (lineTokens[idx] == "DELETE")
		{
			if (isDelete)
				throw std::runtime_error("Syntax Error: Duplicate method 'DELETE' found");
			isDelete = true;
			Methods.push_back("DELETE");
		}
		else
			throw std::runtime_error("Syntax Error: Unsupported method '" + lineTokens[idx] + "'");
		idx++;
	}
	if (lineTokens[idx] != ";")
		throw std::runtime_error("Syntax Error: 'methods' directive must end with a semicolon");

}

void ServerConfigParser::parseRoot(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error("Syntax Error: 'root' requires a path and semicolon");
	idx++;
	std::string path = lineTokens[idx];

	for (std::string::size_type i = 0; i < path.size(); ++i)
	{
		if (	!std::isalpha(static_cast<unsigned char> (path[i])) &&
				!std::isdigit(static_cast< unsigned char > (path[i])) &&
				path[i] != '/')
			throw std::runtime_error("Syntax Error: 'root' path contains invalid character");
	}

	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error("Syntax Error: 'root' directive must end with a semicolon");

}

void ServerConfigParser::parseDirectoryListing(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error("Syntax Error: 'DirectoryListing' needs value and semicolon");
	idx++;
	if (lineTokens[idx] != "on" && lineTokens[idx] != "off")
		throw std::runtime_error("Syntax Error: 'DirectoryListing' must be on|off");
	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error("Syntax Error: 'DirectoryListing' directive must end with a semicolon");
}

void ServerConfigParser::parseDefaultFile(std::vector<std::string> &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error("Syntax Error: 'default_file' needs value and semicolon");
	idx++;
	std::string file = lineTokens[idx];
	for (std::string::size_type i = 0; i < file.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(file[i]);
      if (!std::isalnum(c) && c != '/' && c != '.' && c != '-' && c != '_')
			throw std::runtime_error("Syntax Error: 'default_file' path contains invalid character");
	}
	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error("Syntax Error: 'default_file' directive must end with a semicolon");
}

void ServerConfigParser::parseCgi(std::vector<std::string> &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error("Syntax Error: 'cgi' needs value and semicolon");
	idx++;
	std::string path = lineTokens[idx];
	for (std::string::size_type i = 0; i < path.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(path[i]);
		if (!std::isalnum(c) && c != '/' && c != '.' && c != '-' && c != '_')
			throw std::runtime_error("Syntax Error: 'cgi' path contains invalid character");
	}
	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error("Syntax Error: 'cgi' directive must end with a semicolon");
}

void ServerConfigParser::parseUploadDir(std::vector<std::string > &lineTokens, std::size_t &idx)
{
	if (idx + 2 >= lineTokens.size())
		throw std::runtime_error("Syntax Error: 'upload_directory' needs value and semicolon");
	idx++;
	std::string path = lineTokens[idx];
	for (std::string::size_type i = 0; i < path.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char> (path[i]);
		if (!std::isalnum(c) && c != '/' && c != '.' && c != '-' && c != '_')
			throw std::runtime_error("Syntax Error: 'upload_directory' path contains invalid character");
	}
	idx++;
	if (lineTokens[idx] != ";")
		throw std::runtime_error("Syntax Error: 'upload_directory' directive must end with a semicolon");
}

void ServerConfigParser::parseDirective(std::vector<std::string > &lineTokens)
{
	for (std::size_t idx = 0; idx < lineTokens.size(); ++idx)
	{
		// std::cout << "--------\n";
		// std::cout << lineTokens[idx] << std::endl;
		// // std::cout <<"[" << openingBraceCount_ << " : " <<  closingBraceCount_ << "]" << std::endl;
		// std::cout << "--------\n";

		// Handle brace counting within server block
		// Handle brace counting within server block
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
				throw std::runtime_error("Syntax Error at Line " + std::to_string(lineCounter_) +": Invalid Directive in server block");
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
			else if (lineTokens[idx] == "upload_directory")
				parseUploadDir(lineTokens, idx);
			else
				throw std::runtime_error("Syntax Error at Line " + std::to_string(lineCounter_) +": Invalid Directive in route block");
		}
		// Handle server directive outside blocks
		else if (lineTokens[idx] == "server" && !insideServerBlock_)
			parseServer(lineTokens, idx);
		// Error for invalid context
		else if (!insideServerBlock_)
			throw std::runtime_error("Syntax Error at Line " + std::to_string(lineCounter_) +": Directive outside server block");
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
	try
	{
		loadConfigFile(argc, argv);
		parseConfigLine();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}

// ----------------------------------- SET DEFAULT  PARAMETER  --------------------------------- //

Route::Route()
{
	path = "";
	redirect = "";
	root_dir = "";
	directory_listing = false;
	default_file = "index.html";
	cgi_extension = "";
	upload_dir = "";

}

Server::Server()
{
	port = 0;
	host = "0.0.0.0";
	// Empty server_name means accept all
	is_default = false;
	error_page["404"] = "/default/path";
	error_page["500"] = "/default/path";
	max_body_size = 10;
}