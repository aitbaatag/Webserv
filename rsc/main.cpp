#include "../Includes/Config/Config.hpp"
#include "../Includes/Http_Req_Res/Request.hpp"
#include "../Includes/Http_Req_Res/Response.hpp"
#include "../Includes/Http_Req_Res/StateMachine.hpp"
#include "../Includes/http_client/http_client.hpp"
#include "../Includes/server/server_socket.hpp"

void addConfigServer(ServerConfig &serversConfig,
                     std::vector<ServerSocket *> &servers) {
  for (size_t i = 0; i < servers.size(); i++) {
    if (servers[i]->getServerPort() == serversConfig.port) {
      servers[i]->getServerConfig().push_back(serversConfig);
      return;
    }
  }
  ServerSocket *newServer = new ServerSocket();
  newServer->getServerConfig().push_back(serversConfig);
  newServer->setupServerPort();
  servers.push_back(newServer);
}

void startServers(std::vector<ServerSocket *> &servers) {
  for (size_t i = 0; i < servers.size(); i++)
    servers[i]->startServer();

  while (true) {
    for (size_t i = 0; i < servers.size(); ++i) {
      servers[i]->handleClientConnection();
    }
  }
}

int main(int argc, char **argv) {
  try {
    ServerConfigParser configParser;
    configParser.parseConfigFile(argc, argv);
    std::vector<ServerConfig> &serversConfig = configParser.getServers();
    std::vector<ServerSocket *> servers;

    for (size_t i = 0; i < serversConfig.size(); i++) {
      addConfigServer(serversConfig[i], servers);
    }
    startServers(servers);

    // Clean up allocated memory
    for (size_t i = 0; i < servers.size(); ++i) {
      delete servers[i];
    }

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
