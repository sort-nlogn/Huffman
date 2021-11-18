#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_COMMAND_LENGTH 128
#define MAX_PATH_LENGTH 128
#define MAX_ARG_COUNT 128

#define CLEAR_BUFFER(buffer, n, m)  for(size_t i = 0; i < n; i++){\
                                        for(size_t j = 0; j < n; j++){\
                                            buffer[i][j] = 'a';\
                                        }\
                                    }

typedef enum{ // На основе этого enum'а переходим в следующее состояние
    ENCODE,
    DECODE
}query_type;

typedef enum{ // На основе этого enum'а мы выводим ошибки в консоль
    INCORRECT_ENCODE_PATH,
    ENCODE_SUCCESS,
    INCORRECT_DECODE_DIRECTORY, // Указанная директория не была сжата данной программой  
    INCORRECT_DECODE_PATH,
    DECODE_SUCCESS
}query_result;

typedef enum{
    COMMAND_INPUT, // Вводим команду, начальное состояние
    QUERY, // Если ок - то encode|decode, переход в COMMAND_INPUT и вывод логов, иначе - CONFIRM_RESET и вывод ошибок
    CONFIRM_RESET, //(y/n) y - RESET_PATH, n - возврат в начальное состояние
    RESET_PATH, // если корректный путь, то назад в encode|decode, иначе CONFIRM_RESET
}program_state;

// void str_copy(char *dest, char *crc, int max_lenght, char s){
// }

void handle_query(const char *dir_name, const void (*handler)(const char*file_name, const char *dir_name)){
    DIR *dir = opendir(dir_name);
    struct dirent *entity;
    struct stat s;
    while(entity){
        entity = readdir(dir);
        if(!strcmp(entity->d_name, ".") || !strcmp(entity->d_name, "..")){
            continue;
        }
        char path[256] = {'\0'};
        strcat(path, dir_name);
        strcat(path, "\\");
        strcat(path, entity->d_name);
        stat(path, &s);
        if(s.st_mode & S_IFDIR){
            handle_query(path, handler);
        }else{
            handler(entity->d_name, dir_name); 
        }
    }
}


query_type parse_input(char *input, char *left_word, char *right_word){
    int l = strlen(input), l1 = -1, l2 = -1, w1_l = 0, w2_l = 0;
    for(int i = 0; i < l; i++){
        while(i < l & input[i] == ' '){i++;}
        if(i == l - 1) {break;}
        if(l1 == -1){
            l1 = i;
            while(i < l - 1 & input[i] != ' '){w1_l++, i++;}
        }else{
            l2 = i;
            while(i < l - 1 & input[i] != ' '){w2_l++, i++;}
            break;
        }
    }
    if(l1 == -1 || l2 == -1){return INCORRECT_COMAND;}
    strncpy(left_word, input + l1, w1_l);
    strncpy(right_word, input + l2, w2_l);
    if(!strcmp(left_word, "encode")){return ENCODE;}
    if(!strcmp(left_word, "decode")){return DECODE;}
    return INCORRECT_COMAND;
}

size_t parse_command(char *command, char arg_buffer[MAX_ARG_COUNT][MAX_PATH_LENGTH]){
    size_t len = strlen(command), l = -1, r = -1, words_count = 0;
    for(size_t i = 0; i < len; i++){
        if(command[i] == ' ' || command[i] == '\n'){
            if(l != -1){
                strncpy(arg_buffer[words_count++], command + l, r - l + 1);
                l = -1, r = -1;
            }
        }else{
            if(l == -1){
                l = i;
            }else{
                r = i;
                if(r == len - 1){
                    strncpy(arg_buffer[words_count++], command + l, r - l + 1);
                    break;
                }
            }
        }
    }

    for(size_t i = 0; i < words_count; i++){
        printf("%s\n", arg_buffer[i]);
    }

    return words_count;
}

int main(){
    program_state curr_state = COMMAND_INPUT;
    char entred_command[MAX_COMMAND_LENGTH] = {'\0'};
    char arg_buffer[MAX_ARG_COUNT][MAX_PATH_LENGTH] = {{'\0'}};
    fgets(entred_command, MAX_COMMAND_LENGTH, stdin);
    parse_command(entred_command, arg_buffer);
}