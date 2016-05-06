#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    char *name;
    char *telephoneNumber;
} Person;

typedef struct {
    int length;
    Person *data;
} TelBook;

TelBook book;
char *FileName;
int end = 0;

// WTF - "WriteToFile"
void WTF() {
    FILE *file = fopen(FileName, "wt");
    int i;
    for (i = 0; i < book.length; i++) {
        fprintf(file, "%d %s %s\n", book.data[i].id, book.data[i].name, book.data[i].telephoneNumber);
    }
    fclose(file);
}

int Confirm (char c, int cond) {
    int array[3];
    array[1]=isalpha(c);
    array[2]=isdigit(c);
    array[3]=isalnum(c);
    return array[cond];
}

char *getting(FILE *input, int cond) {
    int l = 0, size = 1, flag = 0;
    char *gotten = NULL, c = fgetc(input);
    while (c != EOF) {
        if (!isspace(c)) {
            if (cond == 2) {
                cond = isalpha(c) ? 0 : 1;
            }
            if (Confirm(c, cond)) {
                if ((l + 1) == size) {
                    size *= 2;
                    gotten = realloc(gotten, size * sizeof(char));
                }
                gotten[l++] = c;
                gotten[l] = '\0';
            } else if ((c == '+' && l > 0) || (cond == 0) || (cond != 0 && c != '(' && c != ')' && c != '-' && c != '+')) {
                flag = 1;
            }
        } else if (l > 0) {
            break;
        }
        c = fgetc(input);
    }
    if (flag == 1) {
        free(gotten);
        gotten = NULL;
        printf("Strange input... I can't fix it\n");
    }
    return gotten;
}

int Compare(char *s1, char *s2) {
    int i = 0;
    char *ss1 = malloc(strlen(s1) * sizeof(char));
    while (s1[i] != '\0') {
        ss1[i] = tolower(s1[i]);
        i++;
    }
    ss1[i] = '\0';
    i = 0;
    char *ss2 = malloc(strlen(s2) * sizeof(char));
    while (s2[i] != '\0') {
        ss2[i] = tolower(s2[i]);
        i++;
    }
    ss2[i] = '\0';
    int res = (strstr(ss1, ss2) != NULL);
    free(ss1);
    free(ss2);
    return res;
}

int getId(int id) {
    int i;
    for (i = 0; i < book.length; i++) {
        if (book.data[i].id == id) {
            return i;
        }
    }
    return -1;
}

void find() {
    char *request = getting(stdin, 2);
    if (request != NULL) {
        int found = 0;
        int i;
        for (i = 0; i < book.length; i++) {
            if ((isdigit(request[0]) && !strcmp(book.data[i].telephoneNumber, request)) ||
                (isalpha(request[0]) && Compare(book.data[i].name, request))) {
                printf("%d %s %s\n", book.data[i].id, book.data[i].name, book.data[i].telephoneNumber);
                found = 1;
            }
        }
        if (!found) {
            printf("Ooooops... There is no such contact \n");
        }
    } else {
        printf("Something gone wrong. Try again. Maybe by help of God everything will fix\n");
    }
    free(request);
}

void delete(int id){
    id = getId(id);
    if(id == -1){
        printf("Wrong ID. No such contact, honey.\n");
        return;
    }
    book.length--;
    free(book.data[id].name);
    free(book.data[id].telephoneNumber);
    book.data[id].id = book.data[book.length].id;
    book.data[id].name = book.data[book.length].name;
    book.data[id].telephoneNumber = book.data[book.length].telephoneNumber;
    book.data = (Person *) realloc(book.data, book.length * sizeof(Person));
    WTF();
}

void change() {
    int id, cond;
    char command[7], *cur;
    scanf("%d %s", &id, command);
    cond = strcmp(command, "number") ? 0 : 1;
    cur = getting(stdin, cond);
    if (cur != NULL) {
        id = getId(id);
        if (id == -1) {
            printf("Wrong ID. No such contact, honey.\n");
            free(cur);
            return;
        }
        if (cond) {
            book.data[id].telephoneNumber = cur;
        } else {
            book.data[id].name = cur;
        }
        WTF();
    } else {
        printf("Something gone wrong. Try again. Maybe by help of God everything will fix\n");
        free(cur);
    }
}

void create(FILE *file, int id) {
    char *name = getting(file, 0);
    char *telephoneNumber = getting(file, 1);
    if (name != NULL && telephoneNumber != NULL) {
        if (id > end) {
            end = id;
        }
        book.data = (Person *) realloc(book.data, (book.length + 1) * sizeof(Person));
        book.data[book.length].id = id;
        book.data[book.length].name = name;
        book.data[book.length].telephoneNumber = telephoneNumber;
        book.length++;
        WTF();
    } else {
        printf("My Darling, loading failed: %d %s %s\n", id, name, telephoneNumber);
        free(name);
        free(telephoneNumber);
    }
}

int main(int argc, char **argv) {
    FileName = argv[1];
    FILE *file = fopen(FileName, "at+");
    if (file == NULL) {
        printf("Sorry, I can't open file :c \n");
        return 0;
    }
    rewind(file);
    book.length = 0;
    book.data = NULL;
    int id;
    while (fscanf(file, "%d", &id) != EOF) {
        create(file, id);
    }
    fclose(file);
    char command[7];
    while (1) {
        scanf("%s", command);
        if (!strcmp(command, "find")) {
            find();
        } else if (!strcmp(command, "create")) {
            create(stdin, end + 1);
        } else if (!strcmp(command, "delete")) {
            scanf("%d", &id);
            delete (id);
        } else if (!strcmp(command, "change")) {
            change();
        } else if (!strcmp(command, "exit")) {
            WTF();
            int i;
            for (i = 0; i < book.length; i++) {
                free(book.data[i].name);
                free(book.data[i].telephoneNumber);
            }
            free(book.data);
            return 0;
        } else {
            printf("What are you asking me to do? I don't understand. Try again!!!\n");
        }
        fflush(stdout);
    }
}
