CXX = c++

CXXFLAGS = -I./Includes

TARGET = Webserv

SRC = ./rsc/main.cpp \
		./rsc/Config/Config.cpp \
		./rsc/response_cgi/CGI.cpp \
		./rsc/response_cgi/Response.cpp \
		./rsc/server/server_socket.cpp \
		./rsc/http_client/http_client.cpp \
		./rsc/Request/StateMachine.cpp \
		./rsc/Request/Request.cpp \
		./rsc/Request/RequestUtils.cpp \
		./rsc/Request/ParseHeader.cpp \
		./rsc/Request/ParseBody.cpp

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

re: fclean all
