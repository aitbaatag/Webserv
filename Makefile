CC = c++
SRC = main.cpp http_client.cpp server_socket.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = webserver
CFLAGS = -std=c++98

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