#include <bits/stdc++.h>
#include <filesystem>
#include <fstream>
using namespace std;
namespace fs = filesystem;

// Tokenizes input while handling quotes and redirection operators
vector<string> parse_tokens(const string &input) {
    vector<string> tokens;
    string currtoken;
    bool in_singlequote = false;
    bool in_doublequote = false;
    bool escape_next = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if (escape_next) {
            currtoken += c;
            escape_next = false;
        }
        else if (in_singlequote) {
            if (c == '\'') in_singlequote = false;
            else currtoken += c;
        }
        else if (in_doublequote) {
            if (c == '\\' && i + 1 < input.size()) {
                char next = input[++i];
                if (next == '\\' || next == '$' || next == '"' || next == '\n') {
                    currtoken += next;
                }
                else {
                    currtoken += c;
                    currtoken += next;
                }
            }
            else if (c == '"') {
                in_doublequote = false;
            }
            else {
                currtoken += c;
            }
        }
        else if (c == '\\') {
            escape_next = true;
        }
        else {
            switch (c) {
                case '\'': 
                    in_singlequote = true;
                    break;
                case '"': 
                    in_doublequote = true;
                    break;
                case '>':
                    if (!currtoken.empty() && isdigit(currtoken.back())) {
                        string num = string(1, currtoken.back());
                        currtoken.pop_back();
                        if (!currtoken.empty()) {
                            tokens.push_back(currtoken);
                        }
                        tokens.push_back(num + ">");
                    }
                    else {
                        if (!currtoken.empty()) {
                            tokens.push_back(currtoken);
                        }
                        tokens.push_back(">");
                    }
                    currtoken.clear();
                    break;
                default:
                    if (isspace(c)) {
                        if (!currtoken.empty()) {
                            tokens.push_back(currtoken);
                            currtoken.clear();
                        }
                    }
                    else {
                        currtoken += c;
                    }
            }
        }
    }

    if (!currtoken.empty()) {
        tokens.push_back(currtoken);
    }

    return tokens;
}

// Finds the full path for a command using PATH environment variable
string get_path(const string &command) {
    if (command.find('/') != string::npos) {
        if (fs::exists(command) && fs::is_regular_file(command)) {
            return fs::absolute(command).string();
        }
        return "";
    }

    stringstream ss(getenv("PATH"));
    string path;
    while (getline(ss, path, ':')) {
        string fullpath = path + "/" + command;
        if (fs::exists(fullpath) && fs::is_regular_file(fullpath)) {
            return fullpath;
        }
    }
    return "";
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    while (true) {
        cout << "$ ";
        string input;
        if (!getline(cin, input)) break;
        if (input.empty()) continue;
        if (input == "exit 0") return 0;

        vector<string> tokens = parse_tokens(input);
        if (tokens.empty()) continue;

        // Parse command and redirection
        vector<string> args;
        string redirect_file;
        for (size_t i = 0; i < tokens.size();) {
            if (tokens[i] == ">" || tokens[i] == "1>") {
                if (++i < tokens.size()) {
                    redirect_file = tokens[i++];
                }
                else {
                    cerr << "Syntax error: missing redirect target\n";
                    redirect_file.clear();
                    break;
                }
            }
            else {
                args.push_back(tokens[i++]);
            }
        }

        if (args.empty()) continue;
        const string &command = args[0];

        // Handle built-in commands
        if (command == "type") {
            ostream *out = &cout;
            ofstream file;
            if (!redirect_file.empty()) {
                file.open(redirect_file);
                out = file.is_open() ? &file : &cerr;
            }

            if (args.size() < 2) {
                *out << "type: missing argument\n";
            }
            else {
                const string &target = args[1];
                const set<string> builtins = {"echo", "exit", "type", "pwd", "cd"};
                if (builtins.count(target)) {
                    *out << target << " is a shell builtin\n";
                }
                else {
                    string path = get_path(target);
                    *out << target << (path.empty() ? ": not found" : " is " + path) << '\n';
                }
            }
        }
        else if (command == "echo") {
            ostream *out = &cout;
            ofstream file;
            if (!redirect_file.empty()) {
                file.open(redirect_file);
                out = file.is_open() ? &file : &cerr;
            }

            for (size_t i = 1; i < args.size(); ++i) {
                if (i > 1) *out << ' ';
                *out << args[i];
            }
            *out << '\n';
        }
        else if (command == "pwd") {
            ostream *out = &cout;
            ofstream file;
            if (!redirect_file.empty()) {
                file.open(redirect_file);
                out = file.is_open() ? &file : &cerr;
            }
            *out << fs::current_path().string() << '\n';
        }
        else if (command == "cd") {
            if (args.size() < 2) {
                cerr << "cd: missing argument\n";
                continue;
            }

            string dir = args[1];
            if (dir == "~") dir = getenv("HOME");

            error_code ec;
            fs::current_path(dir, ec);
            if (ec) {
                cerr << "cd: " << dir << ": " << ec.message() << '\n';
            }
        }
        // Handle external commands
        else {
            string cmd_path = get_path(command);
            if (cmd_path.empty()) {
                cerr << command << ": command not found\n";
                continue;
            }

            // Rebuild command string for popen
            string cmd_line;
            for (const auto &arg : args) {
                cmd_line += (arg.find(' ') != string::npos) ? ("\"" + arg + "\"") : arg;
                cmd_line += ' ';
            }
            if (!cmd_line.empty()) cmd_line.pop_back();

            // Execute command and capture output
            FILE *pipe = popen(cmd_line.c_str(), "r");
            if (!pipe) {
                cerr << "Error executing command\n";
                continue;
            }

            string output;
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe)) {
                output += buffer;
            }

            // Handle output redirection
            if (!redirect_file.empty()) {
                ofstream(redirect_file) << output;
            }
            else {
                cout << output;
            }

            pclose(pipe);
        }
    }

    return 0;
}