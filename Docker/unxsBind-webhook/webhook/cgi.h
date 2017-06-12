/*
FILE
	svn ID removed
LEGAL
	Public Domain. See cgi.c file for more info
*/

typedef struct {
    char *name;
    char *val;
} pentry;

typedef struct {
    char name[128];
    char val[128];
} entry;

void getword(char *word, char *line, char stop);
char x2c(char *what);
void unescape_url(char *url);
void plustospace(char *str);
void spacetoplus(char *str);
char *makeword(char *line, char stop);
char *fmakeword(FILE *f, char stop, int *len); 
void escape_shell_cmd(char *cmd);
