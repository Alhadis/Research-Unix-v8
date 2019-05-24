// <string.h>: the C string functions, UNIX Programmer's Manual vol 1 section 2

extern char* strcat(char*, char*);
extern char* strncat(char*, char*, int);
extern int strcmp(char*, char*);
extern int strncmp(char*, char*, int);
extern int strcpy(char*, char*);
extern int strncpy(char*, char*, int);
extern int strlen(char*);
// bsd:
//extern char* index(char*, int);
//extern char* rindex(char*, int);
// system V:
//extern char* strchr(char*, char);
//extern char* strrchr(char*, char);
//extern char* strpbrk(char*, char*);
//extern int strspn(char*, char*);
//extern int strcspn(char*, char*);
//extern char* strtok(char*, char*);
