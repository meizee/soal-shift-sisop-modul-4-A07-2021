#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdlib.h>

#define DEBUG_MODE
const int INF = 1000;
static const char *dirpath = "/home/hasna/Downloads";
static const char *logpath = "/home/hasna/SinSeiFS.log";

static const char info[] = "INFO";
static const char warn[] = "WARNING";

static const int TIME_SIZE = 30;
static const int LOG_SIZE = 1000;

/*
    Function to check if a path is decoded or not.
    True = Encoded
    False = Decoded
 */
bool isAtoZ(const char *path) {
    int len = strlen(path);
    for (int i = 0; i < len - 5 + 1; i++) {
        if (path[i] == 'A' && path[i+1] == 't' && path[i+2] == 'o' && path[i+3] == 'Z' && path[i+4] == '_') return true;
    }
    return false;
}

bool isRX(const char *path) {
    int len = strlen(path);
    for (int i = 0; i < len - 3 + 1; i++) {
        if (path[i] == 'R' && path[i+1] == 'X' && path[i+2] == '_') return true;
    }
    return false;
}

bool isAisA(const char *path) {
    for (int i = strlen(path) - 1; i >= 7; i--) {
        if (path[i-7] == 'A' && 
            path[i-6] == '_' && 
            path[i-5] == 'i' &&
            path[i-4] == 's' &&
            path[i-3] == '_' &&
            path[i-2] == 'a' && 
            path[i-1] == '_') {
                return true;
            }
    }
    return false;
}

bool isAisA_Content(const char *path) {
    bool startScan = false;
    for (int i = strlen(path) - 1; i >= 7; i--) {
        if (path[i] == '/') {
            // Hanya mengecek 1 parent directory saja.
            if (startScan == true) break;
            startScan = true;
        }
        else {
            if (path[i-7] == 'A' && 
                path[i-6] == '_' && 
                path[i-5] == 'i' &&
                path[i-4] == 's' &&
                path[i-3] == '_' &&
                path[i-2] == 'a' && 
                path[i-1] == '_') {
                    return true;
                }
        }
    }
    return false;
}

void encodeAtbash(char *s) {
    for (int i = 0; i < strlen(s); i++) {
        if ('A' <= s[i] && s[i] <= 'Z') s[i] = 'Z'-s[i]+'A';
        else if ('a' <= s[i] && s[i] <= 'z') s[i] = 'z'-s[i]+'a';
    }
}

void decodeAtbash(char *s) {
    for (int i = 0; s[i]; i++) {
        if ('A' <= s[i] && s[i] <= 'Z') s[i] = 'A'-s[i]+'Z';
        else if ('a' <= s[i] && s[i] <= 'z') s[i] = 'a'-s[i]+'z';
    }
}

void encodeROT13(char *s) {
    for (int i = 0; s[i]; i++) {
        if ('A' <= s[i] && s[i] <= 'Z') s[i] = ((s[i]-'A'+13)%26)+'A';
        else if ('a' <= s[i] && s[i] <= 'z') s[i] = ((s[i]-'a'+13)%26)+'a';
    }
}

void decodeROT13(char *s) {
    for (int i = 0; s[i]; i++) {
        if ('A' <= s[i] && s[i] <= 'Z') s[i] = ((s[i]-'A'-13)%26)+'A';
        else if ('a' <= s[i] && s[i] <= 'z') s[i] = ((s[i]-'a'-13)%26)+'a';
    }
}


void encodeVig(char *s) {
    char key[] = "SISOP";
    for (int i = 0; s[i]; i++) {
    	if ('A' <= s[i] && s[i] <= 'Z') s[i] = ((s[i]-'A'+(key[i%((sizeof(key)-1))]-'A'))%26)+'A';
        else if ('a' <= s[i] && s[i] <= 'z') s[i] = ((s[i]-'a'+(key[i%((sizeof(key)-1))]-'A'))%26)+'a';
    }
}

