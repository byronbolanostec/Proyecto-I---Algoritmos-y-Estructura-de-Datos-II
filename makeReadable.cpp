#include <iostream>
#include <cstdio>
#include <string>
#include <cstdint>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Uso: makeReadable <input.bin> <output.txt>\n";
        return 1;
    }

    string input = argv[1];
    string output = argv[2];

    FILE* a = fopen(input.c_str(), "rb");
    if (!a) {
        cerr << "Error: no se pudo abrir el archivo binario.\n";
        return 1;
    }

    FILE* b = fopen(output.c_str(), "w");
    if (!b) {
        cerr << "Error: no se pudo crear el archivo txt.\n";
        fclose(a);
        return 1;
    }

    int32_t x;
    bool first = true;

    while (fread(&x, sizeof(int32_t), 1, a) == 1) {
        if (!first) fprintf(b, ",");
        fprintf(b, "%d", x);
        first = false;
    }

    fclose(a);
    fclose(b);

    cout << "Archivo legible generado correctamente.\n";
    return 0;
}