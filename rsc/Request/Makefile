CC = c++ -g
SRC = main.cpp ../../rsc/http_client/http_client.cpp server_socket.cpp Request.cpp RequestUtils.cpp
OBJ = $(SRC:.cpp=.o)
SRC2 = main2.cpp ../../rsc/http_client/http_client.cpp Request.cpp RequestUtils.cpp
OBJ2 = $(SRC2:.cpp=.o)
TARGET = webserver
TARGET2 = test2
CFLAGS = -std=c++98

all: $(TARGET)
test2 : $(TARGET2)

$(TARGET2): $(OBJ2)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET2)
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)
	rm -f $(OBJ2)
fclean: clean
	rm -f $(TARGET)

run: all
	./$(TARGET)

.PHONY: all clean fclean run
