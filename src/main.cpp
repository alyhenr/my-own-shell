#include <iostream>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <filesystem>
#include <cstring> 
#include <cstdlib>
#include <sys/wait.h>

std::vector<std::string> getKeyWord(std::string str) {
  std::string token;
  std::stringstream ss(str);
  std::vector<std::string> tokens;
  while (getline(ss, token, ' ')) {
    tokens.push_back(token);
  }

  return tokens;
}

std::vector<std::string> getPaths(const char* s) {
  std::vector<std::string> res;

  std::istringstream stream(s);
  std::string path;

  while (std::getline(stream, path, ':')) {
    res.push_back(path);
  }

  return res;
}

bool findCommand(std::string fullPath) {
  return std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath);
}

int main(int argc, char *argv[]) {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  const char* paths = std::getenv("PATH");

    // if (paths) {
    //     std::cout << "PATH: " << paths << std::endl;
    // } else {
    //     std::cout << "PATH variable is not set." << std::endl;
    // }

  std::string input;

  std::unordered_map<std::string, std::string> keywords = {
    {"echo", ""},
    {"exit", ""},
    {"type", ""}
  };

  while (1) {
    std::cout << "$ ";
    std::getline(std::cin, input);

    std::vector<std::string> tokens = getKeyWord(input);
    std::string cmd = tokens[0];
    
    if (input == "exit 0") break;

    if (cmd == "type") {
      cmd = input.substr(cmd.size() + 1);

      if (keywords.find(cmd) != keywords.end()) {
        std::cout << cmd + " is a shell builtin" + "\n";
      } else {
        if (paths != NULL) {
          bool commandFound = false;
          for (std::string path : getPaths(paths)) {
            std::string fullPath = path + "/" + cmd;
            commandFound = findCommand(fullPath);
            if (commandFound) {
              std::cout << cmd + " is " + fullPath + "\n";
              break;
            }
          }
          if (!commandFound) {
            std::cout << cmd + ": not found\n";
          }
        } else  {
          std::cout << cmd + ": not found\n";
        }
      }
    } else {
      if (keywords.find(cmd) != keywords.end()) {
        std::cout << keywords[cmd];
        if (input.size() > cmd.size()) std::cout << input.substr(cmd.size() + 1) + "\n";
      } else {
        bool commandFound = false;
        for (std::string path : getPaths(paths)) {
            std::string fullPath = path + "/" + cmd;
            if (findCommand(fullPath)) {
                commandFound = true;
                pid_t pid = fork();

                if (pid == -1) {
                    perror("fork failed");
                    break;
                } 
                else if (pid == 0) {
                    std::vector<char*> args;
                    args.push_back(strdup(cmd.c_str()));

                    for (size_t i = 1; i < tokens.size(); i++) {
                        args.push_back(strdup(tokens[i].c_str()));
                    }
                    args.push_back(nullptr);

                    execvp(args[0], args.data());

                    perror("execvp failed");
                    exit(EXIT_FAILURE);
                } 
                else {
                    int status;
                    waitpid(pid, &status, 0);
                }
                break;
            }
        }
        if (!commandFound) std::cout << cmd + ": command not found\n";
      }
    }
  }
}