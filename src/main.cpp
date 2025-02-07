#include <bits/stdc++.h>
#include <termios.h>
#include <unistd.h>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

vector<string> parse_tokens(const string &input){ // basically istringstream but manual
  vector<string> tokens;
  string currtoken = "";
  bool in_singlequote = false;
  bool in_doublequote = false;
  bool escape_next = false;

  for (int i = 0; i < input.size(); i++){
    char c = input[i];
    if (escape_next){
      currtoken += c;
      escape_next = false;
    }
    else if (c == '\\' and !in_singlequote and !in_doublequote){
      escape_next = true;
    }
    else if (in_singlequote){
      if (c == '\'')
        in_singlequote = false;
      else
        currtoken += c;
    }
    else if (in_doublequote){
      if (c == '\\'){
        if (i + 1 < input.size()){
          char next_c = input[i + 1];
          if (next_c == '\\' or next_c == '$' or next_c == '"' or next_c == '\n'){
            currtoken += next_c;
            i++; // Skip the next character
          }
          else{
            currtoken += c; // Add backslash as literal
          }
        }
        else{
          currtoken += c; // Backslash at end of input
        }
      }
      else if (c == '"'){
        in_doublequote = false;
      }
      else{
        currtoken += c;
      }
    }
    else if (c == '\\'){
      currtoken += ' ';
    }
    else{
      if (c == '\''){
        in_singlequote = true;
      }
      else if (c == '"'){
        in_doublequote = true;
      }
      else if (isspace(c)){
        if (!currtoken.empty()){
          tokens.push_back(currtoken);
          currtoken.clear();
        }
      }
      else{
        currtoken += c;
      }
    }
  }

  if (!currtoken.empty()){
    tokens.push_back(currtoken);
  }

  return tokens;
}

string get_path(string command){
  if (command.find('/') != string::npos){
    if (fs::exists(command) and fs::is_regular_file(command)){
      return fs::absolute(command).string();
    }
    else{
      return "";
    }
  }

  string path_env = getenv("PATH");
  stringstream ss(path_env);
  string path;
  while (getline(ss, path, ':')){
    string abs_path = path + "/" + command;
    if (fs::exists(abs_path) && fs::is_regular_file(abs_path)){
      return abs_path;
    }
  }
  return "";
}

bool completeCommand(string &command) {
    for (const string &cmd : command) {
        if (cmd.find(command) == 0) { // Check if command is a prefix
            cout << "\r$ " << cmd << " ";
            command = cmd; // Update command to the completed version
            return true;
        }
    }
    return false;
}

void enableRawMode(){
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void disableRawMode(){
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag |= (ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void handleTabPress(std::string &input){
  if (input == "ech"){
    input = "echo ";
    std::cout << "o ";
  }
  else if (input == "exi"){
    input = "exit ";
    std::cout << "t ";
  }
}

void readInputWithTabSupport(string &input){
  enableRawMode();
  char c;
  while (true){
    c = getchar();
    if (c == '\n'){
      cout << endl;
      break;
    }
    else if (c == '\t'){
      if (completeCommand(input))
        input += " ";
      else
        cout << "\a";
    }
    else if (c == 127){
      if (!input.empty()){
        input.pop_back();
        cout << "\b \b"; // Move cursor back, overwrite character with space, move cursor back again.
      }
    }
    else{
      input += c;
      cout << c;
    }
  }
  disableRawMode();
}

int main(){
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true){
    cout << "$ ";

    string input;
    // getline(cin, input);
    readInputWithTabSupport(input);

    if (input.empty())
      continue;

    if (input == "exit 0")
      return 0;

    vector<string> tokens = parse_tokens(input);
    // istringstream iss(input);
    // string token;
    // while(iss >> token)
    //   tokens.push_back(token);

    if (tokens.empty())
      continue;

    // for(auto i : tokens) cout << i << endl; (tokens debugger)

    string command = tokens[0];

    if (command == "type"){
      if (tokens.size() < 2)
        continue;
      string cmd = tokens[1];
      if (cmd == "echo" or cmd == "exit" or cmd == "type" or cmd == "pwd" or cmd == "cd"){
        cout << cmd << " is a shell builtin" << endl;
      }
      else{
        string path = get_path(cmd);
        if (!path.empty()){
          cout << cmd << " is " << path << endl;
        }
        else{
          cout << cmd << ": not found" << endl;
        }
      }
    }
    else if (command == "echo"){
      if (tokens.size() < 2)
        cout << endl;
      else{
        for (int i = 1; i < tokens.size(); i++){
          if (i > 1)
            cout << ' ';
          cout << tokens[i];
        }
        cout << endl;
      }
    }
    else if (command == "pwd"){
      string cwd = fs::current_path().string();
      cout << cwd << endl;
    }
    else if (command == "cd"){
      if (tokens.size() < 2){
        cerr << "cd: missing argument" << endl;
        continue;
      }
      string dir = tokens[1];
      // If User's Home Directory
      if (dir == "~"){
        fs::current_path(getenv("HOME"));
        continue;
      }
      // Check if the directory exists
      bool exists = fs::exists(dir);
      if (!exists){
        cerr << "cd: " << dir << ": No such file or directory" << endl;
        continue;
      }

      // Check if the path is a directory
      bool is_dir = fs::is_directory(dir);

      if (!is_dir){
        cerr << "cd: " << dir << ": Not a directory" << endl;
        continue;
      }
      // Attempt to change directory and handle errors
      fs::current_path(dir);
    }
    else{
      string cmd = tokens[0];
      string path = get_path(cmd);
      if (path.empty()){
        cout << cmd << ": command not found" << endl;
      }
      else{
        FILE *pipe = popen(input.c_str(), "r");
        if (!pipe){
          cerr << "Error executing command" << endl;
          continue;
        }
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr){
          cout << buffer;
        }
        int status = pclose(pipe);
        if (status != 0){
          cerr << "Command exited with status " << status << endl;
        }
      }
    }
  }
  return 0;
}