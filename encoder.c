/*
    =================       ==========================
    |ЖЁСТКО НАСЖИМАЛ|       |ПОЛНУЮ ДИРЕКТОРИЮ ФАЙЛОВ|
    =================       ==========================
*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define p_len(fname, ent) strlen(fname) - strlen(ent->d_name)
#define file_f "0%.*s %s\n"
#define efile_f "1%.*s %s\n"
#define efolder_f "2%s\n"

#define get_code_value(i) *(codes + 128 + (int)nodes[i].symbol)
typedef struct TmpNode{
    struct TmpNode *left, *right, *parent;
    int symbol;
    unsigned int freq;
} Node;

typedef struct{
    char value[257];
    int length;
} Code;

typedef struct{
    Node *node;
    int visited_cnt;
} StackItem;

int cmp(const void *n1, const void *n2){
    int freq1 = ((Node *)n1)->freq;
    int freq2 = ((Node *)n2)->freq;
    if (!freq1)
        return freq2;
    if (!freq2)
        return -freq1;
    return freq1 - freq2;
}

int set_frequences(FILE *in, Node nodes[513]){
    char c = fgetc(in);
    int alphabet_size = 0;
    nodes[256].freq = 1;
    nodes[256].symbol = 128; //pseudo-EOF
    while (!feof(in)){
        nodes[c + 128].freq++;
        if (nodes[c + 128].freq == 1){ // если первый раз встретили этот символ
            alphabet_size++;
            nodes[c + 128].symbol = c;
        }
        c = fgetc(in);
    }

    qsort(nodes, 257, sizeof(Node), cmp);

    // первые alpahabet_size + 1(+EOF) нод отсортированы, всё остальное - нули;
    return alphabet_size + 1; // +EOF
}

void node_init(Node *new_node, Node *left, Node *right){
    new_node->left = left;
    new_node->right = right;
    new_node->freq = left->freq + right->freq;
    new_node->symbol = '$';
    left->parent = new_node;
    right->parent = new_node;
}

unsigned int get_freq_sum(Node *arr1, Node *arr2, int i1, int i2, int l1, int l2){
    if (i1 == l1 || i2 == l2){
        return UINT_MAX;
    }
    return (arr1 + i1)->freq + (arr2 + i2)->freq;
}

void build_tree(Node arr1[513], int alphabet_size){
    Node *arr2 = arr1 + 257; // с этого адреса начинаются внутренние ноды
    int i = 0, j = 0;
    for (int k = 0; k < alphabet_size - 1; k++){
        unsigned int s1 = get_freq_sum(arr1, arr1, i, i + 1, alphabet_size, alphabet_size);
        unsigned int s2 = get_freq_sum(arr2, arr2, j, j + 1, k, k);
        unsigned int s3 = get_freq_sum(arr1, arr2, i, j, alphabet_size, k);
        if (s1 < s2 && s1 < s3){
            node_init(arr2 + k, arr1 + i++, arr1 + i++);
        }else if (s2 < s1 && s2 < s3){
            node_init(arr2 + k, arr2 + j++, arr2 + j++);
        }else{
            node_init(arr2 + k, arr2 + j++, arr1 + i++);
        }
    }
}

void get_code(Node *leaf, Node *root, Code *code){
    int len = 0;
    do{
        if (leaf->parent->left == leaf){
            code->value[len++] = '0';
        }else{
            code->value[len++] = '1';
        }
        leaf = leaf->parent;
    } while (leaf != root);
    code->length = len;
}

void get_codes(Code codes[257], Node nodes[513], const int alphabet_size){
    Node *root = nodes + 257 + alphabet_size - 2;
    for (int i = 0; i < alphabet_size; i++){
        get_code(nodes + i, root, codes + (int)nodes[i].symbol + 128);
    }
}

void encode_file(FILE *in, FILE *out, Code codes[257]){
    char c = fgetc(in), curr_byte = 0;
    int byte_pos = 0, code_pos = 0;
    Code curr_code = codes[128 + c];
    while (!feof(in)){
        curr_byte |= (curr_code.value[curr_code.length - 1 - code_pos++] == '0' ? 0 : 1) << byte_pos++;
        if (byte_pos == 8){
            fputc(curr_byte, out);
            curr_byte = 0;
            byte_pos = 0;
        }
        if (code_pos == curr_code.length){
            c = fgetc(in);
            curr_code = codes[128 + c];
            code_pos = 0;
        }
    }
    //write pseudo-EOF
    Code eof_code = codes[256]; code_pos = 0;
    while(code_pos != eof_code.length){
        if(byte_pos == 8){
            fputc(curr_byte, out);
            curr_byte = 0;
            byte_pos = 0;
        }
        curr_byte |= (eof_code.value[eof_code.length - 1 - code_pos++] == '0' ? 0 : 1) << byte_pos++;
    }
    fputc(curr_byte, out);
}

//find eof pos and generate alphabet string which contain sybols in dfs-order
int dfs(Node *root, FILE *out, short *eof_pos, int curr_pos, char s[]){ 
    if (root->left){
        curr_pos = dfs(root->left, out, eof_pos, curr_pos, s);
        curr_pos = dfs(root->right, out, eof_pos, curr_pos, s);
        return curr_pos;
    }else{
        if (root->symbol == 128){
            *eof_pos = curr_pos;
            return curr_pos;
        }else{
            s[curr_pos] = root->symbol;
            return ++curr_pos;
        }
    }
}

void dump_alphabet(FILE *out, Node *root, int alphabet_size){//write alphabet in dfs-order
    short eof_pos = 0;
    short as = alphabet_size;
    char alphabet[257] = {'\0'};
    dfs(root, out, &eof_pos, 0, alphabet);
    fwrite(&as, sizeof(short), 1, out);
    fwrite(&eof_pos, sizeof(short), 1, out);
    for (int i = 0; i < alphabet_size - 1; i++){
        fputc(alphabet[i], out);
    }
}

void dump_traverse_string(FILE *out, Node *root){ // dfs-based generation of traverse string
    StackItem st[513] = {{.node = NULL}, {.visited_cnt = 0}};
    size_t head = 1, pos = 0;
    char byte = 0, last = 0;
    st[0].node = root;
    while (head){
        Node *curr = st[head - 1].node;
        if (!curr->left){
            st[--head].visited_cnt = 0;
            pos++;
        }else{
            if (!st[head - 1].visited_cnt){
                byte |= 1 << pos++;
                st[head - 1].visited_cnt++;
                st[head++].node = curr->left;
            }else if (st[head - 1].visited_cnt == 1){
                byte |= 1 << pos++;
                st[head - 1].visited_cnt++;
                st[head++].node = curr->right;
            }else{
                st[--head].visited_cnt = 0;
                if (curr->parent){
                    pos++;
                }
            }
        }
        if (pos >= 8){
            last = (byte >> 7) & 1;
            fputc(byte, out);
            pos %= 8;
            byte = 0;
        }
    }
    if (byte){
        fputc(byte, out);
    }else if (last){
        fputc(byte, out);
    }
}

int is_empty_file(char *path){
    FILE *f = fopen(path, "r");
    fseek (f, 0, SEEK_END);
    int size = ftell(f);
    fclose(f);
    return !size;
}

int is_regular_file(const char *path){
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void dump_file(char *full_path, FILE *archive){
    FILE *in = fopen(full_path, "rb");
    Node nodes[513] = {{.parent = NULL},
                       {.left = NULL},
                       {.right = NULL},
                       {.freq = 0},
                       {.symbol = 0}};
    int alphabet_size = set_frequences(in, nodes);

    Code codes[257] = {{.value = {'\0'}},
                       {.length = 0}};

    build_tree(nodes, alphabet_size);
    get_codes(codes, nodes, alphabet_size);
    for(int i = 0; i < alphabet_size; i++){
        if(nodes[i].symbol < 128){
            printf("%d code: %s length: %d\n", nodes[i].symbol, get_code_value(i), codes[128 + nodes[i].symbol].length);
        }else{
            printf("EOF code: %s length: %d\n", get_code_value(i), codes[128 + nodes[i].symbol].length);
        }
    }printf("\n");
    fseek(in, 0, SEEK_SET);
    Node *root = nodes + 257 + alphabet_size - 2;
    dump_alphabet(archive, root, alphabet_size);
    dump_traverse_string(archive, root);
    encode_file(in, archive, codes);
    fclose(in);
}

void dump_archive(const char *curr, const char *base, FILE *archive){
    DIR *dir = opendir(curr);
    if(!(dir = opendir(curr)))
        return;
    struct dirent *ent;
    int file_cnt = 0;
    while(ent = readdir(dir)){
        if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")){
            continue;
        }
        char path[1024] = {'\0'};
        snprintf(path, sizeof(path), "%s\\%s", curr, ent->d_name);
        if(!is_regular_file(path)){
            dump_archive(path, base, archive);
        }else{ 
            char *fname = path + strlen(base);
            if(is_empty_file(path)){ // empty file
                fprintf(archive, efile_f, p_len(fname, ent), fname, ent->d_name);
            }else{ // regular file
                fprintf(archive, file_f, p_len(fname, ent), fname, ent->d_name);
                dump_file(path, archive);
            }
        }
        file_cnt++;
    }
    if(!file_cnt){ // empty folder
        fprintf(archive, efolder_f, curr + strlen(base));
    }
    closedir(dir);
}

int main(int argc, char *argv[]){
    FILE *in, *out;

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
    if(!strncmp(argv[1], argv[2], l1) || !opendir(argv[2])){
        printf("Incorrect save path");
        exit(0);
    }

    char archive_path[1024] = {'\0'};
    snprintf(archive_path, sizeof(archive_path), "%s\\%s", argv[2], "archive.hf"); 
    out = fopen(archive_path, "wb");
    dump_archive(argv[1], argv[1], out);

    fclose(out);
    return 0;
}
