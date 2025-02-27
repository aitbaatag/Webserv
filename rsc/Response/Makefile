CXX = c++

CXXFLAGS = -I.

TARGET = Webserv

SRC = main.cpp Response.cpp ../../rsc/http_client/http_client.cpp ../../rsc/Request/server_socket.cpp ../../rsc/Request/Request.cpp ../../rsc/Request/RequestUtils.cpp ../../rsc/Request/StateMachine.cpp ../../rsc/Request/ParseHeader.cpp

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