void decodeVig(char *s) {
    char key[] = "SISOP";
    for (int i = 0; s[i]; i++) {
    	if ('A' <= s[i] && s[i] <= 'Z') s[i] = ((s[i]-'A'-(key[i%((sizeof(key)-1))]-'A')+26)%26)+'A';
        else if ('a' <= s[i] && s[i] <= 'z') s[i] = ((s[i]-'a'-(key[i%((sizeof(key)-1))]-'A')+26)%26)+'a';
    }
}

void logInfo(char *command, char *desc) {
    time_t t = time(NULL);
    struct tm* lt = localtime(&t);

    char currTime[TIME_SIZE];
    strftime(currTime, TIME_SIZE, "%d%m%Y-%H:%M:%S", lt);

    char log[LOG_SIZE];
    sprintf(log, "%s::%s:%s::%s", warn, currTime, command, desc);

    FILE *out = fopen(logpath, "a");
    fprintf(out, "%s\n", log);
    fclose(out);

    return;
}


void logWarn(char *command, char *desc) {
    time_t t = time(NULL);
    struct tm* lt = localtime(&t);

    char currTime[TIME_SIZE];
    strftime(currTime, TIME_SIZE, "%d%m%Y-%H:%M:%S", lt);

    char log[LOG_SIZE];
    sprintf(log, "%s::%s:%s::%s", warn, currTime, command, desc);

    FILE *out = fopen(logpath, "a");
    fprintf(out, "%s\n", log);
    fclose(out);

    return;
}

void logEncode(char *dir1, char *dir2, char *cmd, int type) {
    char buff[1024];

    FILE *out = fopen(logpath, "a");
    if(cmd == "RENAME") {
        fprintf(out, "%s ??? %s\n", dir1, dir2);
    } else {
        fprintf(out, "%s\n", dir2);
    }
    fclose(out);

}

void logAll(char *dir1, char *dir2, char *cmd, int type) {
    char buff[1024];

    if(type == 1) {
        if (dir1[0] != '\0') { 
            sprintf(buff, "%s::%s", dir1, dir2);
            logInfo(cmd, buff);
        } else {
            sprintf(buff, "%s", dir2);
            logInfo(cmd, buff);
        }
    } else if(type == 2) {
        sprintf(buff, "%s", dir2);
        logInfo(cmd, buff);
    }
}

void getFileDetail(const char *completeFileName, char *name, char *ext) {
    int index = 0;
    int i = 0;
    while(completeFileName[i]) {
        if (completeFileName[i] == '.') {
            break;
        }
        name[index++] = completeFileName[i];
        i++;
    }
    name[index] = '\0';
    index = 0;
    while (completeFileName[i]) {
        ext[index++] = completeFileName[i];
        i++;
    }
    ext[index] = '\0';
}

int encodeFolderName(const char *basePath, const char* folderName) {
    char encryptedName[512];
    strcpy(encryptedName, folderName);
    encodeAtbash(encryptedName);
    char f_path[1024], t_path[1024];
    sprintf(f_path, "%s/%s", basePath, folderName);
    sprintf(t_path, "%s/%s", basePath, encryptedName);
    int res = rename(f_path, t_path);
    if (res == -1) return -errno;
    return 0;
}

int encodeFolderNameRXmk(const char *basePath, const char* folderName) {
    char encryptedName[512];
    strcpy(encryptedName, folderName);
    encodeAtbash(encryptedName);
    encodeROT13(encryptedName);
    char f_path[1024], t_path[1024];
    sprintf(f_path, "%s/%s", basePath, folderName);
    sprintf(t_path, "%s/%s", basePath, encryptedName);
    int res = rename(f_path, t_path);
    if (res == -1) return -errno;
    return 0;
}

int encodeFolderNameRXrn(const char *basePath, const char* folderName) {
    char encryptedName[512];
    strcpy(encryptedName, folderName);
    encodeAtbash(encryptedName);
    encodeVig(encryptedName);
    char f_path[1024], t_path[1024];
    sprintf(f_path, "%s/%s", basePath, folderName);
    sprintf(t_path, "%s/%s", basePath, encryptedName);
    int res = rename(f_path, t_path);
    if (res == -1) return -errno;
    return 0;
}

