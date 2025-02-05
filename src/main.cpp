#include <bits/stdc++.h>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

vector<string> parse_tokens(const string &input){
  vector<string> tokens;
  string currtoken;
  bool in_quote = false;

  for(char c : input){
    if(in_quote){
      if(c == '\'') in_quote = false;
      else currtoken += c;
    }else{
      if(c == '\''){
        in_quote = true;
      }else if(isspace(c)){
        if(!currtoken.empty()){
          tokens.push_back(currtoken);
          currtoken.clear();
        }
      }else{
        currtoken += c;
      }
    }
  }
  return tokens;
}

string get_path(string command) {
  if(command.find('/') != string::npos){
    if(fs::exists(command) and fs::is_regular_file(command)){
      return fs::absolute(command).string();
    }else{
      return "";
    }
  }

  string path_env = getenv("PATH");
  stringstream ss(path_env);
  string path;

  while(getline(ss, path, ':')){
    string abs_path = path + "/" + command;
    if(fs::exists(abs_path) && fs::is_regular_file(abs_path)){
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

    if(input.empty()) continue;

    if(input == "exit 0") return 0;

    istringstream iss(input);
    vector<string> tokens = parse_tokens(input);
    // string token;
    // while(iss >> token)
    //   tokens.push_back(token);

    if(tokens.empty())continue;

    // for(auto i : tokens) cout << i << endl;

    string command = tokens[0];

    if(command == "type"){
      if(tokens.size() < 2)
        continue;
      string cmd = tokens[1];
      if(cmd == "echo" or cmd == "exit" or cmd == "type" or cmd == "pwd" or cmd == "cd"){
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
      // cout << input.substr(input.find(' ') + 1) << endl;
      if(tokens.size() < 2) cout << endl;
      else{
        for(int i = 1 ; i < tokens.size() ; i++){
          if (i > 1) cout << ' ';
          cout << tokens[i];
        } cout << endl;
      }
    }else if(command == "pwd"){
      string cwd = fs::current_path().string();
      cout << cwd << endl;
    }else if (command == "cd") {
      if(tokens.size() < 2){
        cerr << "cd: missing argument" << endl;
        continue;
      }
      string dir = tokens[1];

      // If User's Home Directory
      if(dir == "~"){
        fs::current_path(getenv("HOME"));
        continue;
      }

      // Check if the directory exists
      bool exists = fs::exists(dir);

      if(!exists){
        cerr << "cd: " << dir << ": No such file or directory" << endl;
        continue;
      }
      
      // Check if the path is a directory
      bool is_dir = fs::is_directory(dir);
      
      if(!is_dir){
        cerr << "cd: " << dir << ": Not a directory" << endl;
        continue;
      }

      // Attempt to change directory and handle errors
      fs::current_path(dir);
    }else{
      string cmd = tokens[0];
      string path = get_path(cmd);
      if(path.empty()){
        cout << cmd << ": command not found" << endl;
      }else{
        FILE *pipe = popen(input.c_str(), "r");
        if(!pipe){
          cerr << "Error executing command" << endl;
          continue;
        }

        char buffer[128];
        while(fgets(buffer, sizeof(buffer), pipe) != nullptr){
          cout << buffer;
        }

        int status = pclose(pipe);
        if(status != 0){
          cerr << "Command exited with status " << status << endl;
        }
      }
    }
  }
  return 0;
}