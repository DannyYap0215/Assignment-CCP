// *******************
// Program: Light_Mariadb_Interpreter.cpp
// Course: CCP6114 Programming Fundamentals
// Lecture Class: TC1L
// Tutorial Class: TT1L
// Trimester: 2430
// Member_1: 242UC244KX | YAP CHI YI | YAP.CHI.YI@student.mmu.edu.my | 0182694514
// Member_2: 242UC244S6 | THAM MEI TING | THAM.MEI.TING@student.mmu.edu.my | 0173268006
// Member_3: 242UC244PF | TAN YONG XIN | TAN.YONG.XIN@student.edu.mmu.my | 0126556505
// Member_4: ID | NAME | EMAIL | PHONE
// *******************
// Task Distribution
// Member_1: function: read_file, get_output_filename, write_to_file, process_command_line, split | Commands function: CREATE TABLE, TABLES, SELECT * FROM
// Member_2: Command function: UPDATE, SELECT COUNT， DATABASE
// Member_3: Command function: INSERT INTO, DELETE
// Member_4:
// *******************

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
void read_file(const string& filename);                                         // function to read input file
void write_to_file(const vector<string>& lines, const string& output_filename); // functions to write output to output file
string get_output_filename(const string& filename);                             // function to get the name for creating output file
void process_command_line(const string& line, const string& current_database);  // check for command line
void split(const string& s, char delim, vector<string>& splitted);                // functions to split strings with ","


vector<string> processed_command_line_outputs;                     // a vector of strings containing processed lines after reading in files
vector<pair<string, vector<vector<variant<int, string>>>>> tables; // a vector containing {string which is tablename,{2d vector data which could contain either int or string}}

const int MAX_COLUMN = 10;

int main() {
    string current_directory = filesystem::current_path().string();  // Get current directory to current folder
    string filename = "fileInput3.mdb"; //choose a input file to be inserted

    string input_filename = filesystem::current_path().string() + "/Database/" + filename; //get directory to input file
    read_file(input_filename); // insert directory of input file to read_file function

    return 0;
}

