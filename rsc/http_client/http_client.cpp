
#include "../../Includes/http_client/http_client.hpp"
#include "../../Includes/utlis/utils.hpp"

HttpClient::~HttpClient() { close_fd(socket_fd_); }

HttpClient::HttpClient() {}

HttpClient::HttpClient(int client_socket, std::string client_ip,
                       uint16_t client_port)
    : SMrequest() {
  socket_fd_ = client_socket;
  pos_ = 0;
  request_status_ = InProgress;
  response_status_ = InProgress;
  this->client_ip = client_ip;
  this->client_port = client_port;
  time_start_ = current_time_in_ms();
  time_client_ = time(NULL);
  res._client = this;
  Srequest._client = this;
  server_config = NULL;
  route = NULL;
}

size_t HttpClient::get_client_time() { return time_client_; };

std::string HttpClient::get_client_ip() { return client_ip; };

int HttpClient::get_socket_fd() { return socket_fd_; }

Status HttpClient::get_request_status() { return request_status_; }

Status HttpClient::get_response_status() { return response_status_; }

bool HttpClient::append_to_request() {
  pos_ = 0;
  bytes_received = recv(socket_fd_, buffer, MAX_RECV - 1, 0);
  if (bytes_received <= 0) {
    request_status_ = Disc;
    return false;
  }
  time_client_ = time(NULL);
  return true;
}

void HttpClient::update_pos(int new_pos) { pos_ = new_pos; }

int HttpClient::get_pos() const { return pos_; }

char *HttpClient::get_request_buffer() { return buffer; }

void HttpClient::set_request_status(Status status) { request_status_ = status; }

void HttpClient::set_response_status(Status status) {
  response_status_ = status;
}

void HttpClient::reset() {
  // httpClient RESET

  pos_ = 0;
  request_status_ = InProgress;
  response_status_ = InProgress;
  time_start_ = current_time_in_ms();
  time_client_ = time(NULL);
  bytes_received = 0;
  response_buffer_.clear();
  readTrack.clear();
  writeTrack.clear();

  SMrequest = StateMachine();

  //// clean the Request
  Srequest.reset();
  res.reset();
  res._client = this;
  Srequest._client = this;
}

void handleConnectionHeader(HttpClient **c, int epfdMaster) {
  if ((*c)->get_request_status() == Failed) {
    handleClientDisconnection(*c, epfdMaster);
    *c = NULL;
    return;
  }
  std::map<std::string, std::string>::iterator it =
      (*c)->Srequest.headers.find("Connection");
  if (it != (*c)->Srequest.headers.end()) {
    std::string val = it->second;
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    if (val == "close") {
      handleClientDisconnection(*c, epfdMaster);
      *c = NULL;
    } else {
      (*c)->reset();
      return;
    }
  } else {
    (*c)->reset();
    return;
  }
}
void processEpollEvents(epoll_event *event, HttpClient *c, int epfdMaster)
{
	if ((event->events &EPOLLIN) && c && c->get_request_status() == InProgress)
	{
		try
		{
			if (c->append_to_request())
        HttpRequest::parseIncrementally(*c);        
		}

		catch (const std::exception &e)
		{
			std::cerr << Logger::error("Error processing request: " + std::string(e.what()));
			return;
		}
	}
	if ((event->events &EPOLLOUT) && c && (c->get_request_status() == Complete || c->get_request_status() == Failed))
	{
		try
		{
			c->res.response_handler();
			if (c->get_response_status() == Complete)
			{
				handleConnectionHeader(&c, epfdMaster);
			}
		}
		catch (const std::exception &e)
		{
			std::cerr << Logger::error("Error sending response: " + std::string(e.what()));
			return;
		}
	}
  if (c && c->get_request_status() == Disc)
    handleClientDisconnection(c, epfdMaster);
}

void handleClientDisconnection(HttpClient *c, int epfdMaster)
{
	int fd = c->socket_fd_;

	if (epoll_ctl(epfdMaster, EPOLL_CTL_DEL, fd, NULL) == -1)
	{
		Logger::error("Failed to remove client " + to_string(fd) + " from epoll: " + std::string(strerror(errno)));
	}
	std::map<int, EpollEventContext *> &FileDescriptorList = c->server->getServerConfigParser()->getFileDescriptorList();
	EpollEventContext *ctx = FileDescriptorList[fd];
	if (ctx)
	{
		if (ctx->httpClient)
		{
			delete ctx->httpClient;
			ctx->httpClient = NULL;
		}

		delete ctx;
		FileDescriptorList[fd] = NULL;
	}

	FileDescriptorList.erase(fd);
}

