#include "../../Includes/Http_Req_Res/Response.hpp"
#include <sys/stat.h>
#include <fcntl.h>

void Response::handleCGIRequest(Request& request, const Route& route) {
    request.fileStream.close();

    // Convert headers to environment variables with "HTTP_" prefix
    std::vector<char*> envp;
    for (std::map<std::string, std::string>::const_iterator it = request.headers.begin();
         it != request.headers.end(); ++it) {
        std::string headerName = it->first;
        // Convert header name to uppercase and replace '-' with '_'
        std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::toupper);
        std::replace(headerName.begin(), headerName.end(), '-', '_');
        // Create environment variable string
        std::string envVar = "HTTP_" + headerName + "=" + it->second;
        envp.push_back(strdup(envVar.c_str()));
    }
    envp.push_back(NULL);

    int pipeOut[2];
    if (pipe(pipeOut) == -1) {
        setStatus(500);
        _body = "<html><body><h1>500 Internal Server Error</h1><p>Pipe creation failed.</p></body></html>";
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        setStatus(500);
        _body = "<html><body><h1>500 Internal Server Error</h1><p>Fork failed.</p></body></html>";
        close(pipeOut[0]);
        close(pipeOut[1]);
        return;
    }

    if (pid == 0) { // Child process
        // Open the request body file
        int file_fd = open(request.filename.c_str(), O_RDONLY);
        if (file_fd < 0) {
            perror("open request body file failed");
            exit(1);
        }

        dup2(file_fd, STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);
        close(pipeOut[0]);

        char* args[] = {
            strdup(route.cgi_extension.at(_filePath.substr(_filePath.find_last_of("."))).c_str()),
            strdup(_filePath.c_str()),
            NULL
        };

        execve(args[0], args, &envp[0]);
        perror("execve failed");
        exit(1);
    } else { // Parent process
        close(pipeOut[1]);

        char buffer[BUFFER_SIZE];
        ssize_t bytesRead;
        while ((bytesRead = read(pipeOut[0], buffer, BUFFER_SIZE)) > 0) {
            _body.append(buffer, bytesRead);
        }
        close(pipeOut[0]);

        int status;
        waitpid(pid, &status, WNOHANG);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            setStatus(200);
        } else {
            setStatus(500);
            _body = "<html><body><h1>500 Internal Server Error</h1><p>CGI script failed.</p></body></html>";
        }

        _bytesToSend = _body.size();
    }

    // Free allocated memory
    for (std::vector<char*>::iterator it = envp.begin(); it != envp.end(); ++it) {
        if (*it) free(*it);
    }
}