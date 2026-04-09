#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <cstdint>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cerr << "Uso: generator -size <SMALL|MEDIUM|LARGE> -output <archivo.bin>\n";
        return 1;
    }

    string size, output;

    for (int i = 1; i < argc; i += 2) {
        string key = argv[i];
        string value = argv[i + 1];

        if (key == "-size") size = value;
        else if (key == "-output") output = value;
        else {
            cerr << "Uso: generator -size <SMALL|MEDIUM|LARGE> -output <archivo.bin>\n";
            return 1;
        }
    }

    if (size.empty() || output.empty()) {
        cerr << "Argumentos invalidos\n";
        return 1;
    }

    const uint64_t KB = 1024ULL;
    const uint64_t MB = 1024ULL * KB;

    uint64_t bytes = 0;
    if (size == "SMALL") bytes = 32ULL * MB;
    else if (size == "MEDIUM") bytes = 64ULL * MB;
    else if (size == "LARGE") bytes = 128ULL * MB;
    else {
        cerr << "Tamano invalido\n";
        return 1;
    }

    const uint64_t totalInts = bytes / sizeof(int32_t);

    ofstream file(output, ios::binary);
    if (!file) {
        cerr << "No se pudo abrir el archivo\n";
        return 1;
    }

    mt19937 gen(random_device{}());
    uniform_int_distribution<int32_t> dist(-1000000, 1000000);

    const uint64_t CHUNK_INTS = 1ULL << 20;
    int32_t* buffer = new int32_t[CHUNK_INTS];

    uint64_t written = 0;
    while (written < totalInts) {
        uint64_t cnt = CHUNK_INTS;
        if (cnt > totalInts - written) cnt = totalInts - written;

        for (uint64_t i = 0; i < cnt; i++) buffer[i] = dist(gen);

        file.write(reinterpret_cast<char*>(buffer), static_cast<streamsize>(cnt * sizeof(int32_t)));
        if (!file) {
            delete[] buffer;
            cerr << "Error escribiendo el archivo\n";
            return 1;
        }

        written += cnt;
    }

    delete[] buffer;
    file.close();

    cout << "Archivo generado: " << output << "\n";
    cout << "Tamano: " << bytes / MB << " MB\n";
    cout << "Enteros: " << totalInts << "\n";
    return 0;
}