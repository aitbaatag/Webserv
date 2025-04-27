CXX = c++ -std=c++98

CXXFLAGS = -I./Includes

TARGET = Webserv

SRC = ./rsc/main.cpp \
		./rsc/utils/utils.cpp \
		./rsc/Config/Config.cpp \
		./rsc/response_cgi/CGI.cpp \
		./rsc/response_cgi/Response.cpp \
		./rsc/server/server_socket.cpp \
		./rsc/http_client/http_client.cpp \
		./rsc/Request/StateMachine.cpp \
		./rsc/Request/Request.cpp \
		./rsc/Request/RequestUtils.cpp \
		./rsc/Request/ParseHeader.cpp \
		./rsc/Request/ParseBody.cpp \
		./rsc/cookies/session.cpp \
		./rsc/cookies/session_manager.cpp \

OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean:
	rm -f $(OBJ) $(TARGET)


# Add these variables at the top with other variables
PROFILE_DURATION = 100
PROFILE_DATA = app_gprof_data.txt
CALL_GRAPH = app_call_graph.dot

# Add this target after other targets
profile: $(TARGET)
	rm -rf app_* gmo* gpro*
	@echo "Starting profiling session for $(PROFILE_DURATION) seconds..."
	@(sleep $(PROFILE_DURATION) && kill -INT $$(pgrep $(TARGET))) & \
	./$(TARGET)	webserv.conf; \
	source ~/.venv/bin/activate && \
	gprof $(TARGET) gmon.out >> $(PROFILE_DATA) && \
	gprof2dot $(PROFILE_DATA) > $(CALL_GRAPH) && \
	xdot $(CALL_GRAPH)


re: fclean all
