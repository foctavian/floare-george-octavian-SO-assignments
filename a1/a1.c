#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>

enum flags {
    MAGIC, VERSION, SECT_NR, SECT_TYPES
};

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

    //regular listing with no additional filters
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
    else if (_size_greater != -1 && name_starts_with == NULL) {
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
    else if (_size_greater != -1 && name_starts_with == NULL) {
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

void parse(const char* path) {
    int _version = 0;
    char _magic = 0;
    int _sect_nr = 0;
    int _header_size = 0;
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("ERROR\ncouldn't open file");
        return;
    }

    //move to the end of the file
    lseek(fd, 0, SEEK_END);

    //error flag vector for
    //every entry of the header
    int error[5];
    error[4] = 0;
    //move the cursor a specific number of bytes
    //to find the characteristics of the SF

    //read magic
    lseek(fd, -1, SEEK_CUR);
    read(fd, &_magic, 1);
    if (_magic != 'D') {
        error[MAGIC] = -1;
        error[4] = -1;
    }

    //read header_size
    lseek(fd, -3, SEEK_CUR);
    read(fd, &_header_size, 2);

    //back to the beginning of the header
    //read version
    lseek(fd, -_header_size, SEEK_END);
    read(fd, &_version, 2);
    if (_version < 97 && _version > 195) {
        error[VERSION] = -1;
        error[4] = -1;
    }
    read(fd, &_sect_nr, 1);
    if (_sect_nr < 6 && _sect_nr > 16) {
        error[SECT_NR] = -1;
        _sect_nr = -1;
        error[4] = -1;
    }

  
    char sect_name[_sect_nr][8];
    char sect_type[_sect_nr];
    int sect_offset[_sect_nr];
    int sect_size[_sect_nr];
    if (_sect_nr != -1 && _version != -1 && _magic != -1) {
        for (int i = 0; i < _sect_nr; i++) {
            read(fd, &sect_name[i], 7);
            sect_name[i][7] = '\0';
            read(fd, &sect_type[i], 1);
            if ( sect_type[i] != 78 &&
                    sect_type[i] != 17 &&
                    sect_type[i] != 15 &&
                    sect_type[i] != 66 &&
                    sect_type[i] != 27 &&
                    sect_type[i] != 70) {
                error[SECT_TYPES] = -1;
                error[4] = -1;
                break;
            }
            read(fd, &sect_offset[i], 4);
            read(fd, &sect_size[i], 4);

        }
    }


    //validation
    int pos = -1;
    if (error[4] == 0) {
        printf("SUCCESS\n");
        printf("version=%d\n", _version);
        printf("nr_sections=%d\n", _sect_nr);

        for (int i = 0; i < _sect_nr; i++) {
            printf("section%d: %s %d %d\n", i + 1, sect_name[i], sect_type[i], sect_size[i]);
        }
        printf("\n");
    }
    else {
        for (int i = 0; i < 4; i++) {
            if (error[i] == -1) {
                if (i == MAGIC) {
                    printf("wrong magic");
                    pos = i;
                    break;
                } else if (i == VERSION) {
                    printf("|wrong version");
                    pos = i;
                    break;
                } else if (i == SECT_NR) {
                    printf("|wrong sect_nr");
                    pos = i;
                    break;
                } else if (i == SECT_TYPES) {
                    printf("|wrong sect_types");
                    pos = i;
                    break;
                }
            }
            else {
                if (i == MAGIC) {
                    printf("magic");
                } else if (i == VERSION) {
                    printf("|version");
                } else if (i == SECT_NR) {
                    printf("|sect_nr");
                } else if (i == SECT_TYPES) {
                    printf("|sect_types");
                }
            }
        }
    }

    if (pos != -1 && pos != 3) {
        for (int i = pos + 1; i < 4; i++) {
            if (i == MAGIC) {
                printf("magic");
            } else if (i == VERSION) {
                printf("|version");
            } else if (i == SECT_NR) {
                printf("|sect_nr");
            } else if (i == SECT_TYPES) {
                printf("|sect_types");
            }
        }
    }
    printf("\n");


    return;
}

int main(int argc, char **argv) {

    int _list = 0;
    int _recursive = 0;
    int _parse = 0;
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
                if (strcmp(argv[i], "parse") == 0) {
                    _parse = 1;
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
    else if (_parse == 1) {
        parse(path);
    }
    free(path);
    free(name_starts_with);
    return 0;
}