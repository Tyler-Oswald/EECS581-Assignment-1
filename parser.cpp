/**
Author: Tyler Oswald
Outside sources: Claude Sonnet 5 medium
Inputs: A string that may or may not contain an ip address with or without a port number
Outputs: A message stating if a valid ip address was found and some data about it
**/
#include <iostream>
#include <string>

using namespace std;

//All function below this were written by Claude with no edits from the author 
//All AI generated comments were removed 
//The author wrote all comments for this code 

//Function to check if a char is a digit 0-9
bool isDigitChar(char c) {
    return c >= '0' && c <= '9';
}

//Function to check if a char is a valid token char
bool isValidTokenChar(char c) {
    return isDigitChar(c) || c == '.' || c == ':';
}

//Function to parse a string for numbers
bool parseNumberManual(const string& s, size_t start, size_t end,
                        int minLen, int maxLen, long maxValue,
                        long& valueOut) {
    //Check for invalid bounds                        
    if (end < start) return false;
    size_t len = end - start;
    if (len < static_cast<size_t>(minLen) || len > static_cast<size_t>(maxLen)) {
        return false;
    }
    //Check for invalid 0's in the number like 192.01.1.1
    if (len > 1 && s[start] == '0') {
        return false; 
    }

    long value = 0;
    //Loop to accumulate numbers 
    for (size_t i = start; i < end; i++) {
        if (!isDigitChar(s[i])) {
            return false;
        }
        value = value * 10 + (s[i] - '0');
        //Check if this IP address would be large than possible
        if (value > maxValue) {
            return false; // early exit
        }
    }
    valueOut = value;
    return true;
}

//Convert number 0-255 back to a string
string octetToStringManual(unsigned int value) {
    if (value == 0) {
        return "0";
    }
    string result;
    while (value > 0) {
        int digit = static_cast<int>(value % 10);
        result.push_back(static_cast<char>('0' + digit));
        value /= 10;
    }
    //reverse the string
    size_t a = 0, b = result.size() - 1;
    while (a < b) {
        char tmp = result[a];
        result[a] = result[b];
        result[b] = tmp;
        a++;
        b--;
    }
    return result;
}

