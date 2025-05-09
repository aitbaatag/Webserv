#include "../../Includes/Http_Req_Res/Response.hpp"
#include "../../Includes/http_client/http_client.hpp"
#include "../../Includes/utlis/utils.hpp"
#include <fcntl.h>

bool Response::handleCGIRequest()
{
	switch (_cgiState)
	{
		case CGI_STATE_INIT:
			cgiInit();
			break;
		case CGI_STATE_FORK:
			cgiFork();
			break;
		case CGI_STATE_READ_OUTPUT:
			cgiReadOutput();
			break;
		case CGI_STATE_WAIT_CHILD:
			cgiWaitChild();
			break;
		case CGI_STATE_PROCESS_RESULT:
			cgiProcessResult();
			break;
		case CGI_STATE_CLEANUP:
			cgiCleanup();
			break;
		case CGI_STATE_DONE:
			{
				_file_path_fd = _cgi_tmp_fd;
				struct stat st;
				if (stat(_cgi_tmp_filename.c_str(), &st) == 0)
					_bytesToSend = st.st_size;
				lseek(_cgi_tmp_fd, 0, SEEK_SET);
				return true;
			}

		case CGI_STATE_ERROR: {
			cgiCleanup();
			return true;
		}

		default:
			break;
	}
	return false;
}

void Response::cgiInit()
{
	close_fd(_client->Srequest.fd_file);
	_cgi_envp.clear();
	std::map<std::string, std::string>::const_iterator it;
	for (it = _client->Srequest.headers.begin(); it != _client->Srequest.headers.end(); ++it)
	{
		std::string headerName = it->first;
		std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::toupper);
		std::replace(headerName.begin(), headerName.end(), '-', '_');
		std::string envVar = "HTTP_" + headerName + "=" + it->second;
		_cgi_envp.push_back(strdup(envVar.c_str()));
	}

	_cgi_envp.push_back(NULL);
	_cgi_envp_built = true;
	_cgiState = CGI_STATE_FORK;
}

void Response::cgiFork()
{
	int pipeOut[2];
	if (pipe(pipeOut) == -1)
	{
		_cgiState = CGI_STATE_ERROR;
		if (error_page("500") == 0)
			_body = "<html><body> < h1>500 Internal Server Error</h1 > < p>Pipe creation failed.</p></body></html>";
		return;
	}

	_cgi_pid = fork();
	if (_cgi_pid == -1)
	{
		close_fd(pipeOut[0]);
		close_fd(pipeOut[1]);
		_cgiState = CGI_STATE_ERROR;
		if (error_page("500") == 0)
			_body = "<html><body> < h1>500 Internal Server Error</h1 > < p>Fork failed.</p></body></html>";
		return;
	}

	if (_cgi_pid == 0)
	{
		int file_fd;
		if (_client->Srequest.filename.empty())
			file_fd = open("/dev/null", O_RDONLY);
		else
			file_fd = open(_client->Srequest.filename.c_str(), O_RDONLY);
		if (file_fd < 0)
		{
			perror("open request body file failed");
			exit(1);
		}

		dup2(file_fd, STDIN_FILENO);
		close_fd(file_fd);
		dup2(pipeOut[1], STDOUT_FILENO);
		close_fd(pipeOut[0]);
		close_fd(pipeOut[1]);
		char *args[] = { strdup(_client->route->cgi_extension.at(_filePath.substr(_filePath.find_last_of("."))).c_str()), strdup(_filePath.c_str()), NULL
		};

		execve(args[0], args, &_cgi_envp[0]);
		free(args[0]);
		free(args[1]);
		exit(1);
	}
	else
	{
		close_fd(pipeOut[1]);
		int flags = fcntl(pipeOut[0], F_GETFL, 0);
		if (flags == -1 || fcntl(pipeOut[0], F_SETFL, flags | O_NONBLOCK) == -1)
		{
			close_fd(pipeOut[0]);
			_cgiState = CGI_STATE_ERROR;
			if (error_page("500") == 0)
				_body = "<html><body> < h1>500 Internal Server Error</h1 > < p>Failed to set pipe non-blocking.</p></body></html>";
			return;
		}

		_cgi_pipe_fd = pipeOut[0];
		_cgi_tmp_filename = ".tmp_cgi_response_" + to_string(_client->get_socket_fd());
		_cgi_tmp_fd = open(_cgi_tmp_filename.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0600);
		if (_cgi_tmp_fd < 0)
		{
			close_fd(_cgi_pipe_fd);
			_cgi_pipe_fd = -1;
			_cgiState = CGI_STATE_ERROR;
			if (error_page("500") == 0)
				_body = "<html><body> < h1>500 Internal Server Error</h1 > < p>Failed to open temp file.</p></body></html>";
			return;
		}

		_cgiState = CGI_STATE_READ_OUTPUT;
	}
}
void Response::cgiReadOutput()
{
	char buffer[MAX_SEND];
	ssize_t bytesRead = read(_cgi_pipe_fd, buffer, MAX_SEND);
	if (bytesRead > 0)
	{
		write(_cgi_tmp_fd, buffer, bytesRead);
		return;
	}

	if (bytesRead == 0)
	{
		close_fd(_cgi_pipe_fd);
		_cgi_pipe_fd = -1;
		_cgiState = CGI_STATE_WAIT_CHILD;
	}
}

void Response::cgiWaitChild()
{
	pid_t result = waitpid(_cgi_pid, &_cgi_child_status, WNOHANG);
	if (result == 0) {
		return;
	}

	_cgiState = CGI_STATE_PROCESS_RESULT;
}

void Response::cgiProcessResult()
{
	if (WIFEXITED(_cgi_child_status) && WEXITSTATUS(_cgi_child_status) == 0) {
		setStatus(200);
	}
	else
	{
		if (error_page("500") == 0) {
			_body = "<html><body> < h1>500 Internal Server Error</h1 > < p>CGI script failed.</p></body></html>";
		}
	}

	_cgiState = CGI_STATE_CLEANUP;
}

void Response::cgiCleanup()
{
	if (_cgi_envp_built)
	{
		for (std::vector<char*>::iterator it = _cgi_envp.begin(); it != _cgi_envp.end(); ++it)
		{
			if (*it)
			{
				free(*it);
				*it = NULL;
			}
		}
		_cgi_envp.clear();
		_cgi_envp_built = false;
	}

	if (_cgi_pipe_fd != -1)
	{
		close_fd(_cgi_pipe_fd);
		_cgi_pipe_fd = -1;
	}

	_cgi_pid = -1;
	_cgi_child_status = 0;
	_cgiState = CGI_STATE_DONE;
}