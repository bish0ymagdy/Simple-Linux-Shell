#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define  SIZE       50
#define  S_SIZE     20
#define  file_mode  "a"

// variables initialization
char input[SIZE];                       // input command
char wd[SIZE] = "/home";                // working directory
char *command[SIZE];                    // parsed  command line arguments
char *sub_command[S_SIZE];              // sub-commands for the case of "&"
char var[SIZE][S_SIZE];                 // variables for export
char val[SIZE][S_SIZE];                 // values for export
int counter = 0;                        // coounter for the number of variables for the export
int back_flag = 0;                      //  flag if background process is needed
pid_t pid;                              // child process ID
pid_t pid_back;                         // background process ID
char file_path[SIZE];                   // file path to the log file of the terminated child process
char directory[SIZE];                   // the directory of the code
const char *file_name = "/log.txt";     // name of the log file
int error_flag = 1;                     // flag to print the error messege

// Function Prototypes
void register_child_signal();
void setup_environment();
void on_child_exit();
void reap_child_zombie();
void create_log_file();
void write_to_log_file(pid_t id);
void get_input();
void split(char* s[SIZE], char *y, int flag);
void exe_cd();
void exe_export();
void exe_echo();
void var_split();
void exe_command();
void execute();
void shell();

void register_child_signal(){

    signal(SIGCHLD, on_child_exit);

}

void on_child_exit(){

    reap_child_zombie();

}

void reap_child_zombie(){

    pid_t terminated_pid = waitpid(0, NULL, WNOHANG);

    if (terminated_pid > 0) {

        // Child process has terminated
        write_to_log_file(terminated_pid);

    }

}

void create_log_file(){ // create the log file
    
    // create the log file path
    getcwd(directory, SIZE);
    snprintf(file_path, sizeof(file_path), "%s%s", directory, file_name);

}

void write_to_log_file(pid_t id){ // write in the file when a child process is terminated

    FILE *f = fopen(file_path, file_mode);

    if (f != NULL) {

        fprintf(f,"%d process is terminated\n", id);
        fclose(f);

    }
    else{

        printf("Error opening the file\n");

    }

}

void setup_environment(){ // set the enviroment to the home

    for (int i=0;i<SIZE;i++){

        printf("*");

    }
    printf("\n\n\t    Welcome to bishoy's Shell!\n\n");
    for (int i=0;i<SIZE;i++){

        printf("*");

    }
    printf("\n\n");
    chdir(getcwd(wd, SIZE));

}

void get_input(){ // get input from user

    printf("myshell@shell:%s$ ", getcwd(wd, SIZE));
    fgets(input, SIZE, stdin);
    if (input[0] != '\n'){ // to not quit if the user pressed enter without typing anything
        input[strcspn(input, "\n")] = '\0';
        error_flag = 1;
    }
    else {
        error_flag = 0;
    }

}

void split(char* s[SIZE], char *y, int flag){     // parse input string and search for special characters

    char *token = s[0];
    int i;

    for(i = 1; token != NULL && i < SIZE; i++){

        token = strtok(NULL, y);
        s[i] = token;

    }

    s[i] = NULL; // add a termination character at the end of array

    for(int j = 0;  s[j] != NULL && flag; j++){ // search for the existance of special characters "&" and "$"

        if (strcmp(s[j], "&") == 0){

            back_flag = 1;

        }
      
        if(s[j][0] == '$'){

            if(counter == 0){

                break;

            }
            char temp[S_SIZE];
            for(int z = 0; s[j][z] != '\0'; z++){
                temp[z] = s[j][z+1];
            }
            for(int k = 0; k < counter; k++){

                if(strcmp(var[k], temp) == 0){

                    strcpy(s[j], val[k]);
                    break;

                }

            }

        }  
    }

    if (i >= SIZE) {

        fprintf(stderr, "Too many tokens. Increase SIZE.\n");
        exit(EXIT_FAILURE);

    }

    // print the command for debugging
    // for(i = 0; command[i]!= NULL; i++) {
    //     printf("%s\n", command[i]);
    // }
}

void exe_cd(){ // cd command execute

    if(command[1] == NULL || strcmp(command[1], "~") == 0){

       chdir("/home");

    }
    else{

        if(chdir(command[1]) != 0){

            perror("chdir");

        }

    }

}

void exe_export(){ // export command execute

    if (command[1] != NULL) {

        strcpy(var[counter], command[1]);
        strcpy(val[counter], command[2]);
        counter++;

    }
    else {

        printf("Invalid export format. Usage: export (KEY=VALUE)\n");

    }

}

void exe_echo(){ // echo command execute

    if (command[1] != NULL){

        for(int i = 1; command[i] != NULL;  i++){

            printf("%s ", command[i]);

        }
        putchar('\n');
    }
    else{

        printf("Invalid echo format. Usage: export (example)\n");

    }

}

void var_split(){ // split the values of $variable for execvp like (x=-a -l -h)

    char *token;
    int j = 0;

    for (int i = 0; command[i] != NULL && j < SIZE; i++) {

        token = strtok(command[i], " ");

        while (token != NULL && j < SIZE) {

            sub_command[j++] = token;
            token = strtok(NULL, " ");

        }
    }

    sub_command[j] = NULL;
}

void exe_command(){ // commands execute

    var_split();

    pid_t pid = fork();
    if (pid == 0){ // child process

        if (execvp(sub_command[0], sub_command) == -1) {
            if (error_flag){
                printf("Command \"%s\" not found\n", command[0]);
            }
            exit(EXIT_FAILURE);

        }
    }
    else if (pid < 0){

        printf("System Error in Fork command\n");
        return;

    }
    if (back_flag){

        back_flag = 0;
        sleep(2); //  wait until the last background process to start

    }
    else{

        waitpid(pid, NULL, 0);
        write_to_log_file(pid);

    }
    
}

void execute(){ // choose which command to execute

    command[0] = strtok(input, " ");
    if (strcmp(command[0], "cd") == 0){ // change directory command 

        split(command, " ", 1);
        exe_cd(); 

    }
    else if (strcmp(command[0], "export") == 0){

        split(command, "=", 1);
        exe_export();

    }
    else  if (strcmp(command[0], "echo") == 0){

        split(command, " ", 1);
        exe_echo();

    }
    else if (strcmp(command[0], "exit") == 0){

        exit(EXIT_FAILURE);

    }
    else {

        split(command, " ", 1);
        exe_command();

    }
}

void shell(){

    do{

        get_input();
        execute();

    }while(strcmp(command[0], "exit"));

}

int main(){

    register_child_signal();
    setup_environment();
    create_log_file();
    shell();

    return 0;
}