int decodeFolderName(const char *basePath, const char* folderName) {
    char decryptedName[512];
    strcpy(decryptedName, folderName);
    decodeAtbash(decryptedName);
    char f_path[1024], t_path[1024];
    sprintf(f_path, "%s/%s", basePath, folderName);
    sprintf(t_path, "%s/%s", basePath, decryptedName);
    int res = rename(f_path, t_path);
    if (res == -1) return -errno;
    return 0;
}

int decodeFolderNameRXmk(const char *basePath, const char* folderName) {
    char decryptedName[512];
    strcpy(decryptedName, folderName);
    decodeROT13(decryptedName);
    decodeAtbash(decryptedName);
    char f_path[1024], t_path[1024];
    sprintf(f_path, "%s/%s", basePath, folderName);
    sprintf(t_path, "%s/%s", basePath, decryptedName);
    int res = rename(f_path, t_path);
    if (res == -1) return -errno;
    return 0;
}

int decodeFolderNameRXrn(const char *basePath, const char* folderName) {
    char decryptedName[512];
    strcpy(decryptedName, folderName);
    decodeVig(decryptedName);
    decodeAtbash(decryptedName);
    char f_path[1024], t_path[1024];
    sprintf(f_path, "%s/%s", basePath, folderName);
    sprintf(t_path, "%s/%s", basePath, decryptedName);
    int res = rename(f_path, t_path);
    if (res == -1) return -errno;
    return 0;
}

int encodeFile(char *basePath, char *name) {
    char fileName[512], ext[64];
    getFileDetail(name, fileName, ext);
    encodeAtbash(fileName);
    char f_path[1024], t_path[1024];
    sprintf(f_path, "%s/%s", basePath, name);
    sprintf(t_path, "%s/%s%s", basePath, fileName, ext);
    int res = rename(f_path, t_path);
    if (res == -1) return -errno;
    return 0;
}

int encodeFileRXmk(char *basePath, char *name) {
    char fileName[1024], ext[64];
    getFileDetail(name, fileName, ext);
    encodeAtbash(fileName);
    encodeROT13(fileName);
    char f_path[1024], t_path[1024];
    sprintf(f_path, "%s/%s", basePath, name);
    sprintf(t_path, "%s/%s%s", basePath, fileName, ext);
    int res = rename(f_path, t_path);
    if (res == -1) return -errno;
    return 0;
}

int encodeFileRXrn(char *basePath, char *name) {
    char fileName[1024], ext[64];
    getFileDetail(name, fileName, ext);
    encodeAtbash(fileName);
    encodeVig(fileName);
    char f_path[1024], t_path[1024];
    sprintf(f_path, "%s/%s", basePath, name);
    sprintf(t_path, "%s/%s%s", basePath, fileName, ext);
    int res = rename(f_path, t_path);
    if (res == -1) return -errno;
    return 0;
}

int decodeFile(char *basePath, char *name) {
    char fileName[1024], ext[64];
    getFileDetail(name, fileName, ext);
    decodeAtbash(fileName);
    char f_path[1024], t_path[1024];
    sprintf(f_path, "%s/%s", basePath, name);
    sprintf(t_path, "%s/%s%s", basePath, fileName, ext);
    int res = rename(f_path, t_path);
    if (res == -1) return -errno;
    return 0;
}

int decodeFileRXmk(char *basePath, char *name) {
    char fileName[1024], ext[64];
    getFileDetail(name, fileName, ext);
    decodeROT13(fileName);
    decodeAtbash(fileName);
    char f_path[1024], t_path[1024];
    sprintf(f_path, "%s/%s", basePath, name);
    sprintf(t_path, "%s/%s%s", basePath, fileName, ext);
    int res = rename(f_path, t_path);
    if (res == -1) return -errno;
    return 0;
}

