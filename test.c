#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_COMMAND_LENGTH 128

typedef enum{ // На основе этого enum'а переходим в следующее состояние
    ENCODE,
    DECODE,
    INCORRECT_COMAND
}CommandInputResult;

typedef enum{ // На основе этого enum'а мы выводим ошибки в консоль
    INCORRECT_ENCODE_PATH,
    ENCODE_SUCCESS,
    INCORRECT_DECODE_DIRECTORY, // Указанная директория не была сжата данной программой  
    INCORRECT_DECODE_PATH,
    DECODE_SUCCESS
}QueryResult;

typedef enum{
    COMMAND_INPUT, // Вводим команду, начальное состояние
    QUERY, // Если ок - то encode|decode, переход в COMMAND_INPUT и вывод логов, иначе - CONFIRM_RESET и вывод ошибок
    CONFIRM_RESET, //(y/n) y - RESET_PATH, n - возврат в начальное состояние
    RESET_PATH, // если корректный путь, то назад в encode|decode, иначе CONFIRM_RESET
}ProgramState;

// void list_files(const char *dir_name){
//     DIR *dir = opendir(dir_name);
//     if(!dir){
//         return;
//     }

//     struct dirent* entity;
//     entity = readdir(dir);
//     while(entity){

//     }
// }



void parse_input(char input[MAX_COMMAND_LENGTH]){
    printf("input: %s\n", input);
    int l1 = -1, l2 = -1, flag = 0;
    char prev_symbol = '\0';
    for(int i = 0; i < strlen(input); i++){
        if(input[i] != ' ' && (!prev_symbol || prev_symbol == ' ')){
            if (l1 == -1) {l1 = i;} else {l2 = i; break;}
        }
        prev_symbol = input[i];
    }

    
    // if(l1 == -1 || l2 == -1 || l2 - l1 < 7){
    //     printf("Incorrect input!\n");
    // }else{
    //     char *word1 = malloc(l2 - l1 - 1);
    //     memset(word1, '\0', l2 - l1 - 1);
    //     for(int i = 0; i < l2 - l1 - 1; i++){
    //         *(word1 + i) = input[l1 + i];
    //     }
    //     printf("%s\n", word1);
    // }
}


int main(){
    // ProgramState curr_state = COMMAND_INPUT, prev_state;
    // char entred_command[MAX_COMMAND_LENGTH] = {'\0'}, *left_word, *right_word;
    // scanf("%[^\n]", entred_command);
    // parse_input(entred_command);
    DIR *dir = opendir(".");
    struct dirent* some;
    some = readdir(dir);

    while (some != NULL){
        printf("%s\n",some->d_type);
        some = readdir(dir);
    }

    closedir(dir);
    // char reset[128] = {'\0'};
    // scanf("%[^\n^ ]", reset);
    // if(!strcmp(reset, "y")){
    //     printf("YES\n");
    // }else if(!strcmp(reset, "n")){
    //     printf("NO\n");
    // }

    // while(1){
    //     switch (curr_state)
    //     {
    //     case:
    //         break;
        
    //     default:
    //         break;
    //     }
    // } 
}