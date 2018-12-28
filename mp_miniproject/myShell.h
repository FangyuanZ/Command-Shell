#ifndef myShell_h
#define myShell_h
#include <dirent.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "var.h"

class myShell
{
  std::string command;
  const char * PATH;
  char ** cmd;
  int cmd_count;
  char ** path;
  std::string directory;
  std::string current_directory;

 public:
  myShell() : command(""), PATH(NULL), cmd_count(0), directory("") {  //constructor
    cmd = new char *[1000]();
    path = new char *[1000]();
    current_directory = getenv("PWD");
  }

  ~myShell() {  //destructor
    delete[] cmd;
    delete[] path;
  }

  void print_prompt() { std::cout << "myShell:" << current_directory << " $ "; }
  void read_prompt() { std::getline(std::cin, command); }
  void parse(char * str,
             std::string delimiters,
             char ** result) {  //function to parse a string with special delimiter
    const char * temp = (delimiters).c_str();
    char * ptr = strtok(str, temp);
    int count = 0;
    while (ptr != NULL) {
      result[count++] = strdup(ptr);
      ptr = strtok(NULL, temp);
    }
  }
  std::string find_substr(
      std::string input,
      std::string substr,
      std::string
          replace) {  //function for finding the particular substring and replace it with a new substring
    size_t pos = input.find(substr);
    while (pos != std::string::npos) {
      input.replace(pos, substr.length(), replace);
      pos = input.find(substr, pos + replace.length());
    }
    return input;
  }
  void find_backslash_and_space() {
    command = find_substr(command, "\\ ", "&&&&");
  }  //turn "\ " into "&&&&"
  void parse_command(std::string temp) {
    char * str = (char *)temp.c_str();
    parse(str, " ", &cmd[0]);
  }
  void delete_sign() {  //turn "&&&&" into " "
    int count = 0;
    while (cmd[count] != NULL) {
      char c[1000];
      strcpy(c, cmd[count]);
      std::string temp(c);
      if (temp.find("&&&&") != std::string::npos) {
        temp = find_substr(temp, "&&&&", " ");
        strcpy(cmd[count], (char *)temp.c_str());
      }
      count++;
    }
  }
  void parse_path(char * str) { parse(str, ":", &path[0]); }

  void get_path() {
    PATH = getenv("PATH");
    if (PATH == NULL) {
      perror("Couldn't find path");
      exit(EXIT_FAILURE);
    }
  }

  bool find_dir(char ** mypath) {  //function for finding the directory
    DIR * Dir;
    struct dirent * dir;
    int count = 0;
    bool has_found = false;
    while (mypath[count] != NULL) {
      Dir = opendir(path[count]);
      if (Dir) {
        while ((dir = readdir(Dir)) != NULL) {
          std::string name = dir->d_name;
          if (name == "" || name == "." || name == "..") {
            continue;
          }
          else if (name == cmd[0]) {
            directory = path[count];
            has_found = true;
          }
        }
      }
      closedir(Dir);
      count++;
    }

    return has_found;
  }

  void reset(char ** temp) {
    int count = 0;
    while (temp[count] != NULL) {
      temp[count] = NULL;
      count++;
    }
  }
  void execute_command() {
    int wstatus;
    pid_t common_pid = fork();
    if (common_pid == -1) {
      perror("error");
      exit(EXIT_FAILURE);
    }
    if (common_pid == 0) {  //perform child process
      execve(cmd[0], cmd, environ);
    }
    else {  //perform parent process
      // code was referenced from man page (waitpid)
      do {
        pid_t wpid = waitpid(common_pid, &wstatus, WUNTRACED | WCONTINUED);
        if (wpid == -1) {
          perror("waitpid");
          exit(EXIT_FAILURE);
        }
        if (WIFEXITED(wstatus)) {
          std::cout << "Program exited with status " << WEXITSTATUS(wstatus) << std::endl;
        }
        else if (WIFSIGNALED(wstatus)) {
          std::cout << "Program killed by signal " << WTERMSIG(wstatus) << std::endl;
        }
        else if (WIFSTOPPED(wstatus)) {
          std::cout << "Program stopped by signal " << WSTOPSIG(wstatus) << std::endl;
        }
        else if (WIFCONTINUED(wstatus)) {
          std::cout << "Program continued" << std::endl;
        }
      } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
    }
  }
  void change_current_directory() {  //function for changing current directory
    char * path = get_current_dir_name();
    std::string temp(path);
    current_directory = temp;
  }

  void execute_cd() {
    if (cmd[1] == NULL) {  //go to home directory
      std::string current_working_directory("/home");
      int return_value = chdir(current_working_directory.c_str());
      if (return_value == -1) {
        perror("cd");
      }
      change_current_directory();
      return;
    }
    if (cmd[1] != NULL && cmd[2] == NULL) {
      std::string current_working_directory(cmd[1]);
      int return_value = chdir(current_working_directory.c_str());
      if (return_value == -1) {
        perror("cd");
      }
      change_current_directory();
      return;
    }
    else {
      std::cout << "too many arguments" << std::endl;
      return;
    }
  }
  int count_cmd() {  //function for counting the number of commands
    int count = 0;
    while (cmd[count] != NULL) {
      count++;
    }
    return count;
  }
  bool is_digit(std::string str) {
    bool result = true;
    if (str[0] == '0') {  //if the first number is 0, then it's not base10
      return false;
    }
    for (size_t i = 0; i < str.length(); i++) {
      if (!(isdigit(str[i]))) {
        result = false;
        break;
      }
    }
    return result;
  }