int decodeFileRXrn(char *basePath, char *name) {
    char fileName[1024], ext[64];
    getFileDetail(name, fileName, ext);
    decodeVig(fileName);
    decodeAtbash(fileName);
    char f_path[1024], t_path[1024];
    sprintf(f_path, "%s/%s", basePath, name);
    sprintf(t_path, "%s/%s%s", basePath, fileName, ext);
    int res = rename(f_path, t_path);
    if (res == -1) return -errno;
    return 0;
}

int encodeFolderRecursively(char *basePath, int depth) {
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir) return 0;
    int count = 0;
    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;
        strcpy(path, basePath);
        strcat(path, "/");
        strcat(path, dp->d_name);

        struct stat path_stat;
        stat(path, &path_stat);
        if (!S_ISREG(path_stat.st_mode)) {
            // Folder
            if (depth > 0) {
                count += encodeFolderRecursively(path, depth - 1);
                encodeFolderName(basePath, dp->d_name);
            }
        }
        else {
            // File
            if (encodeFile(basePath, dp->d_name) == 0) count++;
        }
    }
    closedir(dir);
    return count;
}

int encodeFolderRecursivelyRXmk(char *basePath) {
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir) return 0;
    int count = 0;
    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;
        strcpy(path, basePath);
        strcat(path, "/");
        strcat(path, dp->d_name);

        struct stat path_stat;
        stat(path, &path_stat);
        if (!S_ISREG(path_stat.st_mode)) {
            // Folder
            count += encodeFolderRecursivelyRXmk(path);
            encodeFolderNameRXmk(basePath, dp->d_name);
        }
        else {
            // File
            if (encodeFileRXmk(basePath, dp->d_name) == 0) count++;
        }
    }
    closedir(dir);
    return count;
}

int encodeFolderRecursivelyRXrn(char *basePath, int depth) {
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir) return 0;
    int count = 0;
    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;
        strcpy(path, basePath);
        strcat(path, "/");
        strcat(path, dp->d_name);

        struct stat path_stat;
        stat(path, &path_stat);
        if (!S_ISREG(path_stat.st_mode)) {
            // Folder
            if (depth > 0) {
                count += encodeFolderRecursivelyRXrn(path, depth - 1);
                encodeFolderNameRXrn(basePath, dp->d_name);
            }
        }
        else {
            // File
            if (encodeFileRXrn(basePath, dp->d_name) == 0) count++;
        }
    }
    closedir(dir);
    return count;
}

int decodeFolderRecursively(char *basePath, int depth) {
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir) return 0;
    int count = 0;
    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;
        strcpy(path, basePath);
        strcat(path, "/");
        strcat(path, dp->d_name);

        struct stat path_stat;
        stat(path, &path_stat);
        if (!S_ISREG(path_stat.st_mode)) {
            // Folder
            if (depth > 0) {
                count += decodeFolderRecursively(path, depth - 1);
                decodeFolderName(basePath, dp->d_name);
            }
        }
        else {
            // File
            if (decodeFile(basePath, dp->d_name) == 0) count++;
        }
    }
    closedir(dir);
    return count;
}

int decodeFolderRecursivelyRXmk(char *basePath) {
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir) return 0;
    int count = 0;
    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;
        strcpy(path, basePath);
        strcat(path, "/");
        strcat(path, dp->d_name);

        struct stat path_stat;
        stat(path, &path_stat);
        if (!S_ISREG(path_stat.st_mode)) {
            // Folder
            count += decodeFolderRecursivelyRXmk(path);
            decodeFolderNameRXmk(basePath, dp->d_name);
        }
        else {
            // File
            if (decodeFileRXmk(basePath, dp->d_name) == 0) count++;
        }
    }
    closedir(dir);
    return count;
}

