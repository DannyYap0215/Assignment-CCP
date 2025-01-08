#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <variant>

using namespace std;

// Define the global tables structure
vector<vector<variant<string, vector<variant<int, string>>>>> tables;

// Function declarations
void CREATE(const string&, ofstream&);
void CREATE_TABLE(const string&, ifstream&, ofstream&);

int main() {
    ifstream inputFile("fileInput1.mdb"); // Open the .mdb input file
    ofstream outputFile;
    string line;

    // Read lines from the .mdb input file
    while (getline(inputFile, line)) {
        if (line.find("CREATE ") == 0 && line.find("TABLE") == string::npos && line.back() == ';') {
            CREATE(line, outputFile);
        } else if (line.find("CREATE TABLE") == 0) {
            CREATE_TABLE(line, inputFile, outputFile);
        }
    }

    inputFile.close();
    outputFile.close(); // Close the file when done
    return 0;
}

// CREATE function to create the file
void CREATE(const string& command, ofstream& outputFile) {
    string filename = command.substr(7); // Remove the "CREATE " part

    // Remove the semicolon at the end
    if (filename.back() == ';') {
        filename.pop_back();
    }

    // Create the file with the name filename
    outputFile.open(filename);
    if (outputFile) {
        outputFile << "> " << command << endl;
        cout << "> " << command << endl;
    } else {
        cerr << "Error: Unable to create file." << endl;
    }
}

// CREATE_TABLE function to create and store table structure
void CREATE_TABLE(const string& command, ifstream& inputFile, ofstream& outputFile) {
    string tablename = command.substr(13); // Extract table name
    if (tablename.back() == '(') {
        tablename.pop_back(); // Remove '(' at the end if present
    }

    // Remove leading and trailing spaces
    tablename.erase(0, tablename.find_first_not_of(" "));
    tablename.erase(tablename.find_last_not_of(" ") + 1);

    // Add table name to tables
    tables.push_back({tablename});

    string line;
    vector<variant<int, string>> headers;

    // Read lines until the closing parenthesis is found
    while (getline(inputFile, line)) {
        if (line.find(");") != string::npos) {
            break; // Stop reading when ")" is found
        }

        // Extract column names and types using regex
        regex column_regex(R"((\w+)\s+(INT|TEXT))");
        smatch match;
        if (regex_search(line, match, column_regex)) {
            string column_name = match[1].str();
            string column_type = match[2].str();

            headers.push_back(column_name); // Add column name

            // Output the column creation process
            outputFile << "> Column: " << column_name << " Type: " << column_type << endl;
            cout << "> Column: " << column_name << " Type: " << column_type << endl;
        }
    }

    // Add headers to the table
    tables.back().push_back(headers);

    // Output the table creation confirmation
    outputFile << "> Table '" << tablename << "' created with " << headers.size() << " columns." << endl;
    cout << "> Table '" << tablename << "' created with " << headers.size() << " columns." << endl;
}
