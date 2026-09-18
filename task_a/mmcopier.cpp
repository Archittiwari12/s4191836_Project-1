#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <stdexcept>

using namespace std;

struct ThreadArgs {
    int fileNumber;
    string sourceDir;
    string destinationDir;
};

void* copyFile(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);

    int fileNumber = args->fileNumber;
    string sourceDir = args->sourceDir;
    string destinationDir = args->destinationDir;

    string sourceFileName =
        sourceDir + "/source" + to_string(fileNumber) + ".txt";

    string destinationFileName =
        destinationDir + "/destination" + to_string(fileNumber) + ".txt";

    delete args;

    ifstream sourceFile(sourceFileName);

    if (!sourceFile.is_open()) {
        cerr << "Error: Could not open source file: "
             << sourceFileName << endl;
        return nullptr;
    }

    ofstream destinationFile(destinationFileName);

    if (!destinationFile.is_open()) {
        cerr << "Error: Could not open destination file: "
             << destinationFileName << endl;

        sourceFile.close();
        return nullptr;
    }

    string line;

    while (getline(sourceFile, line)) {
        destinationFile << line << '\n';

        if (!destinationFile) {
            cerr << "Error: Failed to write to destination file: "
                 << destinationFileName << endl;

            sourceFile.close();
            destinationFile.close();

            return nullptr;
        }
    }

    sourceFile.close();
    destinationFile.close();

    return nullptr;
}

int main(int argc, char* argv[]) {

    if (argc != 4) {
        cerr << "Usage: ./mmcopier n source_dir destination_dir"
             << endl;
        return 1;
    }

    int numSources;

    try {
        size_t position;

        numSources = stoi(argv[1], &position);

        if (position != string(argv[1]).length()) {
            cerr << "Error: n must be an integer." << endl;
            return 1;
        }
    }
    catch (const invalid_argument&) {
        cerr << "Error: n must be an integer." << endl;
        return 1;
    }
    catch (const out_of_range&) {
        cerr << "Error: n is too large." << endl;
        return 1;
    }

    if (numSources < 2 || numSources > 10) {
        cerr << "Error: n must be between 2 and 10." << endl;
        return 1;
    }

    string sourceDir = argv[2];
    string destinationDir = argv[3];

    // Maximum of 10 source files are allowed.
    pthread_t threads[10];

    // Create one thread for each source file.
    for (int i = 0; i < numSources; ++i) {

        ThreadArgs* args = new ThreadArgs;

        args->fileNumber = i + 1;
        args->sourceDir = sourceDir;
        args->destinationDir = destinationDir;

        int result = pthread_create(
            &threads[i],
            nullptr,
            copyFile,
            args
        );

        if (result != 0) {
            cerr << "Error: Failed to create thread "
                 << i + 1 << endl;

            delete args;

            // Wait for threads that were already created.
            for (int j = 0; j < i; ++j) {
                pthread_join(threads[j], nullptr);
            }

            return 1;
        }
    }

    // Wait for all threads to finish.
    for (int i = 0; i < numSources; ++i) {

        int result = pthread_join(
            threads[i],
            nullptr
        );

        if (result != 0) {
            cerr << "Error: Failed to join thread "
                 << i + 1 << endl;
            return 1;
        }
    }

    return 0;
}