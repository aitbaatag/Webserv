#include "../../Includes/Http_Req_Res/Response.hpp"

void Response::handleCGIRequest(const Request& request, const Route& route) {
    // Extract file extension
    size_t dotPos = _filePath.find_last_of(".");
    if (dotPos == std::string::npos) {
        setStatus(400);
        _body = "<html><body><h1>400 Bad Request</h1><p>Invalid CGI request.</p></body></html>";
        _bytesToSend = _body.size();
        return;
    }
    
    std::string extension = _filePath.substr(dotPos);
    
    // Check if extension is in route's cgi_extension map
    if (route.cgi_extension.find(extension) == route.cgi_extension.end()) {
        setStatus(400);
        _body = "<html><body><h1>400 Bad Request</h1><p>Unsupported CGI extension.</p></body></html>";
        _bytesToSend = _body.size();
        return;
    }
    
    // Get CGI interpreter path
    std::string cgiInterpreter = route.cgi_extension.at(extension);
    
    // Create pipes for communication with CGI
    int input_pipe[2];  // Server to CGI
    int output_pipe[2]; // CGI to Server
    
    if (pipe(input_pipe) < 0 || pipe(output_pipe) < 0) {
        setStatus(500);
        _body = "<html><body><h1>500 Internal Server Error</h1><p>Failed to create pipes for CGI.</p></body></html>";
        _bytesToSend = _body.size();
        return;
    }
    
    // Fork process
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork failed
        close(input_pipe[0]);
        close(input_pipe[1]);
        close(output_pipe[0]);
        close(output_pipe[1]);
        
        setStatus(500);
        _body = "<html><body><h1>500 Internal Server Error</h1><p>Failed to fork process for CGI.</p></body></html>";
        _bytesToSend = _body.size();
        return;
    } 
    else if (pid == 0) {
        // Child process (CGI)
        // Close unused pipe ends
        close(input_pipe[1]);  // Close write end of input pipe
        close(output_pipe[0]); // Close read end of output pipe
        
        // Redirect stdin to input pipe
        dup2(input_pipe[0], STDIN_FILENO);
        close(input_pipe[0]);
        
        // Redirect stdout to output pipe
        dup2(output_pipe[1], STDOUT_FILENO);
        close(output_pipe[1]);
        
        // Set up CGI environment variables
        setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
        setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
        setenv("REQUEST_METHOD", request.method.c_str(), 1);
        setenv("SCRIPT_FILENAME", _filePath.c_str(), 1);
        setenv("QUERY_STRING", request.query.c_str(), 1);
        setenv("CONTENT_LENGTH", std::to_string(request.body_length).c_str(), 1);
        
        if (request.headers.find("Content-Type") != request.headers.end()) {
            setenv("CONTENT_TYPE", request.headers.at("Content-Type").c_str(), 1);
        }
        
        // Execute CGI interpreter with script as argument
        char* const args[] = {
            const_cast<char*>(cgiInterpreter.c_str()),
            const_cast<char*>(_filePath.c_str()),
            NULL
        };
        
        execve(cgiInterpreter.c_str(), args, environ);
        
        // If execve fails
        exit(1);
    } 
    else {
        // Parent process (Server)
        // Close unused pipe ends
        close(input_pipe[0]);  // Close read end of input pipe
        close(output_pipe[1]); // Close write end of output pipe
        
        // Send request body to CGI if it's a POST request
        if (request.method == "POST" && request.body_length > 0) {
            // Here you would send the request body to the CGI script
            // For simplicity, we'll assume you have the body data elsewhere
            // write(input_pipe[1], requestBody.c_str(), requestBody.size());
        }
        
        // Close write end of input pipe
        close(input_pipe[1]);
        
        // Read CGI output
        std::ostringstream cgiOutput;
        char buffer[4096];
        ssize_t bytesRead;
        
        while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            cgiOutput << buffer;
        }
        
        // Close read end of output pipe
        close(output_pipe[0]);
        
        // Wait for CGI process to finish
        int status;
        waitpid(pid, &status, 0);
        
        // Check if CGI process exited normally
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            // Process CGI output
            std::string output = cgiOutput.str();
            
            // Split CGI output into headers and body
            size_t headerEnd = output.find("\r\n\r\n");
            if (headerEnd == std::string::npos) {
                headerEnd = output.find("\n\n");
            }
            
            if (headerEnd != std::string::npos) {
                // Extract headers and body
                std::string cgiHeaders = output.substr(0, headerEnd);
                _body = output.substr(headerEnd + (output[headerEnd] == '\r' ? 4 : 2));
                
                // Parse CGI headers
                size_t contentLengthPos = cgiHeaders.find("Content-Length:");
                if (contentLengthPos != std::string::npos) {
                    size_t valueStart = cgiHeaders.find_first_not_of(" \t", contentLengthPos + 15);
                    size_t valueEnd = cgiHeaders.find_first_of("\r\n", valueStart);
                    std::string contentLengthValue = cgiHeaders.substr(valueStart, valueEnd - valueStart);
                    _bytesToSend = std::stoul(contentLengthValue);
                } else {
                    _bytesToSend = _body.size();
                }
                
                // Check for status header
                size_t statusPos = cgiHeaders.find("Status:");
                if (statusPos != std::string::npos) {
                    size_t valueStart = cgiHeaders.find_first_not_of(" \t", statusPos + 7);
                    size_t valueEnd = cgiHeaders.find_first_of("\r\n", valueStart);
                    std::string statusValue = cgiHeaders.substr(valueStart, valueEnd - valueStart);
                    
                    // Extract status code
                    size_t spacePos = statusValue.find_first_of(" \t");
                    if (spacePos != std::string::npos) {
                        std::string statusCode = statusValue.substr(0, spacePos);
                        setStatus(std::stoi(statusCode));
                    } else {
                        setStatus(std::stoi(statusValue));
                    }
                } else {
                    setStatus(200);
                }
            } else {
                // No headers found, assume all is body
                _body = output;
                _bytesToSend = _body.size();
                setStatus(200);
            }
        } else {
            // CGI execution failed
            setStatus(500);
            _body = "<html><body><h1>500 Internal Server Error</h1><p>CGI execution failed.</p></body></html>";
            _bytesToSend = _body.size();
        }
    }
}