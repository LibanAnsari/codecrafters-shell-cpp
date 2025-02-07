#include <bits/stdc++.h>
#include <filesystem>
#include <unistd.h>
#include <termios.h>
#include <cctype>
using namespace std;
namespace fs = filesystem;

vector<string> parse_tokens(const string &input) {
    vector<string> tokens;
    string currtoken = "";
    bool in_singlequote = false;
    bool in_doublequote = false;
    bool escape_next = false;

    for(int i = 0 ; i < input.size() ; i++){
        char c = input[i];
        if(escape_next){
            currtoken += c;
            escape_next = false;
        }else if(c == '\\' and !in_singlequote and !in_doublequote){
            escape_next = true;
        }else if(in_singlequote){
            if(c == '\'') in_singlequote = false;
            else currtoken += c;
        }else if(in_doublequote){
            if(c == '\\'){
                if(i + 1 < input.size()){
                    char next_c = input[i + 1];
                    if(next_c == '\\' or next_c == '$' or next_c == '"' or next_c == '\n'){
                        currtoken += next_c;
                        i++;
                    }else{
                        currtoken += c;
                    }
                }else{
                    currtoken += c;
                }
            }else if(c == '"'){
                in_doublequote = false;
            }else{
                currtoken += c;
            }
        }else if(c ==   '\\'){
            currtoken += ' ';
        }else{
            if(c == '\''){
                in_singlequote = true;
            }else if(c == '"'){
                in_doublequote = true;
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
    if(!currtoken.empty()) {
        tokens.push_back(currtoken);
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
    struct termios original_termios;
    tcgetattr(STDIN_FILENO, &original_termios);

    struct termios new_termios = original_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    auto restore_terminal = [&]() {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
    };
    atexit(restore_terminal);

    cout << unitbuf;
    cerr << unitbuf;

    while(true) {
        cout << "$ " << flush;

        string input_buffer;
        char c;

        while(read(STDIN_FILENO, &c, 1) == 1) {
            if (c == '\n') {
                cout << endl;
                break;
            } else if (c == '\t') {
                vector<string> possible;
                for (const string& cmd : {"echo", "exit"}) {
                    if (input_buffer.size() <= cmd.size() && 
                        input_buffer == cmd.substr(0, input_buffer.size())) {
                        possible.push_back(cmd);
                    }
                }
                if (possible.size() == 1) {
                    input_buffer = possible[0] + " ";
                    cout << "\r$ " << input_buffer << "\033[K" << flush;
                }
            } else if (c == 127 || c == '\b') { // Handle backspace
                if (!input_buffer.empty()) {
                    input_buffer.pop_back();
                    cout << "\b \b" << flush;
                }
            } else if (isprint(c)) {
                input_buffer += c;
                cout << c << flush;
            }
        }

        if (input_buffer.empty()) continue;

        vector<string> tokens = parse_tokens(input_buffer);
        if (tokens.empty()) continue;

        string command = tokens[0];

        if (command == "type") {
            if (tokens.size() < 2) continue;
            string cmd = tokens[1];
            if (cmd == "echo" || cmd == "exit" || cmd == "type" || cmd == "pwd" || cmd == "cd") {
                cout << cmd << " is a shell builtin" << endl;
            } else {
                string path = get_path(cmd);
                if (!path.empty()) {
                    cout << cmd << " is " << path << endl;
                } else {
                    cout << cmd << ": not found" << endl;
                }
            }
        } else if (command == "echo") {
            for (size_t i = 1; i < tokens.size(); ++i) {
                if (i > 1) cout << ' ';
                cout << tokens[i];
            }
            cout << endl;
        } else if (command == "pwd") {
            cout << fs::current_path().string() << endl;
        } else if (command == "cd") {
            if (tokens.size() < 2) {
                cerr << "cd: missing argument" << endl;
                continue;
            }
            string dir = tokens[1];
            if (dir == "~") {
                dir = getenv("HOME");
            }
            error_code ec;
            fs::current_path(dir, ec);
            if (ec) {
                cerr << "cd: " << dir << ": " << ec.message() << endl;
            }
        } else if (command == "exit") {
            int status = 0;
            if (tokens.size() > 1) {
                try {
                    status = stoi(tokens[1]);
                } catch (...) {
                    cerr << "exit: invalid argument" << endl;
                    continue;
                }
            }
            return status;
        } else {
            string path = get_path(command);
            if (path.empty()) {
                cout << command << ": command not found" << endl;
            } else {
                string full_cmd = input_buffer + " 2>&1";
                FILE* pipe = popen(full_cmd.c_str(), "r");
                if (!pipe) {
                    cerr << "Error executing command" << endl;
                    continue;
                }
                char buffer[128];
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    cout << buffer;
                }
                int status = pclose(pipe);
                if (status != 0) {
                    cerr << "Command exited with status " << status << endl;
                }
            }
        }
    }
    return 0;
}