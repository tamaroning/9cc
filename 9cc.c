#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED, //記号
    TK_NUM, //整数トークン
    TK_EOF, //入力の終了
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;//トークンの型
    Token *next;//次のトークン
    int val;//TK_NUMの場合、値
    char *str;//トークン文字列
};

char *user_input; // argv1

//現在注目してるトークン
Token *token;


//エラーを報告するための関数
//引数はprintfと同じ
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// reports an error location and exit.
void error_at(char *loc, char *fmt, ...) {
    va_list ap; // 可変長引数を作成
    va_start(ap,fmt);

    int pos = loc - user_input; //posバイト目でエラー
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); //pos個のスペース
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

//次のトークンが期待されている記号のときは、トークンを読み進めて
//trueを返す、それ以外はfalse
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

//次のトークンが期待されている記号のときはトークンを一つ読み進める
//それ以外はエラー
void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op)
        error_at(token->str, "expected '%c'", op);
    token = token->next;
}

//次のトークンが数値のとき、トークンを一つ読み進めて数値を返す
//それ以外はエラー
// ensure that the current token is TK_NUM
int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "expected a number");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof(){
    return token->kind == TK_EOF;
}

//新しいトークンを作成、curにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

//入力文字列pをトークナイズ、連結リストを構築
Token *tokenize(char *p){
    Token head; //dummy
    head.next = NULL;
    Token *cur = &head;

    while (*p){
        if(isspace(*p)) {
            p++;
            continue;
        }

        if(*p == '+' || *p == '-'){
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)){
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }
        
        error("cannot tokenize");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}


int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(argv[1]);

    //アセンブリの前半
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    //式の最初は数
    printf("    mov rax, %d\n", expect_number());

    while (!at_eof()) {
        if (consume('+')) {
            printf("    add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    return 0;
}
