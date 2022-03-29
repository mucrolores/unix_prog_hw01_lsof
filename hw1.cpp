#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
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

void m_strcpy(char* &dst, const char* src){
    
    dst=(char*)calloc(strlen(src)+1, sizeof(char));
    strcpy(dst, src);
}

const char* getUserName(uid_t uid){
    struct passwd *user;
    user = getpwuid(uid);
    return user->pw_name;
}

char* trimString(char* str){
    int non_space_count = 0;
    for(int i=0;i<strlen(str);i++){
        if(str[i] != ' ' && str[i] != '\n' && str[i] != '\t'){
            non_space_count += 1;
        }
    }
    if (non_space_count == 0){
        return NULL;
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
        void print();
        char* command;
        char* pid;
        char* user;
        char* fd;
        char* type;
        char* inode;
        char* name;
};

void print_of_info_vector(std::vector<ofInfo> of_info_vector, char* command_regex, char* type_regex, char* name_regex){
    int max_command = 0;
    int max_pid = 0;
    int max_user = 0;
    int max_fd = 0;
    int max_type = 0;
    int max_inode = 0;
    for(int i=0;i<of_info_vector.size();i++){
        max_command = std::max(max_command, (int)strlen(of_info_vector[i].command));
        max_pid = std::max(max_pid, (int)strlen(of_info_vector[i].pid));
        max_user = std::max(max_user, (int)strlen(of_info_vector[i].user));
        max_fd = std::max(max_fd, (int)strlen(of_info_vector[i].fd));
        max_type = std::max(max_type, (int)strlen(of_info_vector[i].type));
        max_inode = std::max(max_inode, (int)strlen(of_info_vector[i].inode));
    }

    std::cout << of_info_vector[0].command;
    for (int j=0;j<max_command-strlen(of_info_vector[0].command);j++){
        std::cout << " ";
    }
    std::cout << "    ";
    std::cout << of_info_vector[0].pid;
    for (int j=0;j<max_pid-strlen(of_info_vector[0].pid);j++){
        std::cout << " ";
    }
    std::cout << "    ";
    std::cout << of_info_vector[0].user;
    for (int j=0;j<max_user-strlen(of_info_vector[0].user);j++){
        std::cout << " ";
    }
    std::cout << "    ";
    std::cout << of_info_vector[0].fd;
    for (int j=0;j<max_fd-strlen(of_info_vector[0].fd);j++){
        std::cout << " ";
    }
    std::cout << "    ";
    std::cout << of_info_vector[0].type;
    for (int j=0;j<max_type-strlen(of_info_vector[0].type);j++){
        std::cout << " ";
    }
    std::cout << "    ";
    std::cout << of_info_vector[0].inode;
    for (int j=0;j<max_inode-strlen(of_info_vector[0].inode);j++){
        std::cout << " ";
    }
    std::cout << "    ";
    std::cout << of_info_vector[0].name << std::endl;

    for(int i=1;i<of_info_vector.size();i++){
        if(command_regex != NULL){
            std::regex c_regex(command_regex);
            if(!std::regex_search(of_info_vector[i].command, c_regex)){
                continue;
            }
        }
        if(type_regex != NULL){
            std::regex t_regex(type_regex);
            if(!std::regex_search(of_info_vector[i].type, t_regex)){
                continue;
            }
        }
        if(name_regex != NULL){
            std::regex n_regex(name_regex);
            if(!std::regex_search(of_info_vector[i].name, n_regex)){
                continue;
            }
        }

        std::cout << of_info_vector[i].command;
        for (int j=0;j<max_command-strlen(of_info_vector[i].command);j++){
            std::cout << " ";
        }
        std::cout << "    ";
        std::cout << of_info_vector[i].pid;
        for (int j=0;j<max_pid-strlen(of_info_vector[i].pid);j++){
            std::cout << " ";
        }
        std::cout << "    ";
        std::cout << of_info_vector[i].user;
        for (int j=0;j<max_user-strlen(of_info_vector[i].user);j++){
            std::cout << " ";
        }
        std::cout << "    ";
        std::cout << of_info_vector[i].fd;
        for (int j=0;j<max_fd-strlen(of_info_vector[i].fd);j++){
            std::cout << " ";
        }
        std::cout << "    ";
        std::cout << of_info_vector[i].type;
        for (int j=0;j<max_type-strlen(of_info_vector[i].type);j++){
            std::cout << " ";
        }
        std::cout << "    ";
        std::cout << of_info_vector[i].inode;
        for (int j=0;j<max_inode-strlen(of_info_vector[i].inode);j++){
            std::cout << " ";
        }
        std::cout << "    ";
        std::cout << of_info_vector[i].name << std::endl;
    }
}

void traversePid(const char *pid, std::vector<ofInfo> &of_vector){
    // try to acquire information such as command, uid, username
    const char *proc_root = "/proc/";
    char* pid_path = concatString(proc_root, pid);
    char* status_path = concatString(pid_path, "/status");

    std::vector<ofInfo> pid_of_vector;

    FILE *fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    
    const char* command = NULL;
    char* uid = (char*)calloc(100, sizeof(char));
    const char* user;

    // get the status from current process
    fp = fopen(status_path, "r");
    if(fp == NULL){
        return;
    }
    while((read = getline(&line, &len, fp)) != -1)
    {
        char* savePtr;
        char* title = strtok_r(line,": \t",&savePtr);
        if(strcmp(title, "Name") == 0){
            command = trimString(savePtr);
        }
        if(strcmp(title, "Uid") == 0){
            m_strcpy(uid,strtok_r(NULL," \t", &savePtr));
        }
    }
    fclose(fp);
    user = getUserName((uid_t)atoi(uid));

    DIR *p_d;
    struct dirent *p_dir;
    // handling the special files: cwd, rtd, txt, mem
    char **special_files = (char**)calloc(4, sizeof(char*));
    special_files[0] = concatString("cwd","");
    special_files[1] = concatString("root","");
    special_files[2] = concatString("exe","");
    special_files[3] = concatString("fd","");

    char **tag_fd = (char**)calloc(4, sizeof(char*));
    tag_fd[0] = concatString("cwd","");
    tag_fd[1] = concatString("rtd","");
    tag_fd[2] = concatString("txt","");
    tag_fd[3] = concatString("fd","");

    for(int i=0;i<4;i++){
        ofInfo of_info;
        m_strcpy(of_info.command,command);
        m_strcpy(of_info.pid,pid);
        m_strcpy(of_info.user,user);
        m_strcpy(of_info.name, concatString(concatString(pid_path,"/"), special_files[i]));
        m_strcpy(of_info.fd,tag_fd[i]);
        char *link_path = concatString(concatString(pid_path,"/"), special_files[i]);
        if (access(link_path, R_OK) != 0){
            m_strcpy(of_info.type,"unknown");
            m_strcpy(of_info.name,concatString(link_path, " (Permission denied)"));
            if(i == 3){
                m_strcpy(of_info.fd,"NOFD");
            }
        }
        else{
            char *real_path = (char*)calloc(PATH_MAX, sizeof(char));
            int len = readlink(link_path, real_path, PATH_MAX);
            if(len == -1){
                continue;
            }
            else{
                struct stat *buf = (struct stat*)malloc(sizeof(struct stat));
                stat(real_path, buf);
                char* inode_arr = (char*)calloc(10,sizeof(char));
                sprintf(inode_arr,"%ld",buf->st_ino);
                switch(buf->st_mode & S_IFMT){
                    case S_IFREG:
                        m_strcpy(of_info.type,"REG");
                        break;
                    case S_IFDIR:
                        m_strcpy(of_info.type,"DIR");
                        break;
                    case S_IFBLK:
                        m_strcpy(of_info.type,"BLK");
                        break;
                    case S_IFCHR:
                        m_strcpy(of_info.type,"CHR");
                        break;
                    case S_IFIFO:
                        m_strcpy(of_info.type,"FIFO");
                        break;
                    case S_IFSOCK:
                        m_strcpy(of_info.type,"SOCK");
                        break;
                    default:
                        m_strcpy(of_info.type,"unknown");
                        break;
                }
                m_strcpy(of_info.inode,inode_arr);
                m_strcpy(of_info.name,real_path);
            }
        }
        pid_of_vector.push_back(of_info);
    }
    
    
    // handling the map files
    char* maps_path = concatString(pid_path, "/maps");
    if (access(maps_path, R_OK) == 0)
    {
        fp = fopen(maps_path, "r");
        if(fp != NULL)
        {
            while((read = getline(&line, &len, fp)) != -1)
            {
                ofInfo of_info;
                m_strcpy(of_info.command,command);
                m_strcpy(of_info.pid,pid);
                m_strcpy(of_info.user,user);
                strtok(line, "\n");
                char* savePtr;
                
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
                
                if(strcmp(inode, "0") == 0 || pathname == NULL){
                    continue;
                }
                bool exist_flag = false;
                for(int i=0;i<pid_of_vector.size();i++){
                    if(strcmp(pid_of_vector[i].inode, inode) == 0){
                        exist_flag = true;
                        break;
                    }
                    if(strcmp(pid_of_vector[i].name, pathname) == 0){
                        exist_flag = true;
                        break;
                    }
                }
                if(exist_flag){
                    continue;
                }

                if(deleted == NULL){
                    m_strcpy(of_info.fd,"mem");
                }
                else{
                    m_strcpy(of_info.fd,"DEL");
                }
                m_strcpy(of_info.type,"REG");
                m_strcpy(of_info.inode,concatString(inode,"\0"));
                m_strcpy(of_info.name,concatString(pathname,"\0"));
                pid_of_vector.push_back(of_info);
            }

            fclose(fp);
        }
    }

    // handling the fd files
    DIR *fd_d;
    struct dirent *fd_dir;

    char* fd_path = concatString(pid_path, "/fd");
    if (access(fd_path, R_OK) == 0)
    {
        fd_d = opendir(fd_path);
        while((fd_dir = readdir(fd_d)) != NULL) {
            ofInfo of_info;
            m_strcpy(of_info.command,command);
            m_strcpy(of_info.pid,pid);
            m_strcpy(of_info.user,user);
            m_strcpy(of_info.fd, fd_dir->d_name);
            char *real_path = (char*)calloc(PATH_MAX, sizeof(char));
            char *link_path = concatString(fd_path,concatString("/", fd_dir->d_name));
            int len = readlink(link_path, real_path, PATH_MAX);
            if(len == -1){
                continue;
            }
            else{
                struct stat *lbuf = (struct stat*)malloc(sizeof(struct stat));
                lstat(link_path, lbuf);
                int flag = (lbuf->st_mode >> 7) & 3;
                if(flag == 1){
                    of_info.fd = concatString(of_info.fd, "w"); 
                }
                else if(flag == 2){
                    of_info.fd = concatString(of_info.fd, "r");
                }
                else if(flag == 3){
                    of_info.fd = concatString(of_info.fd, "u"); 
                }
                char* save_ptr;
                m_strcpy(of_info.name,strtok_r(real_path," ",&save_ptr));
                free(lbuf);
                
                
                struct stat *sbuf = (struct stat*)malloc(sizeof(struct stat));
                stat(link_path, sbuf);
                switch(sbuf->st_mode & S_IFMT){
                    case S_IFREG:
                        m_strcpy(of_info.type,"REG");
                        break;
                    case S_IFDIR:
                        m_strcpy(of_info.type,"DIR");
                        break;
                    case S_IFBLK:
                        m_strcpy(of_info.type,"BLK");
                        break;
                    case S_IFCHR:
                        m_strcpy(of_info.type,"CHR");
                        break;
                    case S_IFIFO:
                        m_strcpy(of_info.type,"FIFO");
                        break;
                    case S_IFSOCK:
                        m_strcpy(of_info.type,"SOCK");
                        break;
                    default:
                        m_strcpy(of_info.type,"unknown");
                        break;
                }
                char* inode_arr = (char*)calloc(10,sizeof(char));
                sprintf(inode_arr,"%ld",sbuf->st_ino);
                m_strcpy(of_info.inode,inode_arr);
                pid_of_vector.push_back(of_info);
            }
        }
        closedir(fd_d);
    }

    for(int i=0;i<pid_of_vector.size();i++){
        of_vector.push_back(pid_of_vector[i]);
    }
}

int main(int argc, char*argv[] ){
    char *command_regex_str = NULL;
    char *type_regex_str = NULL;
    char *name_regex_str = NULL;
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i], "-c") == 0){
            if(i+1<argc){
                command_regex_str = argv[i+1];
            }
        }
        if(strcmp(argv[i], "-t") == 0){
            if(i+1<argc){
                type_regex_str = argv[i+1];
            }
        }
        if(strcmp(argv[i], "-f") == 0){
            if(i+1<argc){
                name_regex_str = argv[i+1];
            }
        }
    }

    if(type_regex_str!=NULL){
        if(strcmp(type_regex_str,"REG") != 0 && strcmp(type_regex_str,"CHR") != 0 && strcmp(type_regex_str,"DIR") != 0 && strcmp(type_regex_str,"FIFO") != 0 && 
        strcmp(type_regex_str,"SOCK") != 0 && strcmp(type_regex_str,"unknown") != 0){
            std::cout << "Invalid TYPE option." << std::endl;
            return 0;
        }
    }

    std::vector<const char*> proc_vector;

    DIR *d;
    struct dirent *dir;
    std::regex pidRegex("^[0-9]*$");

    d = opendir("/proc");
    while((dir = readdir(d)) != NULL) {
        const char* dir_name = concatString(dir->d_name, "\0");
        if(std::regex_search(dir_name, pidRegex)){
            proc_vector.push_back(dir_name);
        }
    }
    closedir(d);

    std::vector<ofInfo> of_vector;
    ofInfo head_of_info;
    m_strcpy(head_of_info.command, "COMMAND");
    m_strcpy(head_of_info.pid, "PID");
    m_strcpy(head_of_info.user, "USER");
    m_strcpy(head_of_info.fd, "FD");
    m_strcpy(head_of_info.type, "TYPE");
    m_strcpy(head_of_info.inode, "NODE");
    m_strcpy(head_of_info.name, "NAME");
    of_vector.push_back(head_of_info);

    for(int i=0;i<proc_vector.size();i++){
        traversePid(proc_vector[i], of_vector);
    }

    print_of_info_vector(of_vector, command_regex_str, type_regex_str, name_regex_str);
    return 0;
}