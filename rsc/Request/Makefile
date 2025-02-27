CC = c++ -g
SRC = main.cpp ../../rsc/http_client/http_client.cpp server_socket.cpp Request.cpp RequestUtils.cpp StateMachine.cpp ParseHeader.cpp 
OBJ = $(SRC:.cpp=.o)
TARGET = webserver
TARGET2 = test2
CFLAGS = 

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)
fclean: clean
	rm -f $(TARGET)

run: all
	./$(TARGET)

.PHONY: all clean fclean run
