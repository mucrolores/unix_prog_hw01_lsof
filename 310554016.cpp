#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>
#include <regex>

char* concatString(const char* c1,const char* c2){
    char* result = (char*)calloc(strlen(c1) + strlen(c2) + 1, sizeof(char));
    int index=0;
    for(int i=0;i<strlen(c1);i++){
        result[index] = c1[i];
        index += 1;
    }
    for(int i=0;i<strlen(c2);i++){
        result[index] = c2[i];
        index += 1;
    }
    return result;
}

const char* getUserName(uid_t uid){
    struct passwd *user;
    user = getpwuid(uid);
    return user->pw_name;
}

const char* trimString(const char* str){
    int non_space_count = 0;
    for(int i=0;i<strlen(str);i++){
        if(str[i] != ' ' && str[i] != '\n' && str[i] != '\t'){
            non_space_count += 1;
        }
    }
    char* result = (char*)calloc(non_space_count+1, sizeof(char));
    int ptr = 0;
    for(int i=0;i<strlen(str);i++){
        if(str[i] != ' ' && str[i] != '\n' && str[i] != '\t'){
            result[ptr] = str[i];
            ptr += 1;
        }
    }
    return result;
}

class ofInfo{
    public:
        ofInfo();
        void print();
        char* command;
        char* pid;
        char* user;
        char* fd;
        char* type;
        char* node;
        char* name;
};

ofInfo::ofInfo(){
    command = (char*)calloc(2, sizeof(char));
    command[0] = '\t';
    command[1] = '\0';
    pid = (char*)calloc(2, sizeof(char));
    pid[0] = '\t';
    pid[1] = '\0';
    user = (char*)calloc(2, sizeof(char));
    user[0] = '\t';
    user[1] = '\0';
    fd = (char*)calloc(2, sizeof(char));
    fd[0] = '\t';
    fd[1] = '\0';
    type = (char*)calloc(2, sizeof(char));
    type[0] = '\t';
    type[1] = '\0';
    node = (char*)calloc(2, sizeof(char));
    node[0] = '\t';
    node[1] = '\0';
    name = (char*)calloc(2, sizeof(char));
    name[0] = '\t';
    name[1] = '\0';
}

void ofInfo::print(){
    std::cout << command << "\t" << pid << "\t" << user << "\t" << fd << "\t" << type << "\t" << node << "\t" << name << std::endl;
}

