#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <regex>
#include <variant>

using namespace std;

// Function prototypes
void read_file(const string& filename); // function to read input file
void write_to_file(const vector<string>& lines, const string& output_filename); // functions to write output to output file
string get_output_filename(const string& filename); // function to get the name for creating output file
void process_command_line(const string& line, const string& current_database); // check for command line
void print_tables();

vector<string> processed_command_line_outputs; // a vector of strings containing processed lines after reading in files
vector<pair<string, vector<vector<variant<int, string>>>>> tables;

int main() {
    string current_directory = filesystem::current_path().string();  // Get current directory

    string input_filename = filesystem::current_path().string() + "/Database/" + "fileInput1.mdb";
    read_file(input_filename);

    return 0;
}

// Function to read the input file
void read_file(const string& filename) {
    ifstream file(filename);
    string output_filename = get_output_filename(filename);

    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            if (!line.empty()) {
                processed_command_line_outputs.push_back("> " + line);
                cout << "> " << line << endl;
            }
            process_command_line(line, filename);
        }
        file.close();

        string output_file_path = filesystem::current_path().string() + "/Database/" + output_filename;
        write_to_file(processed_command_line_outputs, output_file_path);

    } else {
        cout << "File not found: " << filename << endl;
    }
}

// Function to generate output filename based on the input filename
string get_output_filename(const string& filename) {
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        if (line.find("CREATE ") == 0 && line.find("TABLE") == string::npos && line.back() == ';') {
            string output_filename = line.substr(7);  // Remove the "CREATE " part
            if (output_filename.back() == ';') {
                output_filename.pop_back();
            }
            return output_filename;
        }
    }
    return "output.txt";
}

// Function to write processed commands to the output file
void write_to_file(const vector<string>& lines, const string& output_filename) {
    ofstream output_file(output_filename);

    if (output_file.is_open()) {
        for (const auto& line : lines) {
            output_file << line << endl;
        }
        output_file.close();
    } else {
        cout << "Could not open file for writing: " << output_filename << endl;
    }
}

// Function to process each command line
void process_command_line(const string& line, const string& current_database) {
    regex databases_command("(DATABASES;)");
    regex create_command("(CREATE TABLE)(.*)");
    regex insert_command("(INSERT INTO)(.*)");
    regex select_command("(SELECT)(.*)");
    regex update_command("(UPDATE)(.*)");
    regex delete_command("(DELETE)(.*)");
    regex tables_command("(TABLES;)");
    regex select_all_from_command(R"(SELECT\s+\*\s+FROM\s+(\w+);)");

    string table_name;

    smatch m;
    if (regex_search(line, m, databases_command)) {
        char full_path[FILENAME_MAX];    // full_path is an array which contains FILENAME_MAX (the max length of a file)
        _fullpath(full_path, (current_database).c_str(), FILENAME_MAX);   // (gets abs full path using _fullpath function (current_database).c_str() is the relative path to the database ,the max length of a file)
        string database_path = full_path; // full path converted to a string called database_path
        cout << database_path << endl;
        processed_command_line_outputs.push_back(database_path);
    }

    if (regex_search(line, m, create_command)) {
        if (line.find("CREATE TABLE ") == 0) {
            table_name = line.substr(13, line.find('(') - 13);

            // Extract the columns
            //extracts the substring between the parentheses. The + 1 skips the opening parenthesis,
            //and the - 1 excludes the closing parenthesis
            string columns_str = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);
            regex column_regex(R"((\w+)\s+(\w+))"); 
            //(\w+): Matches one or more word characters (the column name).
            //\s+: Matches one or more spaces.
            //(\w+): Matches one or more word characters (the data type, either INT or TEXT).
            // column_begin is the first match found, column_end is the last match found
            auto column_begin = sregex_iterator(columns_str.begin(), columns_str.end(), column_regex); //column_regex is the pattern needed to match
            auto column_end = sregex_iterator(); 

            vector<variant<int, string>> headers;
            for (auto i = column_begin; i != column_end; ++i) {
                string column_name = (*i)[1].str(); //(*it)[1].str() extract column name
                string column_type = (*i)[2].str(); //(*it)[2].str() extract column type
                headers.push_back(column_name);  // insert name into the headers vector
            }

            // Add the table to the 'tables' vector
            tables.push_back({table_name, {headers}});
        }
    }

    if (regex_search(line, m, tables_command)) {
        for (const auto& table : tables) {
            cout <<table.first << endl;
            processed_command_line_outputs.push_back(table.first); //insert the table name 

            // for (const auto& row : table.second) { // used to get headers
            //     for (const auto& col : row) {
            //         // Check the type of the variant before accessing it
            //         if (std::holds_alternative<int>(col)) { // if it holds int cout int
            //             cout << std::get<int>(col) << " ";
            //         } else if (std::holds_alternative<string>(col)) { // if it holds string cout string
            //             cout << std::get<string>(col) << " ";
            //         }
            //     }
            // }
        }
    //print_tables();
    }

    if (regex_search(line, m, select_all_from_command)) {
        string output;
        for (const auto& table : tables) {
        //cout << "Table: " << table.first << endl;  // Print table name
            for (const auto& header_row : table.second) {
                for (auto it = header_row.begin(); it != header_row.end(); ++it) {
                    if (std::holds_alternative<string>(*it)) {
                        output = output + std::get<string>(*it); // add column into string called output
                        if (it != header_row.end() - 1) {  // Check if it's not the last column
                            output = output + ", "; //add "," for string that is not the last one
                        }
                    }
                }
                cout<<output;
                processed_command_line_outputs.push_back(output);
                cout << endl;
                output.clear();
            }
        }
    }

}

void print_tables() {
    for (const auto& table : tables) {
        //cout << "Table: " << table.first << endl;  // Print table name
        for (const auto& header_row : table.second) {
            cout << "Headers: ";
            for (auto it = header_row.begin(); it != header_row.end(); ++it) {
                if (std::holds_alternative<string>(*it)) {
                    cout << std::get<string>(*it);
                    if (it != header_row.end() - 1) {  // Check if it's not the last column
                        cout << ", ";
                    }
                }

            }
            cout << endl;
        }
    }
}