  void find_dollar1(variable var) {  //find "$" in a string and replace the variable
    size_t pos;
    while ((pos = command.find("$")) != std::string::npos) {
      std::string temp = command;
      size_t pos1 = pos + 1;
      std ::string strr = "";
      while (temp[pos1] != '$' && pos1 != std::string::npos) {
        strr.push_back(temp[pos1]);
        if (var.find_key(
                strr)) {  //if the string after "$" contains a set variable, replace it with its value
          command = find_substr(command, "$" + strr, var.get_variable(strr));
          return;
          //          continue;
        }
        pos1++;
        if (temp[pos1] == '$' || pos1 == std::string::npos) {
          std::cout << "variable not found" << std::endl;
          return;
        }
      }
    }
  }

  void run_myShell() {
    variable var;
    get_path();
    setenv("ECE551PATH", PATH, 1);  //set ECE551PATH
    std::string PATH_temp(PATH);
    parse_path((char *)PATH_temp.c_str());  //parse the PATH with ":"
    while (1) {
      bool has_found = false;
      reset(&cmd[0]);     //make char * cmd[] an empty array
      print_prompt();     //print prompt
      read_prompt();      //read from stdin command str
      find_dollar1(var);  //handle the "$"
      if (std::cin.eof()) {
        std::cout << std::endl;
        return;
      }
      if (command.empty()) {  //if user typed
        continue;
      }
      find_backslash_and_space();  //trun "\ " into "&&&&"
      parse_command(command);      // parse the command by " " and put them into char * cmd[]
      cmd_count = count_cmd();
      if (command.find("&&&&") != std::string::npos) {  //for each cmd[], turn "&&&&" into " "
        delete_sign();
      }
      if (command == "exit") {
        return;
      }
      if (strcmp(cmd[0], "cd") == 0) {
        execute_cd();
        continue;
      }
      if (strcmp(cmd[0], "set") == 0) {
        if (cmd[1] != NULL && cmd[2] != NULL) {
          std::string strr(cmd[1]);
          size_t a = strr.find_first_not_of(  //variable can only be ...
              "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
          if (a != std::string::npos) {
            std::cout << "variable out of range" << std::endl;
            continue;
          }
          std::vector<std::string> var_str;
          std::string result;
          for (int i = 2; i < cmd_count - 1; i++) {  //link values
            std::string temp(cmd[i]);
            result = result + temp + " ";
          }
          std::string temp(cmd[cmd_count - 1]);
          result += temp;
          std::string ans(cmd[1]);
          var_str.push_back(ans);     // ans will be the key
          var_str.push_back(result);  // result will be the value
          var.set_variable(var_str[0], var_str[1]);
          continue;
        }
        else {
          std::cout << "variable not found" << std::endl;
          has_found = true;
        }
      }
      if (strcmp(cmd[0], "export") == 0) {
        char * kv[2];
        if (cmd[1] != NULL) {
          std::string str(cmd[1]);
          if (var.find_key(str)) {
            kv[0] = cmd[1];
            std::string temp = var.get_variable(str);
            kv[1] = (char *)temp.c_str();
            setenv((const char *)kv[0], (const char *)kv[1], 1);  //set new path
            continue;
          }
          else {
            std::cout << "key not found" << std::endl;
            has_found = true;
          }
        }
        else {
          std::cout << "variable not found" << std::endl;
          has_found = true;
        }
      }
      if (strcmp(cmd[0], "inc") == 0) {
        if (cmd[1] != NULL && cmd[2] == NULL) {  //only one argument accepted
          std::string temp(cmd[1]);
          if (var.find_key(temp)) {
            std::string str = var.get_variable(temp);
            if (is_digit(str)) {
              int ans = atoi(str.c_str()) + 1;
              std::cout << "ans = " << ans << std::endl;
              std::ostringstream strr;
              strr << ans;
              std::string result = strr.str();
              var.set_variable(temp, result);
            }
            else {  //set variable to 1
              std::string str_temp = "1";
              var.set_variable(temp, str_temp);
            }
          }
          else {
            std::string str_temp = "1";
            var.set_variable(temp, str_temp);
          }
        }
        else {
          std::cout << "only one key accepted" << std::endl;
          has_found = true;
        }
      }
      else {
        int count = 0;
        if (command.find("/") == std::string::npos) {  //check if the cmd is full path
          while (path[count] != NULL) {
            if (find_dir(&path[count])) {
              has_found = true;
              std::string temp(cmd[0]);
              temp = directory + "/" + temp;
              cmd[0] = (char *)temp.c_str();
              execute_command();  //execute command
              break;
            }
            count++;
          }
        }
        else {
          has_found = true;
          execute_command();  //execute command
          continue;
        }
        if (!has_found) {
          std::cout << "Command " << cmd[0] << " not found" << std::endl;
        }
      }
    }
    exit(EXIT_SUCCESS);
  }
};

#endif
