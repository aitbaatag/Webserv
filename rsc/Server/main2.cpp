#include <cstring>
#include <string>
#include <vector>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>












int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << "<config_file>" << std::endl;
		return 1;
	}

	std::string config_file = argv[1];
	std::ifstream file(config_file);
	if (!file.is_open())
	{
		std::cerr << "Error: Could not open configuration file '" << config_file << "'" << std::endl;
		return 1;
	}

	std::string line;
	int line_number = 0;
	bool is_server = false;
	int count = -1;
	std::size_t cbrkt_count = 0;

	try
	{
		while (std::getline(file, line))
		{
			line_number++;

			if (!line.empty() && line[0] != '#')
			{
				line = clean_spaces(line);
				if (line == "server {")
				{
					is_server = true;
					cbrkt_count += 1;
					server.servers.push_back({});
					count++;
				}
				else if (is_server)
				{
					if (std::strncmp(line.c_str(), "listen",6))

				}
				else
				{
					throw std::runtime_error("Configuration file syntax error at line " + std::to_string(line_number) +
						": Unexpected line detected outside of a server block"
				);
				}
			}
		}

		// Optional: Check if any server blocks were found
		if (count == -1)
		{
			std::cerr << "Warning: No server blocks found in configuration file" << std::endl;
		}
	}

	catch (const std::runtime_error &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		file.close();
		return 1;
	}

	file.close();
	return 0;
}



std::vector<std::string> split(const std::string &line)
{
	std::vector<std::string> tokens;
	std::string current;
	for (std::size_t i = 0; i < line.size(); ++i)
	{
		if (line[i] == ' ' && !current.empty())
		{
			tokens.push_back(current);
			current.clear();
		}
		else if (line[i] == ';')
		{
			if (!current.empty())
				tokens.push_back(current);
			tokens.push_back(";");
			current.clear();
		}
		else if (line[i] != ' ' && line[i] != '\t')
			current += line[i];
	}

	if (!current.empty())
		tokens.push_back(current);

	return tokens;
}

int main()
{
	std::string h = "hello 10 10320 045640 23103 ;;";
	std::vector<std::string> token = split(h);
	for (int i = 0; i < token.size(); i++)
		std::cout << token[i] << std::endl;
}