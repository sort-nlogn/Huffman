#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define p_len(fname, ent) strlen(fname) - strlen(ent->d_name)
#define file_f "0%.*s %s\n"
#define efile_f "1%.*s %s\n"
#define efolder_f "2%s\n"

int is_empty_file(char *path){
    FILE *f = fopen(path, "r");
    fseek (f, 0, SEEK_END);
    int size = ftell(f);
    fclose(f);
    return !size;
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void dump_file(const char *fr_path, const char *base, int is_empty){
    char cmnd[1024] = {'\0'};
    snprintf(cmnd, sizeof(cmnd), "mkdir %s%.*s", base, strrchr(fr_path, '\\')-fr_path+1, fr_path);
    if(!opendir(strrchr(cmnd, ' ') + 1)){
        system(cmnd);
    }
    char ptf[1024] = {'\0'};
    snprintf(ptf, sizeof(ptf), "%s%s", strrchr(cmnd, ' ')+1, strrchr(fr_path, '\\')+1);
    FILE *f = fopen(ptf, "w");
    fclose(f);
}

void dump_empty_folder(const char *path, const char *base){
    char cmnd[1024] = {'\0'};
    snprintf(cmnd, sizeof(cmnd), "mkdir %s%s", base, path);
    if(!opendir(strrchr(cmnd, ' ') + 1)){
        system(cmnd);
    }
}

void dir_cpy(const char *curr, const char *src, const char *dst){
    DIR *dir = opendir(curr);
    struct dirent *ent;
    int cnt = 0;
    while(ent = readdir(dir)){
        if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
            continue;
        char path[256] = {'\0'};
        snprintf(path, sizeof(path), "%s\\%s", curr, ent->d_name);
        if(!is_regular_file(path)){
            dir_cpy(path, src, dst);
        }else{ 
            char *fname = path + strlen(src);
            dump_file(fname, dst, is_empty_file(path));
        }
        cnt++;
    }
    if(!cnt){
        dump_empty_folder(curr + strlen(src), dst);
    }
    closedir(dir);
}

void list_dir(const char *dir_name, const char *base, FILE *archive){
    DIR *dir;

    if(!(dir = opendir(dir_name)))
        return;

    struct dirent *ent;
    
    int file_cnt = 0;
    while(ent = readdir(dir)){
        if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")){
            continue;
        }
        char path[256] = {'\0'};
        snprintf(path, sizeof(path), "%s\\%s", dir_name, ent->d_name);
        if(!is_regular_file(path)){
            list_dir(path, base, archive);
        }else{ 
            char *fname = path + strlen(base);
            if(!is_empty_file(path)){
                fprintf(archive, efile_f, strrchr(fname, '\\')-fname+1, fname, ent->d_name);
            }else{
                fprintf(archive, file_f, strrchr(fname, '\\')-fname+1, fname, ent->d_name);
            }
        }
        file_cnt++;
    }
    if(!file_cnt && strcmp(dir_name, base)){
        fprintf(archive, efolder_f, dir_name + strlen(base));
    }
    closedir(dir);
}

int main(int argc, char *argv[]){
    // FILE *archive = fopen("archive.hf", "wb");
    // fprintf(archive, "[BASE] %s\n", argv[1]);

    // list_dir(argv[1], argv[1], archive);
    if(argc < 3){
        printf("Too few command arguments");
        exit(0);
    }
    if(!opendir(argv[1])){
        printf("Incorrect source path");
        exit(0);
    }
    int l1 = strlen(argv[1]);
    while(argv[1][l1 - 1] == '\\'){l1--;}
    if(!strncmp(argv[1], argv[2], l1)){
        printf("Incorrect save path");
        exit(0);
    }
    dir_cpy(argv[1], argv[1], argv[2]);
}
