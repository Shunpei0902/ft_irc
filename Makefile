NAME = ircserv

SRC = main.cpp parsing.cpp utils.cpp \
	class/channel.cpp class/client.cpp class/server.cpp \
	commands/invite.cpp commands/kick.cpp commands/part.cpp \
	commands/nick.cpp commands/privmsg.cpp commands/quit.cpp \
	commands/join.cpp commands/mode.cpp commands/pass.cpp \
	commands/ping.cpp commands/topic.cpp commands/user.cpp \
	commands/cap.cpp

# OBJ = $(SRC:.cpp=.o)
OBJS = ${SRC:%.cpp=${OBJ_DIR}%.o}
INC = server.hpp client.hpp main.hpp

CXX = c++
FLAGS = -Wall -Wextra -Werror -std=c++98

SRC_DIR = src/
OBJ_DIR = obj/
INC_DIR = inc/

SRC_FILES = $(addprefix $(SRC_DIR), $(SRC))
OBJ_FILES = $(addprefix $(OBJ_DIR), $(OBJ))
INC_FILES = $(addprefix $(INC_DIR), $(INC))

all: $(NAME)

# $(NAME): $(OBJ_FILES)
#	$(CC) $(FLAGS) $(OBJ_FILES) -o $(NAME)
$(NAME): $(OBJS)
	$(CXX) $(FLAGS) $(OBJS) -o $(NAME)

$(OBJS) : $(OBJ_DIR)%.o : $(SRC_DIR)%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(FLAGS) -I $(INC_DIR) -c $< -o $@


# 静的ルールを生成する
# $(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
# 	mkdir -p $(dir $@)
# 	$(CC) $(FLAGS) -I $(INC_DIR) -c $< -o $@

# これがポイント！依存を静的に書き出す
$(OBJ_FILES): | $(OBJ_DIRS)

$(OBJ_DIRS):
	mkdir -p $(OBJ_DIR)class $(OBJ_DIR)commands

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

