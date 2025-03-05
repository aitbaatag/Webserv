CXX = c++

CXXFLAGS = -I.

TARGET = Webserv

SRC = 	main.cpp \
		Response.cpp \
		CGI.cpp \
		../http_client/http_client.cpp \
		../Request/server_socket.cpp \
		../Request/Request.cpp \
		../Request/RequestUtils.cpp \
		../Request/StateMachine.cpp \
		../Request/ParseHeader.cpp

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