int decodeFolderRecursivelyRXrn(char *basePath, int depth) {
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir) return 0;
    int count = 0;
    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;
        strcpy(path, basePath);
        strcat(path, "/");
        strcat(path, dp->d_name);

        struct stat path_stat;
        stat(path, &path_stat);
        if (!S_ISREG(path_stat.st_mode)) {
            // Folder
            if (depth > 0) {
                count += decodeFolderRecursivelyRXrn(path, INF);
                decodeFolderNameRXrn(basePath, dp->d_name);
            }
        }
        else {
            // File
            if (decodeFileRXrn(basePath, dp->d_name) == 0) count++;
        }
    }
    closedir(dir);
    return count;
}

void makeHiddenRX(int flag, char *path){
    char name[1024];
    sprintf(name, "%s/.map", path);
    FILE *fptr = fopen(name, "w");
    if(fptr == NULL) exit(EXIT_FAILURE);
    fprintf(fptr, "%d", flag);
    fclose(fptr);
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
    char fpath[1000];
    // Concat path
    sprintf(fpath, "%s%s", dirpath, path);
    res = lstat(fpath, stbuf);
    if (res == -1) return -errno;
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    if (strcmp(path, "/") == 0) {
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;

    dp = opendir(fpath);

    if (dp == NULL) return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;

        memset(&st, 0, sizeof(st));

        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        if(filler(buf, de->d_name, &st, 0)) break;
    }
    closedir(dp);
    return 0;
}

