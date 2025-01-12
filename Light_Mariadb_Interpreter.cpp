// *********************************************************
// Program: Light_Mariadb_Interpreter.cpp
// Course: CCP6114 Programming Fundamentals
// Lecture Class: TC1L
// Tutorial Class: TT1L
// Trimester: 2430
// Member_1: 242UC244KX | YAP CHI YI | YAP.CHI.YI@student.mmu.edu.my | 018-2694514
// Member_2: ID | NAME | EMAIL | PHONE
// Member_3: ID | NAME | EMAIL | PHONE
// Member_4: ID | NAME | EMAIL | PHONE
// *********************************************************
// Task Distribution
// Member_1: function: read_file, get_output_filename, write_to_file | Commands function: CREATE TABLE, TABLES, SELECT * FROM
// Member_2: 
// Member_3:
// Member_4:
// *********************************************************

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
    string filename = "fileInput2.mdb";

    string input_filename = filesystem::current_path().string() + "/Database/" + filename;
    read_file(input_filename);

    return 0;
}

// Function to read the input file
void read_file(const string& filename) {
    ifstream file(filename);
    string output_filename = get_output_filename(filename);
    
    if (file.is_open()) {
        string line;
        string command_buffer;  // temporarly accumulate multiple line commands
        bool inside_create_table = false;  // track multiple line commands

        while (getline(file, line)) {
            if (!line.empty()) {
                if (inside_create_table){ // bassically if no command type are found ,">" will not be added
                    processed_command_line_outputs.push_back(line);
                    cout <<line << endl;
                } else{ 
                    processed_command_line_outputs.push_back("> " + line); 
                    cout << "> " << line << endl;
                }
            }

            // Check if line contain CREATE TABLE command or INSERT INTO command
            if (line.find("CREATE TABLE") == 0 || line.find("INSERT INTO") == 0) {
                inside_create_table = true;
                command_buffer = line;  // Start accumulating the command
                continue;  // Skip to next if statement
            }

            // If theres multiple lines inside of CREATE TABLE command or INSERT INTO command, accumulate the lines in command buffer
            if (inside_create_table) {
                command_buffer += " " + line;  // Append line to buffer
                if (line.find(");") != string::npos) {  // Check if ");" is found
                    inside_create_table = false;  // Reset the flag to false thus stopping the command buffer
                    process_command_line(command_buffer, filename);  // Process the completed command
                    command_buffer.clear();  // Clear the buffer
                }
                continue;  
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


// essentially the CREATE command
string get_output_filename(const string& filename) {
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        if (line.find("CREATE ") == 0 && line.find("TABLE") == string::npos && line.back() == ';') {
            string output_filename = line.substr(7);  // Remove the "CREATE " part
            if (output_filename.back() == ';') {
                output_filename.pop_back();
            }
            return output_filename ;
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
    regex databases_command("(DATABASES;)"); // THAM MEI TING
    regex create_command("(CREATE TABLE)(.*)"); // YAP CHI YI
    regex insert_command("(INSERT INTO)(.*)"); // TAN YONG XIN
    regex update_command(R"(UPDATE\s+(\w+)\s+SET\s+(\w+)\s*=\s*'?(.*?)'?\s+WHERE\s+(\w+)\s*=\s*'?(.*?)'?\s*;)"); // THAM MEI TING
    regex delete_command("(DELETE)(.*)"); // TAN YONG XIN
    regex select_count_command(R"(SELECT\s+COUNT\(\*\)\s+FROM\s+(\w+);)"); // THAM MEI TING
    regex tables_command("(TABLES;)"); // YAP CHI YI
    regex select_all_from_command(R"(SELECT\s+\*\s+FROM\s+(\w+);)"); // YAP CHI YI

    string table_name;

    smatch m;

    if (regex_match(line, m, update_command)){
        string table_name = m[1].str();        // table name
        string set_column = m[2].str();        // update column
        string new_value = m[3].str();         // new value
        string condition_column = m[4].str();  // column in -where-
        string condition_value = m[5].str();   // value in -where-

        // find table in -table- vector
        auto table_it = find_if(tables.begin(), tables.end(), [&](const auto& t){
            return t.first == table_name;
        });

        if (table_it == tables.end()){ // not found, xit
            return;
        }

        auto& table = table_it->second;
        auto& headers = table[0];
        int set_col_index = -1;
        int cond_col_index = -1;

        // find i for set col and condition col
        for (int i = 0; i < headers.size(); ++i){
            if (get<string>(headers[i]) == set_column) set_col_index = i;
            if (get<string>(headers[i]) == condition_column) cond_col_index = i;
        }

        if (set_col_index == -1 || cond_col_index == -1){
            return;
        }

        int updated_rows = 0;
        for (auto& row : table){
            if (std::holds_alternative<std::string>(row[cond_col_index])){ // check
                if (std::get<std::string>(row[cond_col_index]) == condition_value){ // compare
                    row[set_col_index] = new_value; // update
                    ++updated_rows;
                }
            } else if (std::holds_alternative<int>(row[cond_col_index])){ // check
                try {
                    int cond_value_as_int = std::stoi(condition_value); // convert to integer
                    if (std::get<int>(row[cond_col_index]) == cond_value_as_int){ // compare
                        row[set_col_index] = new_value; // update
                        ++updated_rows;
                    }
                } catch (std::invalid_argument& e){ // not valid, exit
                    return;
                }
            }
        }
    }

    
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

    if (regex_search(line, m, select_count_command)){
        string table_name = m[1];
        int row_count = 0;  

        for (const auto& table : tables) { // find the table
            if (table.first == table_name) {
                // loop the rows in the table
                for (size_t i = 1; i < table.second.size(); ++i){
                    row_count++;
                }

            }
        }
        cout << row_count << endl;
        processed_command_line_outputs.push_back(to_string(row_count)); 
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
            //**for now there are not going to be multiple tables when running the program**
            if (table.first == table_name) {  // Match the table name and access the specific table, 
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
    regex table_name_regex(R"(INSERT INTO (\w+))");  // Regex to capture the table name and access the specifc table data
    smatch table_name_match; 

        if (regex_search(line, table_name_match, table_name_regex)) {
            string table_name = table_name_match[1].str(); // table_name_match[1].str() refer to "customers"; if table_name_match[0].str() then it refers to "INSERT INTO customers"
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