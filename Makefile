NAME = ircserv
SRC = client.cpp server.cpp main.cpp
OBJ = $(SRC:.cpp=.o)
INC = server.hpp client.hpp main.hpp
CC = c++
FLAGS = -Wall -Wextra -Werror -std=c++98
SRC_DIR = src/
OBJ_DIR = obj/
INC_DIR = inc/
SRC_FILES = $(addprefix $(SRC_DIR), $(SRC))
OBJ_FILES = $(addprefix $(OBJ_DIR), $(OBJ))
INC_FILES = $(addprefix $(INC_DIR), $(INC))

all: $(NAME)

$(NAME): $(OBJ_FILES)
	$(CC) $(FLAGS) $(OBJ_FILES) -o $(NAME)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(INC_FILES)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(FLAGS) -I $(INC_DIR) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
