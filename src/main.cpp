#include <bits/stdc++.h>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

vector<string> split_string(const string &s, char delimiter){
  stringstream ss(s);
  vector<string> return_vect;
  string token;
  while(getline(ss, token, delimiter)){
    return_vect.push_back(token);
  }
  return return_vect;
}

string get_path(string command){
  // Get the PATH environment variable
  string path_env = getenv("PATH");
  stringstream ss(path_env);

  string path;
  // Split the PATH variable by ':' and check each directory
  while(getline(ss, path, ':')){
      string abs_path = path + "/" + command;

      // Check if the command exists in the current directory
      if(fs::exists(abs_path)){
          return abs_path;
      }
  }
  return "";
}

int main(){
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while(true){
      cout << "$ ";
      string input;
      getline(cin, input);

      if(input == "exit 0") return 0;

      string path_env = getenv("PATH");
      vector<string> path = split_string(path_env, ':');
      
      istringstream iss(input);
      vector<string> tokens;
      string token;
      while(iss >> token) tokens.push_back(token);
      
      if(tokens.empty()) continue;
      
      string command = tokens[0];
      
      if(command == "type"){
        if(tokens.size() < 2) continue;
        string cmd = tokens[1];
        if(cmd == "echo" or cmd == "exit" or cmd == "type"){
          cout << cmd << " is a shell builtin" << endl;
        }else{
          string path = get_path(cmd);
          if(!path.empty()){
            cout << cmd << " is " << path << endl;
          }else{
            cout << cmd << ": not found" << endl;
          }
        }
      }else if(command == "echo"){
        cout << input.substr(input.find(' ') + 1) << endl;
      }else{
        string filepath;
        for(int i = 0; i < path.size(); i++){
          filepath = path[i] + '/' + tokens[0];
          ifstream file(filepath);
          if(file.good()){
            string command = "exec " + path[i] + '/' + input;
            system(command.c_str());
            break;
          }else if(i == path.size() - 1){
            cout << tokens[0] << ": not found" << endl;
          }
        }
      }
  }
  return 0;
}
