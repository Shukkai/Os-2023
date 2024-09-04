# include<iostream>
# include<string.h>
# include<vector>
# include<unistd.h>
# include<sys/wait.h>
# include<cstdlib>
# include<sys/types.h>
# include<fcntl.h>
using namespace std;

void split_cmd(char *cmd, char **argv, bool &zombie){
    char *tmp = strtok(cmd," ");
    int i = 0;
    while(tmp != NULL){
        // cout<<tmp<<endl;
        argv[i] = tmp;
        tmp = strtok(NULL," ");
        i++;
    }
    if(strcmp(argv[i-1],"&") == 0){
        argv[i-1] = NULL;
        zombie = true;
    }
    else{
        argv[i] = NULL;
    }   
}

void split_diract(char *in, char *f_cmd, char *b_cmd, char *delimeter){
    char *tmp = strtok(in, delimeter);
    strcpy(f_cmd,tmp);
    tmp = strtok(NULL, delimeter);
    strcpy(b_cmd, tmp);
}

int pipe_check(char *in){
    int ch1 = '<', ch2 = '>', ch3 = '|';
    if(strcmp(in,"exit") == 0){
        return 0;
    }
    else if(strchr(in, ch1) != NULL){
        return 2;
    }
    else if(strchr(in, ch2) != NULL){
        return 3;
    }
    else if(strchr(in, ch3) != NULL){
        return 4;
    }
    else{
        return 1;
    }
} 

void execute_zombie(char **argv, bool zombie){
    pid_t pid;
    pid = fork();
    if(zombie){
        if(pid < 0){
            fprintf(stderr, "fork failed");
            exit(-1);
        }
        else if(pid == 0){
            pid = fork();
            if(pid < 0){
                fprintf(stderr, "fork failed");
                exit(-1);
            }
            else if(pid == 0){
                execvp(argv[0],argv);
                exit(0);
            }
            else{
                exit(0);
            }
        }
        else{
            wait(NULL);
        }
    }
    else{
        if(pid < 0){
            fprintf(stderr, "fork failed");
            exit(-1);
        }
        else if(pid == 0){
            execvp(argv[0],argv);
            exit(0);
        }
        else{
            wait(NULL);
        }
    }
}

void rediract(char **f_argv, char **b_argv, char *delimeter){
    pid_t pid;
    pid = fork();
    if(pid < 0){
        fprintf(stderr, "fork failed");
        exit(-1);
    }
    else if(pid == 0){
        if(strcmp(delimeter,">") == 0){
            int fd = open(b_argv[0],O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
            dup2(fd,STDOUT_FILENO);
            execvp(f_argv[0],f_argv);
            exit(0);
        }
        else if(strcmp(delimeter,"<") == 0){
            int fd = open(b_argv[0], O_RDONLY,S_IRUSR|S_IWUSR);
            dup2(fd,STDIN_FILENO);
            execvp(f_argv[0],f_argv);
            exit(0);
        }
    }
    else{
        wait(NULL);
    }
}

void run_piping(char **f_agrv, char **b_argv){
    pid_t pip1,pip2;
    int pfd[2];
    if(pipe(pfd) == -1){
        fprintf(stderr, "fork failed");
        exit(-1);
    }
    pip1 = fork();
    if(pip1 < 0){
        fprintf(stderr, "fork failed");
        exit(-1);
    }
    else if(pip1 == 0){
        close(pfd[0]);
        dup2(pfd[1],STDOUT_FILENO);
        execvp(f_agrv[0],f_agrv);
        exit(0);
    }
    if(pip2 < 0){
        fprintf(stderr, "fork failed");
        exit(-1);
    }
    pip2 = fork();
    if(pip2 == 0){
        close(pfd[1]);
        dup2(pfd[0],STDIN_FILENO);
        execvp(b_argv[0],b_argv);
        exit(0);
    }
    close(pfd[0]);
    close(pfd[1]);
    wait(NULL);
    wait(NULL);

}

int main(){
    char *cmd = (char*)malloc(100*sizeof(char));
    cout<<"> ";
    while(true){
        fgets(cmd,100,stdin);
        cmd = strtok(cmd,"\n");
        bool zombie = false; 
        char **f_argv = (char**)malloc(100*sizeof((char*)malloc(100*sizeof(char)))), **b_argv = (char**)malloc(100*sizeof((char*)malloc(100*sizeof(char))));      
        int status = pipe_check(cmd);
        if(status == 0){
            break;
        }
        else if(status == 1){
            split_cmd(cmd,f_argv,zombie);
            execute_zombie(f_argv,zombie);
        }
        else{
            char *f_cmd = (char*)malloc(100*sizeof(char)), *b_cmd = (char*)malloc(100*sizeof(char)), *delimeter = (char*)malloc(100*sizeof(char));
            if(status == 2){
                *delimeter = '<';
            }
            else if(status == 3){
                *delimeter = '>';
            }
            else if(status == 4){
                *delimeter = '|';
            }
            split_diract(cmd,f_cmd,b_cmd,delimeter);
            split_cmd(f_cmd,f_argv,zombie);
            split_cmd(b_cmd,b_argv,zombie);
            if(status == 2 || status == 3){
                rediract(f_argv,b_argv,delimeter);
            }
            else{
                run_piping(f_argv,b_argv);
            }
            
        }
        cout<<"\n"<<"> ";
    }
    return 0;
}