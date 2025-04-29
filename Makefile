# Compiler and flags
CXX      = c++ -std=c++98
CXXFLAGS = -I./Includes

# Colors
GREEN    = \033[0;32m
CYAN     = \033[0;36m
RED      = \033[0;31m
RESET    = \033[0m

# Project settings
TARGET   = Webserv
SRC      = ./rsc/main.cpp \
           ./rsc/utils/utils.cpp \
           ./rsc/Config/Config.cpp \
           ./rsc/response_cgi/CGI.cpp \
           ./rsc/response_cgi/Response.cpp \
           ./rsc/server/server_socket.cpp \
           ./rsc/http_client/http_client.cpp \
           ./rsc/http_client/Request_Struct.cpp \
           ./rsc/Request/StateMachine.cpp \
           ./rsc/Request/Request.cpp \
           ./rsc/Request/RequestUtils.cpp \
           ./rsc/Request/ParseHeader.cpp \
           ./rsc/Request/ParseBody.cpp \
           ./rsc/cookies/session.cpp \
           ./rsc/cookies/session_manager.cpp
OBJ      = $(SRC:.cpp=.o)

# Rules
all: $(TARGET)
	@printf "\033[2K\r$(GREEN)🌐 Webserv ready! $(RESET)\n"

$(TARGET): $(OBJ)
	@printf "\033[2K\r$(CYAN)🔗 Linking$(RESET)"
	@$(CXX) $(OBJ) -o $(TARGET)

%.o: %.cpp
	@printf "\033[2K\r$(CYAN)⚙️ Compiling $(notdir $<)$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@printf "\033[2K\r$(RED)🧹 Cleaning objects$(RESET)"
	@rm -f $(OBJ)
fclean: clean
	@printf "\033[2K\r$(RED)🗑️ Cleaning $(TARGET)! $(RESET)\n"
	@rm -f $(TARGET)

re: fclean all
	@rm -f $(OBJ)


.PHONY: all clean fclean re