#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <regex>
#include <variant>
#include <sstream>

using namespace std;

// Function prototypes
void read_file(const string& filename); // function to read input file
void write_to_file(const vector<string>& lines, const string& output_filename); // functions to write output to output file
string get_output_filename(const string& filename); // function to get the name for creating output file
void process_command_line(const string& line, const string& current_database); // check for command line
void print_tables();

vector<string> processed_command_line_outputs; // a vector of strings containing processed lines after reading in files
vector<pair<string, vector<vector<variant<int, string>>>>> tables;

const int MAX_COLUMN = 10;

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
    regex databases_command("(DATABASES;)"); // YAP CHI YI
    regex create_command("(CREATE TABLE)(.*)"); // YAP CHI YI
    regex insert_command("(INSERT INTO)(.*)"); // TAN YONG XIN
    regex update_command("(UPDATE)(.*)"); // THAM MEI TING
    regex delete_command("(DELETE)(.*)"); // TAN YONG XIN
    regex select_count_command(R"(SELECT\s+COUNT\(\*\)\s+FROM\s+(\w+);)"); // THAM MEI TING
    regex tables_command("(TABLES;)"); // YAP CHI YI
    regex select_all_from_command(R"(SELECT\s+\*\s+FROM\s+(\w+);)"); // YAP CHI YI

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
                string column_type = (*i)[2].str(); //(*it)[2].str() extract column type // no use
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
        }
    }

    if (regex_search(line, m, select_all_from_command)) {
        string table_name = m[1];  // Capture the table name from the SELECT query
        string header_row;
        string data_row;

        for (const auto& table : tables) {
            if (table.first == table_name) {  // Match the table name
                // Print column headers
                for (int i = 0; i < table.second[0].size(); ++i) { // i = 0 here access the header row
                    if (std::holds_alternative<string>(table.second[0][i])) {
                        if (i != table.second[0].size() - 1) {
                            cout << std::get<string>(table.second[0][i]) + ",";
                            header_row = header_row + std::get<string>(table.second[0][i]) + "," ;
                            
                        } else { // check if the header is the last one, if yes dont add "," to it
                            cout << std::get<string>(table.second[0][i]);
                            header_row = header_row + std::get<string>(table.second[0][i])  ;
                        }
                    }
                }
                processed_command_line_outputs.push_back(header_row);
                cout << endl;

                // Print rows of data
                for (int i = 1; i < table.second.size(); ++i) {  // starts from row 1 as row 0 here is the header
                    for (int j = 0; j < table.second[i].size(); ++j) {  // Iterate over columns
                        // If it's not the last column, print with a comma
                        if (j != table.second[i].size() - 1) {
                            if (std::holds_alternative<string>(table.second[i][j])) {
                                cout << std::get<string>(table.second[i][j]) + ",";
                                data_row = data_row + std::get<string>(table.second[i][j]) + "," ;
                            } else if (std::holds_alternative<int>(table.second[i][j])) {
                                string change_int_data_to_string ;
                                change_int_data_to_string = to_string(std::get<int>(table.second[i][j]));
                                cout << change_int_data_to_string << ",";
                                data_row = data_row + change_int_data_to_string + ",";
                            }
                        }
                        // For the last column, print without a comma
                        else {
                            if (std::holds_alternative<string>(table.second[i][j])) {
                                cout << std::get<string>(table.second[i][j]);
                                data_row = data_row + std::get<string>(table.second[i][j]) ;
                            } else if (std::holds_alternative<int>(table.second[i][j])) {
                                string change_int_data_to_string ;
                                change_int_data_to_string = to_string(std::get<int>(table.second[i][j]));
                                cout << change_int_data_to_string << ",";
                                data_row = data_row + change_int_data_to_string  + ",";
                            }
                        }
                    }
                    processed_command_line_outputs.push_back(data_row);
                    data_row.clear();
                    cout << endl;  // New line after each row
                }
            }
        }
    }


    if (regex_search(line, m, insert_command)) {
    regex table_name_regex(R"(INSERT INTO (\w+))");  // Regex to capture the table name
    smatch table_name_match;

        if (regex_search(line, table_name_match, table_name_regex)) {
            string table_name = table_name_match[1].str();
            auto table_iter = find_if(tables.begin(), tables.end(), 
                                    [&table_name](const pair<string, vector<vector<variant<int, string>>>>& table) {
                                        return table.first == table_name;
                                    });

            if (table_iter != tables.end()) {
                auto& table_rows = table_iter->second;  // Access rows of the correct table

                // Extract the part after 'VALUES' using regex
                regex values_regex(R"(VALUES\s+\(([^)]+)\))");  // Matches the values after 'VALUES ( ... )'
                smatch values_match;

                if (regex_search(line, values_match, values_regex)) {
                    string values_str = values_match[1].str();  // Get the matched values string

                    // Split the values by commas and trim extra spaces
                    stringstream ss(values_str);
                    string value;

                    // add values to the table's rows
                    vector<variant<int, string>> row_data;  // Vector to store a single row of data
                    while (getline(ss, value, ',')) {
                        // Check if the value contains a space to decide if it's a string
                        if (value.find(" ") != string::npos || value.front() == '\'' || value.back() == '\'') {
                            // for string value (may contain spaces or be wrapped in quotes)
                            row_data.push_back(value.substr(1, value.size() - 2));  // Remove quotes from string , as 1 here is "
                        } else {
                            // for int value
                            row_data.push_back(stoi(value));  // Convert to int and add to the row
                        }
                    }

                    // Insert the row directly into the table's rows
                    table_rows.push_back(row_data);
                }
            }
        }
    }

}


// void print_tables() {
//     for (const auto& table : tables) {
//         //cout << "Table: " << table.first << endl;  // Print table name
//         for (const auto& header_row : table.second) {
//             cout << "Headers: ";
//             for (auto it = header_row.begin(); it != header_row.end(); ++it) {
//                 if (std::holds_alternative<string>(*it)) {
//                     cout << std::get<string>(*it);
//                     if (it != header_row.end() - 1) {  // Check if it's not the last column
//                         cout << ", ";
//                     }
//                 }

//             }
//             cout << endl;
//         }
//     }
// }







