#include <fstream>
#include <iostream>

#include "slre.h"

using namespace std;

extern const char *prologue;

static void usage(const char *progname) {
    cout << "Usage:" << endl;
    cout << endl;
    cout << progname << " PROG_FILE" << endl;
    cout << "PROG_FILE - C source to protect" << endl;
}

static const char *FUNC_DEF = "^\\s*void\\s*(\\S+)\\s*\\([^)]*\\)\\s*{\\s*$";
static const char *VAR_DEF= "^\\s*(char|short|long|int|float|double)\\s+[\\S]+\\s*.*;\\s*$";
static const char *EMPTY_LINE = "^\\s*$";
static const char *BLOCK_BEGIN = "^.*{\\s*$";
static const char *BLOCK_END = "^\\s*}.*?\\s*$";
static const char *RETURN = "^\\s*return\\s*;\\s*$";

#define TRACE       ""

static int protect_file(ifstream &in, ofstream &out) {
    size_t nesting = 0;
    bool var_block = false;
    string line;

    // Write prologue of internally used code
    out << prologue;

    while (getline(in, line)) {
        if (slre_match(EMPTY_LINE, line.c_str(), line.length(), NULL, 0, 0) >= 0) {
            out << line << endl;
        // Check if current line is a function definition
        } else if (slre_match(FUNC_DEF, line.c_str(), line.length(), NULL, 0, 0) >= 0) {
            nesting++;
            var_block = true;

            out << line << endl;

            // Emit first canary
            out << "\tunsigned int __canary1 = __CANARY;" << TRACE << endl;

        } else if (slre_match(VAR_DEF, line.c_str(), line.length(), NULL, 0, 0) >= 0) {
            out << line << endl;
        } else {
            if (var_block) {
                // Variable defs done, emit second canary
                out << "\tunsigned int __canary2 = __CANARY;" << TRACE << endl;
                var_block = false;
            }

            if (slre_match(BLOCK_BEGIN, line.c_str(), line.length(), NULL, 0, 0) >= 0) {
                nesting++;
                out << line << endl;
            } else if (slre_match(BLOCK_END, line.c_str(), line.length(), NULL, 0, 0) >= 0) {
                if (!--nesting)
                    out << "\t__STACK_CHK();" << TRACE << endl;

                out << line << endl;
            } else if (slre_match(RETURN, line.c_str(), line.length(), NULL, 0, 0) >= 0) {
                out << "\t__STACK_CHK();" << TRACE << endl;
                out << line << endl;
            } else {
                out << line << endl;
            }
        }

    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    }

    string out_filename = "protected_" + string(argv[1]);
    cerr << "Writing result to " << out_filename << endl;

    ifstream in(argv[1]);
    ofstream out(out_filename.c_str());

    if (!in.is_open() || !out.is_open()) {
        cerr << "Failed to open input or output file." << endl;
        return 1;
    }

    int ret = protect_file(in, out);

    in.close();
    out.close();

    return ret;
}