void traversePid(const char *pid, std::vector<ofInfo> &of_vector){
    // try to acquire information such as command, uid, username
    const char *proc_root = "/proc/";
    char* pid_path = concatString(proc_root, pid);
    char* status_path = concatString(pid_path, "/status");

    FILE *fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    
    const char* command;
    char* uid;
    const char* user;

    // get the status from current process
    fp = fopen(status_path, "r");
    if(fp == NULL){
        std::cout << "cannot open pid command\n";
        return;
    }
    while((read = getline(&line, &len, fp)) != -1)
    {
        strtok(line, "\n");
        char* savePtr;
        strtok_r(line,":",&savePtr);
        if(strcmp(line, "Name") == 0){
            command = trimString(savePtr);
        }
        if(strcmp(line, "Uid") == 0){
            strtok(savePtr," ");
            uid = (char*)calloc(strlen(line), sizeof(char));
            strcpy(uid,line);
        }
    }
    fclose(fp);
    user = getUserName((uid_t)atoi(uid));

    DIR *p_d;
    struct dirent *p_dir;
    // handling the special files: cwd, rtd, txt, mem
    p_d = opendir(pid_path);
    while((p_dir = readdir(p_d)) != NULL){
        ofInfo of_info;
        strcpy(of_info.command,command);
        strcpy(of_info.pid,pid);
        strcpy(of_info.user,user);
        strcpy(of_info.name, concatString(concatString(pid_path,"/"), p_dir->d_name));
        if(strcmp(p_dir->d_name, "cwd") == 0 || strcmp(p_dir->d_name, "root") == 0 || strcmp(p_dir->d_name, "exe") == 0){
            if(strcmp(p_dir->d_name, "cwd") == 0){
                strcpy(of_info.fd,"cwd");
            }
            else if(strcmp(p_dir->d_name, "root") == 0){
                strcpy(of_info.fd,"trd");
            }
            else if(strcmp(p_dir->d_name, "exe") == 0){
                strcpy(of_info.fd,"txt");
            }
            char *link_path = concatString(concatString(pid_path,"/"), p_dir->d_name);
            if (access(link_path, R_OK) != 0){
                strcpy(of_info.type,"unknown");
                strcpy(of_info.name,concatString(link_path, " (Permission denied)"));
            }
            else{
                char *real_path = (char*)calloc(PATH_MAX, sizeof(char));
                std::cout << link_path << std::endl;
                int len = readlink(link_path, real_path, PATH_MAX);
                if(len == -1){
                    continue;
                }
                else{
                    // std::cout << "target_path" << real_path << std::endl;
                    struct stat *buf = (struct stat*)malloc(sizeof(struct stat));
                    stat(real_path, buf);
                    char* inode_arr = (char*)calloc(10,sizeof(char));
                    sprintf(inode_arr,"%ld",buf->st_ino);
                    strcpy(of_info.type,"DIR");
                    strcpy(of_info.node,inode_arr);
                    strcpy(of_info.name,concatString(link_path, real_path));
                }
            }
            of_vector.push_back(of_info);
        }
    }
    closedir(p_d);
    // handling the map files

    char* maps_path = concatString(pid_path, "/maps");
    if (access(maps_path, R_OK) == 0)
    {
        fp = fopen(maps_path, "r");
        if(fp == NULL){
            std::cout << "cannot open maps command\n";
            return;
        }
        while((read = getline(&line, &len, fp)) != -1)
        {
            ofInfo of_info;
            strcpy(of_info.command,command);
            strcpy(of_info.pid,pid);
            strcpy(of_info.user,user);
            strtok(line, "\n");
            char* savePtr;
            
            std::cout << "line: " << line << std::endl;
            
            char* memory_part = NULL;
            char* permission = NULL;
            char* offset = NULL;
            char* dev = NULL;
            char* inode = NULL;
            char* pathname = NULL;
            char* deleted = NULL;

            memory_part = strtok_r(line," ",&savePtr);
            permission = strtok_r(NULL," ",&savePtr);
            offset = strtok_r(NULL," ",&savePtr);
            dev = strtok_r(NULL," ",&savePtr);
            inode = strtok_r(NULL," ",&savePtr);
            pathname = strtok_r(NULL," ",&savePtr);
            if(strcmp(savePtr, "") == 0){
                deleted = NULL;
            }
            else{
                deleted = savePtr;
            }
            if(pathname!=NULL)
            {
                std::cout << "pathname" << pathname << std::endl;
                std::cout << "savePtr" << savePtr << std::endl;
            }
            
            if(strcmp(inode, "0") == 0 || pathname == NULL){
                std::cout << "inode 0 or pathname NULL" << std::endl;
                continue;
            }
            bool exist_flag = false;
            for(int i=0;i<of_vector.size();i++){
                if(strcmp(of_vector[i].node, inode) == 0){
                    exist_flag = true;
                    break;
                }
                if(strcmp(of_vector[i].name, pathname) == 0){
                    exist_flag = true;
                    break;
                }
            }
            if(exist_flag){
                continue;
            }

            if(deleted == NULL){
                strcpy(of_info.fd,"mem");
            }
            else{
                strcpy(of_info.fd,"DEL");
            }
            strcpy(of_info.type,"REG");
            strcpy(of_info.node,concatString(inode,"\0"));
            strcpy(of_info.name,concatString(pathname,"\0"));
            // of_info.print();
            of_vector.push_back(of_info);
        }
        fclose(fp);
    }
    

    

}

int main(){
    std::vector<const char*> proc_vector;

    DIR *d;
    struct dirent *dir;
    std::regex pidRegex("^[0-9]*$");
    std::smatch m;

    d = opendir("/proc");
    while ((dir = readdir(d)) != NULL) {
        const char* dir_name = concatString(dir->d_name, "\0");
        if(std::regex_search(dir_name, pidRegex)){
            proc_vector.push_back(dir_name);
        }
    }
    closedir(d);

    std::vector<ofInfo> of_vector;

    for(int i=0;i<proc_vector.size();i++){
        if(strcmp(proc_vector[i],"121\0") == 0){
            traversePid(proc_vector[i], of_vector);
        }
    }

    std::cout << "COMMAND\tPID\tUSER\tFD\tTYPE\tNODE\tNAME\n";
    for(int i=0;i<of_vector.size();i++){
        of_vector[i].print();
    }
    return 0;
}

