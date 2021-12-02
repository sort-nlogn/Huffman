#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_ALPHABET_SIZE 128
#define MAX_TEXT_SIZE 100000
#define MAX_STRING_SIZE 10000

typedef struct Tnode{
    struct Tnode *left, *right, *parent;
    char symbol;
    int freq;
}node;

int key(const node *a, const node *b){
    return a->freq - b->freq;   
}

void reverse_string(char *s){
    for(int i = 0, n = strlen(s); i < n / 2; i++){
        char tmp = s[i];
        s[i] = s[n - i - 1];
        s[n - i - 1] = tmp;
    }
}

node *set_frequences(char *text, int *alphabet_size_ptr){
    int frequences[128] = {0};
    int alphabet_size = 0;
    node *nodes = malloc(MAX_ALPHABET_SIZE * sizeof(node));
    for(int i = 0; i < strlen(text); i++){
        frequences[text[i]]++;
    }
    for(int i = 0; i < 128; i++){
        if(frequences[i]){
            nodes[alphabet_size].freq = frequences[i];
            nodes[alphabet_size++].symbol = (char)i;
        }
    }
    *alphabet_size_ptr = alphabet_size;
    qsort(nodes, alphabet_size, sizeof(node), (int(*)(const void *, const void*))key);
    return nodes;
}

// void set_frequences2(node *nodes, FILE *in){
//     char *buffer = malloc(10000);
//     int cnt = 0;


// }

void update_tree(node *new_node, node *left, node *right){
    new_node->left = left;
    new_node->right = right;
    new_node->freq = left->freq + right->freq;
    new_node->symbol = '$';
    left->parent = new_node;
    right->parent = new_node;
}

int get_mins(node *arr1, node *arr2, int l1, int l2, int i, int j){
    if(i == l1 || j == l2){
        return INT_MAX;
    }
    return arr1[i].freq + arr2[j].freq;
}

node *build_tree(node *arr1, int alphabet_size){
    node *arr2 = malloc((alphabet_size - 1) * sizeof(node));
    int i = 0, j = 0;
    for(int k = 0; k < alphabet_size - 1; k++){
        int s1 = get_mins(arr1, arr1, alphabet_size, alphabet_size, i, i + 1);
        int s2 = get_mins(arr1, arr2, alphabet_size, k, i, j);
        int s3 = get_mins(arr2, arr2, k, k, j, j + 1);
        if(s1 <= s2 && s1 <= s3){
            update_tree(arr2 + k, &arr1[i++], &arr1[i++]);
        }else if(s2 <= s1 && s2 <= s3){
            update_tree(arr2 + k, &arr2[j++], &arr1[i++]);
        }else{
            update_tree(arr2 + k, &arr2[j++], &arr2[j++]);
        }
    }
    return &arr2[alphabet_size - 2];
}

int dfs(node *root, char *traverse_string, int l, FILE *f){
    if(root->symbol == '$'){
        traverse_string[l++] = 'L';
        l = dfs(root->left, traverse_string, l, f);
        traverse_string[l++] = 'U';
        traverse_string[l++] = 'R';
        l = dfs(root->right, traverse_string, l, f);
        traverse_string[l++] = 'U';
        return l;
    }else{
        fprintf(f, "%c", root->symbol);
        return l;
    }
}

int get_code(node *leaf, node *root, char *code){
    int code_len = 0;
    do{
        if(leaf->parent->left == leaf){
            code[code_len++] = '0';
        }else{
            code[code_len++] = '1';
        }
        leaf = leaf->parent;
    }while(leaf != root);
    code = realloc(code, code_len);
    return code_len;
}

void get_codes(char **codes, node *root, node *nodes, int alphabet_size){
    for(int i = 0; i < alphabet_size; i++){
        char *code = malloc(alphabet_size);
        memset(code, '\0', alphabet_size);
        get_code(nodes + i, root, code);
        reverse_string(code);
        codes[i] = code;
        printf("%c - %s\n", nodes[i].symbol, codes[i]);
    }
}

int main(){
    FILE *f, *in;

    f = fopen("tree_info.txt", "w");
    in = fopen("in.txt", "r");
    char *message = malloc(MAX_TEXT_SIZE);
    char *start = message;
    char *s;

    while(!feof(in)){
        s = fgets(message, MAX_STRING_SIZE, in);
        message += strlen(s);
    }

    int alphabet_size = 0;
    node *nodes = set_frequences(start, &alphabet_size);
    node *root = build_tree(nodes, alphabet_size);
    char *traverse_string = malloc(4 * alphabet_size - 4);
    memset(traverse_string, '\0', 4 * alphabet_size - 4);

    dfs(root, traverse_string, 0, f);
    fprintf(f, "\n");
    for(int i = 0; i < 4 * alphabet_size - 4; i++){
        fprintf(f, "%c", traverse_string[i]);
    }

    char **codes = malloc(alphabet_size * sizeof(char *));
    printf("%d\n", root->freq);
    return 0;
}