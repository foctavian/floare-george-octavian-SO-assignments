#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>

int starts_with(char* input, char* prefix) {

    return strncmp(prefix, input, strlen(prefix));
}

void list_recursive(const char* path, off_t _size_greater, char* name_starts_with) {
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[4096];
    struct stat statbuf;

    dir = opendir(path);
    if (dir == NULL) {
        perror("ERROR\ninvalid directory path\n");
        return;
    }

    //regular listing with not additional filters
    if (_size_greater == -1 && name_starts_with == NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                snprintf(fullPath, 4096, "%s/%s", path, entry->d_name);
                if (lstat(fullPath, &statbuf) == 0) {
                    printf("%s\n", fullPath);
                    if (S_ISDIR(statbuf.st_mode)) {
                        list_recursive(fullPath, _size_greater, name_starts_with);
                    }
                }
            }
        }
    }
    else if (_size_greater == -1 && name_starts_with != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                if (starts_with(entry->d_name, name_starts_with) == 0) {
                    snprintf(fullPath, 4096, "%s/%s", path, entry->d_name);
                    if (lstat(fullPath, &statbuf) == 0) {
                        printf("%s\n", fullPath);
                        if (S_ISDIR(statbuf.st_mode)) {
                            list_recursive(fullPath, _size_greater, name_starts_with);
                        }
                    }
                }
            }
        }
    }
    else if(_size_greater != -1 && name_starts_with == NULL){
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                snprintf(fullPath, 4096, "%s/%s", path, entry->d_name);
                if (lstat(fullPath, &statbuf) == 0) {
                    if (S_ISREG(statbuf.st_mode) &&
                            statbuf.st_size > _size_greater) {
                        printf("%s\n", fullPath);
                    }
                    if (S_ISDIR(statbuf.st_mode)) {
                        list_recursive(fullPath, _size_greater, name_starts_with);
                    }
                }
            }
        }
    }
    closedir(dir);
}

void list_simple(const char* path, off_t _size_greater, char* name_starts_with) {
    DIR* dir = NULL;
    struct dirent *entry = NULL;
    char file[1024];
    struct stat statbuf;
    dir = opendir(path);
    if (dir == NULL) {
        perror("ERROR\ninvalid directory path\n");
        return;
    }
    if (_size_greater == -1 && name_starts_with == NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 &&
                    strcmp(entry->d_name, "..") != 0) {
                snprintf(file, 1024, "%s/%s", path, entry->d_name);
                printf("%s\n", file);
            }
        }
        closedir(dir);
    } else if (_size_greater == -1 && name_starts_with != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (starts_with(entry->d_name, name_starts_with) == 0) {
                snprintf(file, 1024, "%s/%s", path, entry->d_name);
                printf("%s\n", file);
            }
        }
    }
    else if(_size_greater != -1 && name_starts_with == NULL){
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 &&
                    strcmp(entry->d_name, "..") != 0) {
                snprintf(file, 1024, "%s/%s", path, entry->d_name);
                if (lstat(file, &statbuf) == 0) {
                    if (S_ISREG(statbuf.st_mode) &&
                            statbuf.st_size > _size_greater) {
                        printf("%s\n", file);
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv) {

    int _list = 0;
    int _recursive = 0;
    off_t _size_greater = -1;
    char *name_starts_with = NULL;
    char *path = NULL;


    if (argc >= 2) {
        if (strcmp(argv[1], "variant") == 0) {
            printf("55635\n");
        }
        else {
            for (int i = 1; i < argc; i++) {
                if (strcmp(argv[i], "list") == 0 ) {
                    _list = 1;
                }
                if (strcmp(argv[i], "recursive") == 0) {
                    _recursive = 1;
                }
                else {
                    char* token = strtok(argv[i], "=");
                    if (strcmp(token, "path") == 0) {
                        token = strtok(NULL, "=");

                        path = calloc(strlen(token) + 3, sizeof(char));
                        if (path == NULL) {
                            return -1;
                        }
                        strncpy(path, token, strlen(token) + 1);
                    }
                    if (strcmp(token, "size_greater") == 0) {
                        token = strtok(NULL, "=");
                        char** endptr = NULL;
                        _size_greater = strtoll(token, endptr, 10);
                    }
                    else if (strcmp(token, "name_starts_with") == 0) {
                        token = strtok(NULL, "=");
                        name_starts_with = calloc(strlen(token) + 1 , sizeof(char));
                        if (name_starts_with == NULL) {
                            return -1;
                        }
                        strncpy(name_starts_with, token, strlen(name_starts_with) + 1);
                    }
                }
            }
        }
    }

    if (_list == 1 && _recursive == 1) {
        printf("SUCCESS\n");
        list_recursive(path, _size_greater, name_starts_with);
    }
    else if (_list == 1 && _recursive == 0) {
        printf("SUCCESS\n");
        list_simple(path, _size_greater, name_starts_with);
    }



    free(path);
    free(name_starts_with);
    return 0;
}