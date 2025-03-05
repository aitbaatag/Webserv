#include "../../Includes/Http_Req_Res/Response.hpp"

void Response::handleCGIRequest() {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        setStatus(500);
        _body = "Failed to create pipe for CGI execution.\n";
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        setStatus(500);
        _body = "Fork failed.\n";
        return;
    }

    if (pid == 0) {  // Child process
        close(pipe_fd[0]); // Close read end

        // Redirect stdout to pipe write end
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        // Convert _filePath to a format execve can use
        char *argv[] = { (char*)_filePath.c_str(), NULL };
        char *envp[] = { NULL };

        execve(argv[0], argv, envp);

        // If execve fails
        std::cerr << "CGI execution failed\n";
        exit(1);
    } 
    else {  // Parent process
        close(pipe_fd[1]); // Close write end

        char buffer[1024];
        std::ostringstream output;
        ssize_t bytesRead;

        while ((bytesRead = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            output << buffer;
        }

        close(pipe_fd[0]);

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            setStatus(200);
            _body = output.str();
        }
        else {
            setStatus(500);
            _body = "CGI script failed to execute properly.\n";
        }
    }
}
