#include "../../Includes/Http_Req_Res/Response.hpp"

void Response::handleCGIRequest(const Request& request, const Route& route) {
    std::map<std::string, std::string> env;

    env["REQUEST_METHOD"] = request.method;
    env["PATH_INFO"] = _filePath;
    env["QUERY_STRING"] = request.query;
    env["CONTENT_LENGTH"] = std::to_string(request.body.size()); // Ensure correct length
    env["CONTENT_TYPE"] = request.headers.count("Content-Type") ? request.headers.at("Content-Type") : "text/plain";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["SCRIPT_FILENAME"] = _filePath;
    env["REDIRECT_STATUS"] = "200";
    env["GATEWAY_INTERFACE"] = "CGI/1.1";

    // std::cout << "content: " << request.body << std::endl;

    std::vector<char*> envp;
    for (const auto& pair : env) {
        std::string envVar = pair.first + "=" + pair.second;
        envp.push_back(strdup(envVar.c_str()));
    }
    envp.push_back(nullptr);

    int pipeIn[2], pipeOut[2];
    if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1) {
        setStatus(500);
        _body = "<html><body><h1>500 Internal Server Error</h1><p>Failed to create pipes.</p></body></html>";
        return;
    }

    pid_t pid = fork();
    if (pid == -1) {
        setStatus(500);
        _body = "<html><body><h1>500 Internal Server Error</h1><p>Failed to fork process.</p></body></html>";
        close(pipeIn[0]); close(pipeIn[1]);
        close(pipeOut[0]); close(pipeOut[1]);
        return;
    }

    if (pid == 0) { // Child process
        dup2(pipeIn[0], STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);
        close(pipeIn[1]);
        close(pipeOut[0]);

        char* args[] = {strdup(route.cgi_extension.at(_filePath.substr(_filePath.find_last_of("."))).c_str()), strdup(_filePath.c_str()), nullptr};
        execve(args[0], args, envp.data());
        perror("execve failed");
        exit(1);
    }
    else {
        close(pipeIn[0]);
        close(pipeOut[1]);

        // Write POST body to pipe and log result
        ssize_t bytesWritten = write(pipeIn[1], request.body.c_str(), request.body.size());
        if (bytesWritten == -1) {
            std::cerr << "Failed to write to CGI input: " << strerror(errno) << std::endl;
            setStatus(500);
            _body = "<html><body><h1>500 Internal Server Error</h1><p>Failed to write to CGI input.</p></body></html>";
            close(pipeIn[1]);
            close(pipeOut[0]);
            return;
        } else if (bytesWritten != static_cast<ssize_t>(request.body.size())) {
            std::cerr << "Incomplete write to CGI input: " << bytesWritten << " of " << request.body.size() << " bytes" << std::endl;
            setStatus(500);
            _body = "<html><body><h1>500 Internal Server Error</h1><p>Incomplete write to CGI input.</p></body></html>";
            close(pipeIn[1]);
            close(pipeOut[0]);
            return;
        }

        // Ensure data is flushed
        fsync(pipeIn[1]);
        close(pipeIn[1]);

        // Read CGI output
        char buffer[BUFFER_SIZE];
        ssize_t bytesRead;
        while ((bytesRead = read(pipeOut[0], buffer, BUFFER_SIZE)) > 0) {
            _body.append(buffer, bytesRead);
        }
        close(pipeOut[0]);

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            setStatus(200);
        }
        else {
            setStatus(500);
            _body = "<html><body><h1>500 Internal Server Error</h1><p>CGI execution failed.</p></body></html>";
        }
        _bytesToSend = _body.size();
    }

    for (char* var : envp) {
        free(var);
    }
}