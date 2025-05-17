#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>

#define CURREN_DIR "."

bool execute_ls(const char* directory);

int main (int argc, const char* argv[]) {
    const char* directory = (argc > 1) ? argv[1] : CURREN_DIR;

    if (!execute_ls(directory)) {
        printf("Error reading the directory!!!");
        return EXIT_FAILURE;
    }

    return 0;
}

bool execute_ls(const char* directory) {
    DIR* dir;
    struct dirent * entry;
    
    dir = opendir(directory);

    if (!dir) {
        return false;
    }
    
    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        printf("%s\n", entry->d_name);
    };

    closedir(dir);

    return true;
}