//Returns true if a valid address was found, false otherwise
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) {
    outAddress = 0;
    outPort = -1;

    size_t i = 0;
    size_t n = str.size();

    while (i < n) {
        //Skip garbage chars 
        while (i < n && !isValidTokenChar(str[i])) {
            i++;
        }
        size_t tokenStart = i;
        //Keep adding chars until we run out of valid chars or if we hit the end of the string
        while (i < n && isValidTokenChar(str[i])) {
            i++;
        }
        size_t tokenEnd = i; 
        //Case for end of input string
        if (tokenStart == tokenEnd) {
            break;
        }

        //Block for finding colons in a token
        long colonPos = -1;
        int colonCount = 0;
        //Loop through the token to find a colon
        for (size_t k = tokenStart; k < tokenEnd; k++) {
            if (str[k] == ':') {
                //Increment the number of colons in this token
                colonCount++;
                //set the position of the found colon
                colonPos = static_cast<long>(k);
            }
        }
        //If we have found more than 1 colon in a token, then this token is garbage
        if (colonCount > 1) {
            //Continue looking for valid tokens
            continue;
        }
        //If there is a valid colon, then the end is the colon position.
        //Otherwise the end is the end of the token
        size_t ipEnd = (colonCount == 1) ? static_cast<size_t>(colonPos) : tokenEnd;
        //The port starts at colon position unless there is no colon
        size_t portStart = (colonCount == 1) ? static_cast<size_t>(colonPos) + 1 : 0;
        size_t portEnd = tokenEnd;
        //Check for colon but no port number
        if (colonCount == 1 && portStart >= portEnd) {
            //No port number mean this token is garbage
            continue; 
        }

        //Find the three .'s for octets 
        size_t dotPositions[3];
        int dotCount = 0;
        bool badChar = false;
        //Loop over the token
        for (size_t k = tokenStart; k < ipEnd; k++) {
            char c = str[k];
            if (c == '.') {
                //If there is more than 3 .'s this is garbage token
                if (dotCount >= 3) {
                    badChar = true;
                    break;
                }
                dotPositions[dotCount++] = k;
            } 
            //If something other than a digit or a . is present then this is garbage
            else if (!isDigitChar(c)) {
                badChar = true;
                break;
            }
        }
        //Garbage token 
        if (badChar || dotCount != 3) {
            continue;
        }
        //Build segments of address from the positions we have collected
        size_t segStart[4], segEnd[4];
        segStart[0] = tokenStart;          segEnd[0] = dotPositions[0];
        segStart[1] = dotPositions[0] + 1; segEnd[1] = dotPositions[1];
        segStart[2] = dotPositions[1] + 1; segEnd[2] = dotPositions[2];
        segStart[3] = dotPositions[2] + 1; segEnd[3] = ipEnd;

        long octetVal[4];
        bool ok = true;
        for (int s = 0; s < 4; s++) {
            long v = 0;
            //Check if this octet forms a valid number
            if (!parseNumberManual(str, segStart[s], segEnd[s], 1, 3, 255, v)) {
                ok = false;
                break;
            }
            //Set the value for this octet
            octetVal[s] = v;
        }
        //If one of the octets was not valid we need to keep looking for new ones
        if (!ok) {
            continue; 
        }

        long portVal = -1;
        //If there is a colon, we need to check for a valid port number
        if (colonCount == 1) {
            //Parse the port number and check if it is in range to be valid
            if (!parseNumberManual(str, portStart, portEnd, 1, 5, 65535, portVal)) {
                //port number is not valid so we need to reject his ip address 
                continue; 
            }
        }

        //We have made it past all the checks, so we can build octets from the values we collected
        unsigned long address =
            static_cast<unsigned long>(octetVal[0]) * 16777216UL +
            static_cast<unsigned long>(octetVal[1]) * 65536UL +
            static_cast<unsigned long>(octetVal[2]) * 256UL +
            static_cast<unsigned long>(octetVal[3]);
        //Set the found address
        outAddress = address;
        //Set the port number if we found one
        //Otherwise, set the port number to -1
        outPort = (colonCount == 1) ? static_cast<int>(portVal) : -1;
        return true;
    }
    //Case for not finding a valid address
    outAddress = 0;
    outPort = -1;
    return false;
}

//Main was generated by Claude with edits from author 
//Main edits were to adjust the input and output messages and user display
int main() {
    string line;
    //Main logic loop 
    while (true) {
        //Ask the user to enter a string
        cout << "Enter a string (or 'END' to quit): ";
        getline(cin, line);

        //Check for program exit
        if(line == "END"){
            cout << "Program terminated." << endl;
            return 0;
        }
        unsigned long address = 0;
        int port = -1;
        //Try to extract address from user input
        bool found = extractIPv4(line, address, port);

        //Case for if address was found from the input 
        if (found) {
            //Convert the returned address into 4 octets 
            unsigned int o0 = static_cast<unsigned int>((address / 16777216UL) % 256UL);
            unsigned int o1 = static_cast<unsigned int>((address / 65536UL) % 256UL);
            unsigned int o2 = static_cast<unsigned int>((address / 256UL) % 256UL);
            unsigned int o3 = static_cast<unsigned int>(address % 256UL);
            //Combind the octects with .'s to form an ip address 
            string ipStr = octetToStringManual(o0) + "." +
                           octetToStringManual(o1) + "." +
                           octetToStringManual(o2) + "." +
                           octetToStringManual(o3);

            //Display the ip address to the user
            cout << "Extracted IPv4 address: " << ipStr
                 << " (decimal value: " << address << ", port: ";
            //Check if there was a port number found
            if (port == -1) {
                cout << "none";
            } 
            else {
                cout << port;
            }
            cout << ")" << endl;
        } 
        else {
            cout << "Invalid input: no valid IPv4 address found" << endl;
        }
    }
    return 0;
}