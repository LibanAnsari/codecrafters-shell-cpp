#include <iostream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <fstream>
std::vector<std::string> split_string(const std::string &s, char delimiter){
   std::stringstream ss(s);
    std::vector<std::string> return_vect;
    std::string token;
    while(getline(ss, token, delimiter)){
      return_vect.push_back(token);
    }
    return return_vect;
}
void handle_type_command(std::vector<std::string> arguments, std::vector<std::string> path){
  if(arguments[1] == "echo" || arguments[1] == "exit" || arguments[1] == "type"){
        std::cout << arguments[1] << " is a shell builtin\n";
  }else{
    std::string filepath;
    for(int i = 0; i < path.size(); i++){
      filepath = path[i] + '/' + arguments[1];
      std::ifstream file(filepath);
      if(file.good()){
        std::cout << arguments[1] << " is " << filepath << "\n";
        return;
      }
    }
    std::cout << arguments[1] << ": not found\n";
  }
}
int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::string path_string = getenv("PATH");
  std::vector<std::string> path = split_string(path_string, ':');
  std::string input;
  std::vector<std::string> arguments;
  while(true){
    std::cout << "$ ";
    std::getline(std::cin, input);
    if(input == "exit 0"){
      return 0;
    }
    arguments = split_string(input, ' ');
    if(arguments[0] == "echo"){
      for(int i = 1; i < arguments.size(); i++){
        if(i == arguments.size() -1)
          std::cout << arguments[i] << "\n";
        else
          std::cout << arguments[i] << " ";
      }
    }else if(arguments[0] == "type"){
      handle_type_command(arguments, path);
    }else{
      std::string filepath;
      for(int i = 0; i < path.size(); i++){
        filepath = path[i] + '/' + arguments[0];
        std::ifstream file(filepath);
        if(file.good()){
          std::string command = "exec " + path[i] + '/' + input;
          std::system(command.c_str());
          break;
        }
        else if(i == path.size() - 1){
          std::cout << arguments[0] << ": not found\n";
        }
      }
    }
  }
  
}