int getRXtype(char *path){
    char newpath[1024];
    strcpy(newpath, path);
    int len = strlen(path);
    for (int i = len-1; i >= 3 ; i--) {
        if (path[i-3] == 'R' && path[i-2] == 'X' && path[i-1] == '_'){
            for(int j = i; j<len; j++){
                if(newpath[j] == '/'){
                    newpath[j] = '\0';
                    break;
                }
            }
            strcat(newpath, "/.map");
            FILE *fptr = fopen(newpath, "r");
            char ch;
            fscanf(fptr, "%c", &ch);
            fclose(fptr);
            if(ch == '1')return 1;
            else return 0;
        }
    }
    return -1;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    if (strcmp(path, "/") == 0) {
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

    int fd;
    int res;
    (void) fi;

    fd = open(fpath, O_RDONLY);

    if (fd == -1) return -errno;

    res = pread(fd, buf, size, offset);

    if (res == -1) res = -errno;

    close(fd);

    return res;
}

static int xmp_rename(const char *from, const char *to, unsigned int flags) {
    int res;
    if (flags) return -EINVAL;
    char fpath[1000], tpath[1000];
    if (strcmp(from, "/") == 0) {
        from = dirpath;
        sprintf(fpath, "%s", from);
    }
    else sprintf(fpath, "%s%s", dirpath, from);
    if (strcmp(to, "/") == 0) {
        to = dirpath;
        sprintf(tpath, "%s", to);
    }
    else sprintf(tpath, "%s%s", dirpath, to);

    struct stat path_stat;
    stat(fpath, &path_stat);
    if (!S_ISREG(path_stat.st_mode)) {
        // Rename AtoZ to A_is_a_
        if (isAtoZ(fpath) && isAisA(tpath)) {
            decodeFolderRecursively(fpath, 0);
            #if defined DEBUG_MODE
            printf("[QAQ] Decoding %s with depth = 0.\n", fpath);
            #endif
        }
        // Rename A_is_a_ to AtoZ
        else if (isAisA(fpath) && isAtoZ(tpath)) {
            encodeFolderRecursively(fpath, 0);
            #if defined DEBUG_MODE
            printf("[QAQ] Encoding %s with depth = 0.\n", tpath);
            #endif
        }
        else if (isAtoZ(fpath) && !isAtoZ(tpath)) {
            #if defined DEBUG_MODE
            printf("[QAQ] Decoding %s.\n", fpath);
            #endif
            // logEncode(fpath, tpath, "RENAME", 1);
            int count = decodeFolderRecursively(fpath, INF);
            #if defined DEBUG_MODE
            printf("[QAQ] Total decoded file : %d\n", count);
            #endif
        }
        // Else if the folder is decoded and will be encoded
        else if (!isAtoZ(fpath) && isAtoZ(tpath)) {
            #if defined DEBUG_MODE
            printf("[QAQ] Encoding %s.\n", fpath);
            #endif
            logEncode(fpath, tpath, "RENAME", 1);
            int count = encodeFolderRecursively(fpath, INF);
            #if defined DEBUG_MODE
            printf("[QAQ] Total encoded file : %d\n", count);
            #endif
        }
        // Rename RX_ to A_is_a_
        else if (isRX(fpath) && isAisA(tpath)) {
            decodeFolderRecursivelyRXrn(fpath, 0);
        }
        // Rename A_is_a to RX_
        else if (isAisA(fpath) && isRX(tpath)) {
            encodeFolderRecursivelyRXrn(fpath, 0);
        }else if(!isRX(fpath) && isRX(tpath)){
            encodeFolderRecursivelyRXrn(fpath, INF);
        }else if(isRX(fpath) && !isRX(tpath)){
            decodeFolderRecursivelyRXrn(fpath, INF);
        }
    }

    logAll(fpath, tpath, "RENAME", 1);

    res = rename(fpath, tpath);
    if (res == -1) return -errno;
    return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
    char fpath[1000];
    if (strcmp(path, "/") == 0) {
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

    if(getRXtype(fpath)){
        
    }
	int res;

	if (S_ISREG(mode)) {
		res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(fpath, mode);
	else
		res = mknod(fpath, mode, rdev);
	if (res == -1)
		return -errno;

    logAll("", fpath, "CREATE", 1);
    

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode) {
    int res;
    char fpath[1000];
    if (strcmp(path, "/") == 0) {
        path = dirpath;
        sprintf(fpath, "%s", path);
    }
    else sprintf(fpath, "%s%s", dirpath, path);

    res = mkdir(fpath, mode);
    if (res == -1) return -errno;

    if (isAtoZ(fpath)){
        logEncode("", fpath, "MKDIR", 1);
    }

    else if(isRX(fpath)){
        makeHiddenRX(1, fpath);
        
    }

    logAll("", fpath, "MKDIR", 1);
    
    return 0;
}

static int xmp_unlink(const char *path) { //ngehapus file
	
	char newPath[1000];
	if(strcmp(path,"/") == 0){
		sprintf(newPath,"%s",path);
	} else sprintf(newPath, "%s%s",dirpath,path);


    logAll("",path,"REMOVE", 2);
	
    int res;
	res = unlink(newPath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path) {//ngehapus directory
	
	char newPath[1000];
	sprintf(newPath, "%s%s",dirpath,path);
    // char str[100];
	// sprintf(str, "RMDIR::%s", path);
	// logging(str,2);

    logAll("",path,"RMDIR", 2);

	int result;
	result = rmdir(newPath);
	if (result == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi){ //open file
	char newPath[1000];
	sprintf(newPath, "%s%s", dirpath, path);
	int result;
	result = open(newPath, fi->flags);
	if (result == -1)
		return -errno;
	close(result);
	return 0;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) { //write file
	
	char newPath[1000];
	sprintf(newPath, "%s%s", dirpath, path);
	
    int fd;
	int result;
	(void) fi;
	fd = open(newPath, O_WRONLY);
	if (fd == -1)
		return -errno;
    // char str[100];
	// sprintf(str, "WRITE::%s", path);
	// logging(str,1);

    logAll("", path, "WRITE", 1);

	result = pwrite(fd, buf, size, offset);
	if (result == -1)
		result = -errno;
	close(fd);
	return result;
}

static const struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .read = xmp_read,
    .mkdir = xmp_mkdir,
    .rename = xmp_rename,
    .mknod = xmp_mknod,

	.unlink = xmp_unlink,
	.rmdir = xmp_rmdir,
	.open = xmp_open,
	.write = xmp_write,
};

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
