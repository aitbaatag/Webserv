CC = c++
SRC = main.cpp config_parser.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = webserver
CFLAGS = #-std=c++98

all: $(TARGET) clean

$(TARGET): $(OBJ)
	@$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.cpp
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJ)
	clear

fclean: clean
	@rm -f $(TARGET)
	

run: all
	./$(TARGET)

.PHONY: all clean fclean run