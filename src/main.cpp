#include <bits/stdc++.h>
#include <filesystem>
#include <fstream>
using namespace std;
namespace fs = filesystem;

vector<string> parse_tokens(const string &input) {
    vector<string> tokens;
    string currtoken = "";
    bool in_singlequote = false;
    bool in_doublequote = false;
    bool escape_next = false;

    for (int i = 0; i < input.size(); i++) {
        char c = input[i];
        if (escape_next) {
            currtoken += c;
            escape_next = false;
        } else if (in_singlequote) {
            if (c == '\'') {
                in_singlequote = false;
            } else {
                currtoken += c;
            }
        } else if (in_doublequote) {
            if (c == '\\') {
                if (i + 1 < input.size()) {
                    char next_c = input[i + 1];
                    if (next_c == '\\' || next_c == '$' || next_c == '"' || next_c == '\n') {
                        currtoken += next_c;
                        i++;
                    } else {
                        currtoken += c;
                    }
                } else {
                    currtoken += c;
                }
            } else if (c == '"') {
                in_doublequote = false;
            } else {
                currtoken += c;
            }
        } else if (c == '\\') {
            escape_next = true;
        } else {
            if (c == '\'') {
                in_singlequote = true;
            } else if (c == '"') {
                in_doublequote = true;
            } else if (isspace(c)) {
                if (!currtoken.empty()) {
                    tokens.push_back(currtoken);
                    currtoken.clear();
                }
            } else if (c == '>') {
                if (!currtoken.empty()) {
                    char last_char = currtoken.back();
                    if (isdigit(last_char)) {
                        string part = currtoken.substr(0, currtoken.size() - 1);
                        string op = string(1, last_char) + ">";
                        if (!part.empty()) {
                            tokens.push_back(part);
                        }
                        tokens.push_back(op);
                    } else {
                        tokens.push_back(currtoken);
                        tokens.push_back(">");
                    }
                    currtoken.clear();
                } else {
                    tokens.push_back(">");
                }
            } else {
                currtoken += c;
            }
        }
    }

    if (!currtoken.empty()) {
        tokens.push_back(currtoken);
    }

    return tokens;
}

string get_path(string command) {
    if (command.find('/') != string::npos) {
        if (fs::exists(command) && fs::is_regular_file(command)) {
            return fs::absolute(command).string();
        } else {
            return "";
        }
    }

    string path_env = getenv("PATH");
    stringstream ss(path_env);
    string path;

    while (getline(ss, path, ':')) {
        string abs_path = path + "/" + command;
        if (fs::exists(abs_path) && fs::is_regular_file(abs_path)) {
            return abs_path;
        }
    }
    return "";
}

int main() {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true) {
        cout << "$ ";
        string input;
        getline(cin, input);

        if (input.empty()) continue;

        if (input == "exit 0") return 0;

        vector<string> tokens = parse_tokens(input);
        if (tokens.empty()) continue;

        vector<string> args;
        string redirect_file;

        for (size_t i = 0; i < tokens.size();) {
            if (tokens[i] == ">" || tokens[i] == "1>") {
                if (i + 1 < tokens.size()) {
                    redirect_file = tokens[i + 1];
                    i += 2;
                } else {
                    cerr << "Syntax error: missing filename for redirection." << endl;
                    redirect_file.clear();
                    break;
                }
            } else {
                args.push_back(tokens[i]);
                i++;
            }
        }

        if (args.empty()) continue;

        string command = args[0];

        if (command == "type") {
            if (args.size() < 2) continue;
            string cmd = args[1];
            ostream *output = &cout;
            ofstream out_file;
            if (!redirect_file.empty()) {
                out_file.open(redirect_file);
                if (!out_file) {
                    cerr << "Error opening file for redirection." << endl;
                    continue;
                }
                output = &out_file;
            }
            if (cmd == "echo" || cmd == "exit" || cmd == "type" || cmd == "pwd" || cmd == "cd") {
                *output << cmd << " is a shell builtin" << endl;
            } else {
                string path = get_path(cmd);
                if (!path.empty()) {
                    *output << cmd << " is " << path << endl;
                } else {
                    *output << cmd << ": not found" << endl;
                }
            }
        } else if (command == "echo") {
            ostream *output = &cout;
            ofstream out_file;
            if (!redirect_file.empty()) {
                out_file.open(redirect_file);
                if (!out_file) {
                    cerr << "Error opening file for redirection." << endl;
                    continue;
                }
                output = &out_file;
            }
            if (args.size() < 2) {
                *output << endl;
            } else {
                for (size_t i = 1; i < args.size(); ++i) {
                    if (i > 1) *output << ' ';
                    *output << args[i];
                }
                *output << endl;
            }
        } else if (command == "pwd") {
            ostream *output = &cout;
            ofstream out_file;
            if (!redirect_file.empty()) {
                out_file.open(redirect_file);
                if (!out_file) {
                    cerr << "Error opening file for redirection." << endl;
                    continue;
                }
                output = &out_file;
            }
            string cwd = fs::current_path().string();
            *output << cwd << endl;
        } else if (command == "cd") {
            if (args.size() < 2) {
                cerr << "cd: missing argument" << endl;
                continue;
            }
            string dir = args[1];
            if (dir == "~") {
                fs::current_path(getenv("HOME"));
                continue;
            }
            if (!fs::exists(dir)) {
                cerr << "cd: " << dir << ": No such file or directory" << endl;
                continue;
            }
            if (!fs::is_directory(dir)) {
                cerr << "cd: " << dir << ": Not a directory" << endl;
                continue;
            }
            fs::current_path(dir);
        } else {
            string cmd_str;
            for (size_t i = 0; i < args.size(); ++i) {
                if (i > 0) cmd_str += ' ';
                cmd_str += args[i];
            }
            FILE *pipe = popen(cmd_str.c_str(), "r");
            if (!pipe) {
                cerr << "Error executing command" << endl;
                continue;
            }
            char buffer[128];
            string output_str;
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                output_str += buffer;
            }
            int status = pclose(pipe);
            if (!redirect_file.empty()) {
                ofstream out_file(redirect_file);
                if (out_file) {
                    out_file << output_str;
                } else {
                    cerr << "Error opening file for redirection." << endl;
                }
            } else {
                cout << output_str;
            }
            if (status != 0) {
                cerr << "Command exited with status " << status << endl;
            }
        }
    }
    return 0;
}