void cleanAllClientServer(
    std::map<int, EpollEventContext *> &FileDescriptorList) {
  for (std::map<int, EpollEventContext *>::iterator it =
           FileDescriptorList.begin();
       it != FileDescriptorList.end(); ++it) {
    if (it->second) {
      close(it->first);

      if (it->second->descriptorType == ServerSocketFd) {
        close(it->second->serverSocket->get_socket_fd());
        delete it->second->serverSocket;
        it->second->serverSocket = NULL;
      } else if (it->second->descriptorType == ClientSocketFd) {
        close(it->second->httpClient->get_socket_fd());
        delete it->second->httpClient;
        it->second->httpClient = NULL;
      }
      delete it->second;
      it->second = NULL;
    }
  }
  FileDescriptorList.clear();
}

void handleClientTimeouts(
    std::map<int, EpollEventContext *> &FileDescriptorList, int epfdMaster) {
  size_t now = time(NULL);
  for (std::map<int, EpollEventContext *>::iterator it =
           FileDescriptorList.begin();
       it != FileDescriptorList.end();) {
    EpollEventContext *ctx = it->second;
    if (ctx && ctx->descriptorType == ClientSocketFd && ctx->httpClient) {
      size_t client_time = ctx->httpClient->get_client_time();
      size_t elapsed = now - client_time;
      if (elapsed > TIMEOUT) {
        int fd = it->first;
        std::string client_ip = ctx->httpClient->client_ip;
        uint16_t client_port = ctx->httpClient->client_port;
        std::cout << Color::BOLD << "[TRACE] " << Color::RESET << Color::CYAN
                  << "[" << Logger::get_timestamp() << "] " << Color::RESET
                  << client_ip << ":" << client_port << " " << Color::YELLOW
                  << "\"TIMEOUT after " << elapsed << "s\"" << Color::RESET
                  << std::endl;
        if (ctx->httpClient->res.getpid() > 0) {
          kill(ctx->httpClient->res.getpid(), SIGKILL);
          ctx->httpClient->res.resetpid();
        }
        handleClientDisconnection(ctx->httpClient, epfdMaster);
        it = FileDescriptorList.upper_bound(fd);
        continue;
      }
    }
    ++it;
  }
}

void addConfigServer(ServerConfig &serversConfig,
                     std::vector<ServerSocket *> &servers, int epfdMaster) {
  for (size_t i = 0; i < servers.size(); i++) {
    if (servers[i]->getServerPort() == serversConfig.port) {
      servers[i]->getServerConfig().push_back(serversConfig);
      return;
    }
  }
  ServerSocket *newServer = new ServerSocket(epfdMaster);
  newServer->setServerConfigParser(serversConfig.scp);
  newServer->getServerConfig().push_back(serversConfig);
  newServer->setupServerPort();
  servers.push_back(newServer);
}

void HttpClient::deleteFileEpoll(int fd) {
  std::map<int, EpollEventContext *> &FileDescriptorList =
      server->getServerConfigParser()->getFileDescriptorList();

  if (epoll_ctl(server->getEpfdMaster(), EPOLL_CTL_DEL, fd, NULL) < 0) {
    throw std::runtime_error("Failed to remove file from epoll");
  }

  std::map<int, EpollEventContext *>::iterator it = FileDescriptorList.find(fd);
  if (it != FileDescriptorList.end()) {
    delete it->second;
    FileDescriptorList.erase(it);
  }
}

void HttpClient::registerFileEpoll(int fd) {
  std::map<int, EpollEventContext *> &FileDescriptorList =
      server->getServerConfigParser()->getFileDescriptorList();

  struct epoll_event ev;
  ev.events = EPOLLIN | EPOLLOUT;
  ev.data.fd = fd;
  ev.data.ptr = EpollEventContext::createFileData(fd, this);
  FileDescriptorList[fd] = static_cast<EpollEventContext *>(ev.data.ptr);
  if (epoll_ctl(server->getEpfdMaster(), EPOLL_CTL_ADD, fd, &ev) < 0) {
    throw std::runtime_error("Failed to add file to epoll");
  }
}
