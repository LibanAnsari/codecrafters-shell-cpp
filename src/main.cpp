#include <bits/stdc++.h>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

string get_path(string command) {
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
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while(true){
    cout << "$ ";

    string input;
    getline(cin, input);

    if(input == "exit 0") return 0;

    string firstWord = input.substr(0, input.find(' ')); 

    if(firstWord == "type"){
      string cmd = input.substr(5, input.size());
      if(cmd == "echo" || cmd == "exit" || cmd == "type"){
        cout << cmd << " is a shell builtin" << endl;
      }else{
        string path = get_path(cmd);
        if(!path.empty()){
          cout << input.substr(5) << " is " << path << endl;
        }else{
          cout << cmd << ": not found" << endl;
        }
      }
    }else if(firstWord == "echo"){
      cout << input.substr(input.find(' ')+1) << endl;
    }else{
      string cmd = input.substr(0, input.find(' '));
      string path = get_path(cmd);
      if(path.empty()){
        cout << input << ": command not found" << endl;
      }else{
        string args = input.substr(cmd.length());
        string full = "./" + cmd + args; // Use only the executable name and arguments
        char buffer[128];
        string result;

        FILE *pipe = popen(full.c_str(), "r");

        while(!feof(pipe)){
          if(fgets(buffer, 128, pipe) != NULL){
            cout << buffer;
          }
        }
        pclose(pipe);
      }
    }

  }

  return 0;
}