// Function to read the input file
void read_file(const string& filename) {
    ifstream file(filename);
    string output_filename = get_output_filename(filename); //get out_filename through inserting inputfilename into get_output_filename function

    if (file.is_open()) { //opens input file
        string line;
        string command_buffer;  // temporarly accumulate multiple line commands for multiple line commands purpose
        bool inside_create_table = false;  // track multiple line commands

        while (getline(file, line)) {
            if (!line.empty()) { 
                if (inside_create_table){ //this is only for CREATE TABLE AND INSERT INTO case where multiple lined commands is present
                    cout <<line << endl;  //bassically if no command type are found ,">" will not be added
                    processed_command_line_outputs.push_back(line);
                } else{                   // else for other commands just add ">" as intended
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
            // if no multiple lined commands present just insert each line to read for command through process_command_line
            process_command_line(line, filename);
        }

        file.close();

        string output_file_path = filesystem::current_path().string() + "/Database/" + output_filename; //path to output file
        write_to_file(processed_command_line_outputs, output_file_path); // write processed_command_line_outputs which contain processed lines into our output file

    } else {
        cout << "File not found: " << filename << endl;
    }
}


// function to get the output filename
string get_output_filename(const string& filename) {
    ifstream file(filename); //opens input file
    string line;
    while (getline(file, line)) { //read through input and find lines where CREATE is present but TABLE is not present to avoid clashing with CREATE TABLE
        if (line.find("CREATE ") == 0 && line.find("TABLE") == string::npos && line.back() == ';') { // make sure ";" is the last element
            string output_filename = line.substr(7);  // Remove the "CREATE " part
            if (output_filename.back() == ';') { //replace ';' with nothing , essentialy removing ';' from the line
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

    if (output_file.is_open()) { //open output file
        for (const auto& line : lines) { //insert all lines into output file
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
    regex delete_command(R"(DELETE\s+FROM\s+(\w+)\s+WHERE\s+(\w+)\s*=\s*'?(.*?)'?\s*;)"); // TAN YONG XIN
    regex select_count_command(R"(SELECT\s+COUNT\(\*\)\s+FROM\s+(\w+);)"); // THAM MEI TING
    regex tables_command("(TABLES;)"); // YAP CHI YI
    regex select_all_from_command(R"(SELECT\s+\*\s+FROM\s+(\w+);)"); // YAP CHI YI
    //basic pattern used :
    // * means zero or more
    // + is one or more
    // need to use R as we have \ here
    // ? is optional

    string table_name;

    smatch m; // m is lines that is matched

    // command for updating data in the tables
    if (regex_match(line, m, update_command)){
        string table_name = m[1].str();        // extract table name
        string set_column = m[2].str();        // extract column to be updated
        string new_value = m[3].str();         // extract new value to set
        string condition_column = m[4].str();  // extract the column used in WHERE condiion
        string condition_value = m[5].str();   // extract the value used in WHERE condition

        // find table in tables vector
        auto& table = tables[0].second;
        auto& headers = table[0]; // get table headers (column names)
        int set_col_index ; // index of column to be updated
        int cond_col_index ; // index of column used in WHERE condition

        // find i for set column and condition column
        for (int i = 0; i < headers.size(); ++i){
            if (get<string>(headers[i]) == set_column) set_col_index = i; // matches with column to be updated
            if (get<string>(headers[i]) == condition_column) cond_col_index = i; // matches with condition column
        }

        int updated_rows = 0; // counter of row updates
        for (auto& row : table){
            if (holds_alternative<string>(row[cond_col_index])){ // check if condition column contain string
                if (get<string>(row[cond_col_index]) == condition_value){ // comparison
                    row[set_col_index] = new_value; // condition matches, update the value in specified column
                    ++updated_rows;
                }
            } else if (holds_alternative<int>(row[cond_col_index])){ // check if condition contain integer
                try {
                    int cond_value_as_int = stoi(condition_value); // convert condition value to integer
                    if (get<int>(row[cond_col_index]) == cond_value_as_int){ // comparison
                        row[set_col_index] = new_value; // condition matches, update the value in specified column
                        ++updated_rows;
                    }
                } catch (invalid_argument& e){ // not valid, exit fucntion
                    return;
                }
            }
        }
    }

    //command to get path to input file
    if (regex_search(line, m, databases_command)) {
        char full_path[FILENAME_MAX];    // full_path is an array which contains FILENAME_MAX (the max length of a file)
        _fullpath(full_path, (current_database).c_str(), FILENAME_MAX);   // (gets abs full path using _fullpath function (current_database).c_str() is the relative path to the database ,the max length of a file)
        string database_path = full_path; // full path converted to a string called database_path
        cout << database_path << endl;
        processed_command_line_outputs.push_back(database_path);
        //so what this means that a string starting from the first character of current_database 
        //is stored in full_path with the capacity of FILENAME_MAX
    }

    // command to both create the table_name and get the headers for making the table
    if (regex_search(line, m, create_command)) {
        if (line.find("CREATE TABLE ") == 0) {
            table_name = line.substr(13, line.find('(') - 13); //get the table name which starts from index 13 and end before '('

            // Extract the columns
            //extracts the substring between the parentheses. The + 1 skips the opening parenthesis,
            //and the - 1 excludes the closing parenthesis
            string columns_str = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);
            regex column_regex(R"((\w+)\s+(\w+))");
            
            // column_begin is the first match found, column_end is the last match found
            auto column_begin = sregex_iterator(columns_str.begin(), columns_str.end(), column_regex); //column_regex is the pattern needed to match
            auto column_end = sregex_iterator();

            vector<variant<int, string>> headers; //create a vector which could contain either int or string just for headers
            for (auto i = column_begin; i != column_end; ++i) {
                string column_name = (*i)[1].str(); //(*it)[1].str() extract column name
                string column_type = (*i)[2].str(); //(*it)[2].str() extract column type // no use
                headers.push_back(column_name);  // insert name of each columns into the headers vector
            }

            // Add the table to the 'tables' vector
            tables.push_back({table_name, {headers}}); //add the table_name and headers into tables
        }
    }

    //command to get the rows of data in the table
    if (regex_search(line, m, select_count_command)){
        string table_name = m[1]; // extract table name
        int row_count = 0; // initialize row counter

        for (const auto& table : tables) { // find the matching table
            if (table.first == table_name) {
                // loop the rows in the table and count them
                for (size_t i = 1; i < table.second.size(); ++i){
                    row_count++;
                }

            }
        }
        cout << row_count << endl; // print out total number of rows
        processed_command_line_outputs.push_back(to_string(row_count)); // add row count to processed output list
    }

    //command to get the table name
    if (regex_search(line, m, tables_command)) {
        for (const auto& table : tables) {
            cout <<table.first << endl;
            processed_command_line_outputs.push_back(table.first); //insert the table name
        }
    }

    //command to print out tables data
    if (regex_search(line, m, select_all_from_command)) {
        string table_name = m[1];  // Capture the table name from the SELECT query
        string header_row;
        string data_row;

        for (const auto& table : tables) {
            //**for now there are not going to be multiple tables when running the program**
            if (table.first == table_name) {  // Match the table name and access the specific table,
                // Print column headers
                for (int i = 0; i < table.second[0].size(); ++i) { // i = 0 here access the header row
                    if (holds_alternative<string>(table.second[0][i])) {
                        if (i != table.second[0].size() - 1) {
                            cout << get<string>(table.second[0][i]) + ",";
                            header_row = header_row + get<string>(table.second[0][i]) + "," ;

                        } else { // check if the header is the last one, if yes dont add "," to it
                            cout << get<string>(table.second[0][i]);
                            header_row = header_row + get<string>(table.second[0][i])  ;
                        }
                    }
                }
                processed_command_line_outputs.push_back(header_row); // add the headers into vector = processed_command_line_outputs
                cout << endl;

                // Print rows of data
                for (int i = 1; i < table.second.size(); ++i) {  // starts from row 1 as row 0 here is the header
                    for (int j = 0; j < table.second[i].size(); ++j) {  // Iterate over columns
                        // If its not the last column, print with a comma
                        // (j != table.second[i].size() - 1) checks if its the last row
                        if (j != table.second[i].size() - 1) { // for loop to add each lines of row into a string called data_row
                            if (holds_alternative<string>(table.second[i][j])) {  // checks if string
                                cout << get<string>(table.second[i][j]) + ",";
                                data_row = data_row + get<string>(table.second[i][j]) + "," ; //add the output into data_row containing strings
                            } else if (holds_alternative<int>(table.second[i][j])) { //checks if its an int data change it to string and add to data_row
                                string change_int_data_to_string ;
                                change_int_data_to_string = to_string(get<int>(table.second[i][j]));
                                cout << change_int_data_to_string << ",";
                                data_row = data_row + change_int_data_to_string + ",";
                            }
                        }
                        // For the last column, print without a comma
                        else {
                            if (holds_alternative<string>(table.second[i][j])) {
                                cout << get<string>(table.second[i][j]);
                                data_row = data_row + get<string>(table.second[i][j]) ;
                            } else if (holds_alternative<int>(table.second[i][j])) {
                                string change_int_data_to_string ;
                                change_int_data_to_string = to_string(get<int>(table.second[i][j]));
                                cout << change_int_data_to_string ;
                                data_row = data_row + change_int_data_to_string ;
                            }
                        }
                    }
                    processed_command_line_outputs.push_back(data_row); //adds the string into vector = processed_command_line_outputs
                    data_row.clear(); // clear the data of data_row
                    cout << endl;  // New line after each row
                }
            }
        }
    }

    //command to add data into tables
    if (regex_search(line, m, insert_command)) {
    regex table_name_regex(R"(INSERT INTO (\w+))");  // Regex to capture the table name and access the specifc table data
    smatch table_name_match;

        if (regex_search(line, table_name_match, table_name_regex)) {
            string table_name = table_name_match[1].str(); // table_name_match[1].str() refer to "customers"; if table_name_match[0].str() then it refers to "INSERT INTO customers"
        
            for (const auto& table : tables) { //iterate through table in tables
                if ( table_name == table.first) { //check if table_name for inserting is same as in data
                    auto& table_rows = tables[0].second;  // access the row ** using [0] as this program does not implement many tables yet ** may do in future -21/1/2025

                    // extract part after 'VALUES' using regex
                    regex values_regex(R"(VALUES\s+\(([^)]+)\))");  
                    smatch values_match;

                    if (regex_search(line, values_match, values_regex)) {
                        string values_str = values_match[1].str();  // Get the matched values string

                        vector<string> values_tokens;
                        // instead of stringstream we now changed to using split function - 23/1/2025
                        split(values_str, ',', values_tokens); // instead of stringstream we now changed to using split function - 23/1/2025

                        // add values to the table's rows
                        vector<variant<int, string>> row_data;  // row_data to store all data 
                        for (const string& value : values_tokens) {
                            // Check if the value is a string (contains spaces or is enclosed in quotes)
                            if (value.find(" ") != string::npos || value.front() == '\'' || value.back() == '\'') {
                                // Remove quotes from string value and push_back into row_data
                                row_data.push_back(value.substr(1, value.size() - 2));  // Remove quotes
                            } else {
                                // Convert to int and add to the row
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

    if (regex_search(line, m, delete_command)) {
    string table_name = m[1].str();
    string condition_column = m[2].str();
    string condition_value = m[3].str();

    // match the table in tables
    for (auto& table_entry : tables) {
        if (table_name == table_entry.first) { // get the table name
            auto& table = table_entry.second;  // get the table rows
            auto& headers = table[0]; // header row
            int cond_col_index ;

            // Find the index of the condition column
            for (int i = 0; i < headers.size(); ++i) {
                if (holds_alternative<string>(headers[i]) &&
                    get<string>(headers[i]) == condition_column) {
                    cond_col_index = i;
                    break;
                }
            }

            for (int i = 1; i < table.size(); ++i) {  // skip header row
                auto& row = table[i];  //access the row of table
                bool should_delete = false; //used to check if our condition value is the same as or value inside our row 

                // check if its int or string
                if (holds_alternative<string>(row[cond_col_index])) {
                    //if the condition is the same as our value in our row it sets should_delete to true 
                    //if condition is not met, do nothing
                    if (get<string>(row[cond_col_index]) == condition_value){
                        should_delete = true;
                    };
                    
                } else if (holds_alternative<int>(row[cond_col_index])) {
                    //same as the first if, but change int to str for comparison
                    int cond_value_as_int = stoi(condition_value); // Convert to int
                    if (get<int>(row[cond_col_index]) == cond_value_as_int){
                        should_delete = true;
                    };
                }

                // delete row if condition is met
                if (should_delete) {
                    //table.begin() is indexed at 0
                    // + i here means we want to delete row at 0 + i.
                    //Example, i could be 3 which means we delete row 3
                    table.erase(table.begin() + i);  // Delete the current row
                }
            }
        }
    }

    }
}

// functions to split strings with "," 
void split(const string& s, char delim, vector<string>& splitted) {
    int startPos = 0;  // Starting position 
    int delimPos = s.find(delim); // pos of ","

    // If delimiters is found 
    while (delimPos != string::npos) {
        string s_without_delim = s.substr(startPos, delimPos - startPos);
        splitted.push_back(s_without_delim);

        // move delimPos up by 1 
        delimPos++;

        // starting position is now delimPos + 1 as delimPos++;
        startPos = delimPos;

        // starts at delimPos which is the "," that we found and from that proceed to find the next ","
        delimPos = s.find(delim, delimPos); 

        //last values with no delimiter = "," found
        if (delimPos == string::npos) {
            string s_without_delim = s.substr(startPos, delimPos - startPos);
            splitted.push_back(s_without_delim);
        }
    }
}
