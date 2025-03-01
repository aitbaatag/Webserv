#include "config_parser.hpp"


void printTokens(const std::vector<std::string > &tokens, std::size_t i)
{
	std::cout << i << ": ";
	for (std::size_t i = 0; i < tokens.size(); ++i)
	{
		std::cout << "[" << tokens[i] << "]";
		if (i + 1 < tokens.size())
			std::cout << " ";
	}

	std::cout << "\n";
}
int main(int argc, char* argv[])
{
	ServerConfigParser server;
	server.parseConfigFile(argc, argv);
	

   return 0;
}