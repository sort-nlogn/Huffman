#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <dirent.h>

int is_directory_name(char* fName){
    char *p = strchr(fName, '.');
    if((!(p - fName) || !p) && strcmp(fName, ".") && strcmp(fName, "..")){
        return 1;
    }
    return 0;
}

void construct_path(char path[500], char* dirName, char *fileName){
    int l = strchr(dirName, '*') ? strlen(dirName) - 1: strlen(dirName);
    for(int i = 0; i < l; i++){
        path[i] = dirName[i];
    }
    strcat(path, fileName);
    strcat(path, "\\*");
}

void list_files(char *dirName){
    WIN32_FIND_DATA ffd;
    LARGE_INTEGER fileSize;
    HANDLE hFind;
    hFind = FindFirstFile(dirName, &ffd);
    do{
        char *fileName = ffd.cFileName;
        if (!strcmp(fileName, ".") || !strcmp(fileName, ".."))
        {
            continue;
        }
        if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
            char path[500] = {'\0'};
            construct_path(path, dirName, fileName);
            list_files(path);
        }else{
            printf("%s --- %s\n", fileName, dirName);
        }
    }while(FindNextFile(hFind, &ffd) != 0);
    FindClose(hFind);
}

int main(){
    WIN32_FIND_DATA ffd;
    LARGE_INTEGER fileSize;
    HANDLE hFind;
    list_files("C:\\Users\\acer\\Documents\\Huffman\\*");
    return 0;
}