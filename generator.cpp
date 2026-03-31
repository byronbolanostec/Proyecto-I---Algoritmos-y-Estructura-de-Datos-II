#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <cstdint>

using namespace std;

int main(int argc, char* argv[]) {
    string size = argv[2];
    string output = argv[4];

    uint64_t bytes = 0;
    
    const uint64_t KB = 1024ULL;
    const uint64_t MB = 1024ULL * KB;
    const uint64_t GB = 1024ULL * MB;

    if (size == "SMALL") bytes = (512ULL * MB) / 2;
    else if (size == "MEDIUM") bytes = (1ULL * GB) / 2;
    else if (size == "LARGE") bytes = (2ULL * GB) / 2;
    else {
        cerr << "Tamano invalido\n";
        return 1;
    }

    uint64_t totalInts = bytes / 4;

    ofstream file(output, ios::binary);
    if (!file) {
        cerr << "No se pudo abrir el archivo\n";
        return 1;
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int32_t> dist(-1000000, 1000000);

    int32_t num;
    for (uint64_t i = 0; i < totalInts; i++) {
        num = dist(gen);
        file.write(reinterpret_cast<char*>(&num), sizeof(num));
    }

    file.close();
    cout << "Archivo generado\n";
    return 0;
}