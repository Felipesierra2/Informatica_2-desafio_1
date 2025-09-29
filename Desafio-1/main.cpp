#include <iostream>
#include <fstream>
#include <cstdio>

using namespace std;

// --- Constantes y Prototipos ---
const long LIMITE_DE_MEMORIA = 30000000; // Límite de 30 MB, más que suficiente
const int P_k = 256;
const int P_n = 7;

// Funciones auxiliares
long mlen(const char* str);
const char* Find_P(const char* texto, const char* patron);
void copy_mem(char* destino, const char* origen, long n);

// Funciones principales
char* readfile(const char* namefile, long& sizeofile);
char* limpiarPistaBOM(char* datosPista, long& tamanoPista);

// Funciones de desencriptado separadas
char rotar_derecha(char byte, int n);
char* aplicar_xor(const char* data, long size, unsigned char k);      // Encriptado ^ clave
char* aplicar_rotacion(const char* data, long size, int n); // Sinclave rotado x veces

// Funciones de descompresión
char* desempaquetarParaRLE(const char* data, long tamanoOriginal, long& tamanoNuevo);
char* descomprimirRLE(const char* dataComprimida, long tamanoComprimido, long& tamanoSalida);
char* descomprimirLZ78(const char* dataComprimida, long tamanoComprimido, long& tamanoSalida);


// --- Función Principal ---
int main() {
    int numeroDeArchivos;
    cout << "Ingrese el numero de archivos a procesar: ";
    cin >> numeroDeArchivos;

    for (int i = 1; i <= numeroDeArchivos; i++) {
        cout << "\n\n=========================================================" << endl;
        cout << "--- Procesando Par de Archivos #" << i << " ---" << endl;
        cout << "=========================================================" << endl;

        char nombreEncriptado[30], nombrePista[30];
        sprintf(nombreEncriptado, "Encriptado%d.txt", i);
        sprintf(nombrePista, "pista%d.txt", i);

        long tamanoEncriptado = 0;
        char* datosEncriptados = readfile(nombreEncriptado, tamanoEncriptado);
        if (datosEncriptados == nullptr) continue;

        long tamanoPista = 0;
        char* datosPista_raw = readfile(nombrePista, tamanoPista);
        if (datosPista_raw == nullptr) {
            delete[] datosEncriptados;
            continue;
        }
        char* datosPista = limpiarPistaBOM(datosPista_raw, tamanoPista);

        cout << "Texto Encriptado Leido (" << tamanoEncriptado << " bytes):" << endl;
        for(int j = 0; j < 75 && j < tamanoEncriptado; ++j) cout << datosEncriptados[j];
        cout << "..." << endl;
        cout << "Pista a buscar: \"" << datosPista << "\"" << endl;

        bool claveEncontrada = false;
        for (int k = 0; k < P_k && !claveEncontrada; k++) {
            for (int n = 1; n <= P_n && !claveEncontrada; n++) {

                // Desencriptado en dos pasos
                char* buffer_sin_xor = aplicar_xor(datosEncriptados, tamanoEncriptado, k);
                char* bufferDesencriptado = aplicar_rotacion(buffer_sin_xor, tamanoEncriptado, n);
                delete[] buffer_sin_xor; // Liberamos el buffer intermedio

                // --- Intento con RLE ---
                long tamanoRLEEmpaquetado = 0;
                char* bufferRLE_listo = desempaquetarParaRLE(bufferDesencriptado, tamanoEncriptado, tamanoRLEEmpaquetado);
                if (bufferRLE_listo) {
                    long tamanoFinalRLE = 0;
                    char* bufferFinalRLE = descomprimirRLE(bufferRLE_listo, tamanoRLEEmpaquetado, tamanoFinalRLE);
                    if (bufferFinalRLE) {
                        if (Find_P(bufferFinalRLE, datosPista)) {
                            cout << "\n\n¡¡¡EXITO!!! Combinacion encontrada:" << endl;
                            cout << "  -> Metodo de Compresion: RLE" << endl;
                            cout << "  -> Clave (k): " << k << " (Hex: 0x" << hex << k << dec << ")" << endl;
                            cout << "  -> Rotacion (n): " << n << endl;
                            cout << "\n---------- MENSAJE REVELADO ----------" << endl;
                            cout << bufferFinalRLE << endl;
                            cout << "--------------------------------------" << endl;
                            claveEncontrada = true;
                        }
                        delete[] bufferFinalRLE;
                    }
                    delete[] bufferRLE_listo;
                }

                // --- Intento con LZ78 ---
                if (!claveEncontrada) {
                    long tamanoFinalLZ78 = 0;
                    char* bufferFinalLZ78 = descomprimirLZ78(bufferDesencriptado, tamanoEncriptado, tamanoFinalLZ78);
                    if (bufferFinalLZ78) {
                        if (Find_P(bufferFinalLZ78, datosPista)) {
                            cout << "\n\n¡¡¡EXITO!!! Combinacion encontrada:" << endl;
                            cout << "  -> Metodo de Compresion: LZ78" << endl;
                            cout << "  -> Clave (k): " << k << " (Hex: 0x" << hex << k << dec << ")" << endl;
                            cout << "  -> Rotacion (n): " << n << endl;
                            cout << "\n---------- MENSAJE REVELADO ----------" << endl;
                            cout << bufferFinalLZ78 << endl;
                            cout << "--------------------------------------" << endl;
                            claveEncontrada = true;
                        }
                        delete[] bufferFinalLZ78;
                    }
                }

                delete[] bufferDesencriptado;
            }
        }

        if (!claveEncontrada) {
            cout << "\nNo se pudo encontrar la combinacion para este archivo." << endl;
        }

        delete[] datosEncriptados;
        delete[] datosPista;
    }
    cout << "\n--- Proceso completado. ---" << endl;
    return 0;
}

// --- Implementación de Funciones ---


char* readfile(const char* namefile, long& sizeofile) {

    /**
 * @brief Lee el contenido completo de un archivo binario y lo carga en memoria.
 * @param namefile El nombre (ruta) del archivo a leer.
 * @param sizeofile Una referencia a una variable 'long' donde se almacenará el tamaño del archivo.
 * @return Un puntero a un nuevo buffer de memoria dinámica (char*) que contiene los datos del archivo.
 * Retorna 'nullptr' si el archivo no se puede abrir, está vacío o falla la lectura.
 * @note El llamador es responsable de liberar la memoria devuelta usando 'delete[]'.
 */

    ifstream archivo(namefile, ios::binary | ios::ate);
    if (!archivo.is_open()) { cout << "Error: no se pudo abrir " << namefile << endl; return nullptr; }
    sizeofile = archivo.tellg();
    if (sizeofile <= 0) { archivo.close(); return nullptr; }
    archivo.seekg(0, ios::beg);
    char* dataread = new char[sizeofile + 1];
    if (!archivo.read(dataread, sizeofile)) {
        archivo.close(); delete[] dataread; return nullptr;
    }
    archivo.close();
    dataread[sizeofile] = '\0';
    return dataread;
}

char* limpiarPistaBOM(char* datosPista_raw, long& tamanoPista) {

    /**
 * @brief Revisa si una cadena de texto (la pista) comienza con un Byte Order Mark (BOM) de UTF-8 y lo elimina.
 * @param datosPista_raw El buffer original de la pista leída del archivo.
 * @param tamanoPista La referencia al tamaño de la pista, que será actualizada si se elimina el BOM.
 * @return Un puntero al buffer de la pista. Si se elimina el BOM, es un nuevo puntero y la memoria original es liberada.
 * Si no hay BOM, se devuelve el puntero original.
 * @note Un BOM es una secuencia de 3 bytes invisibles (0xEF, 0xBB, 0xBF) que algunos editores añaden
 * al inicio de los archivos UTF-8 y que puede interferir con la búsqueda de cadenas.
 */
    if (tamanoPista >= 3 &&
        static_cast<unsigned char>(datosPista_raw[0]) == 0xEF &&
        static_cast<unsigned char>(datosPista_raw[1]) == 0xBB &&
        static_cast<unsigned char>(datosPista_raw[2]) == 0xBF) {

        tamanoPista -= 3;
        char* pistaLimpia = new char[tamanoPista + 1];
        copy_mem(pistaLimpia, datosPista_raw + 3, tamanoPista);
        pistaLimpia[tamanoPista] = '\0';
        delete[] datosPista_raw;
        return pistaLimpia;
    }
    return datosPista_raw;
}

char rotar_derecha(char byte, int n) {

    /**
 * @brief Rota los bits de un byte 'n' posiciones hacia la derecha.
 * @param byte El byte sobre el cual realizar la rotación.
 * @param n El número de posiciones a rotar.
 * @return El byte resultante con los bits rotados.
 */
    unsigned char u_byte = static_cast<unsigned char>(byte);
    return (u_byte >> n) | (u_byte << (8 - n));
}

char* aplicar_xor(const char* data, long size, unsigned char k) {

    /**
 * @brief Aplica una operación XOR a cada byte de un buffer de datos.
 * @param data Puntero a los datos de entrada.
 * @param size El tamaño del buffer de datos.
 * @param k La clave de 8 bits a usar para la operación XOR.
 * @return Un nuevo buffer de memoria dinámica con el resultado de la operación.
 */

    char* bufferResultado = new char[size + 1];
    for (long i = 0; i < size; i++) {
        bufferResultado[i] = data[i] ^ k;
    }
    bufferResultado[size] = '\0';
    return bufferResultado;
}

char* aplicar_rotacion(const char* data, long size, int n) {

    /**
 * @brief Aplica una rotación de bits a la derecha a cada byte de un buffer de datos.
 * @param data Puntero a los datos de entrada.
 * @param size El tamaño del buffer de datos.
 * @param n El número de bits a rotar.
 * @return Un nuevo buffer de memoria dinámica con el resultado de la operación.
 */
    char* bufferResultado = new char[size + 1];
    for (long i = 0; i < size; i++) {
        bufferResultado[i] = rotar_derecha(data[i], n);
    }
    bufferResultado[size] = '\0';
    return bufferResultado;
}


char* desempaquetarParaRLE(const char* data, long tamanoOriginal, long& tamanoNuevo) {

    /**
 * @brief Convierte datos de un formato de ternas a un formato de pares para RLE.
 * @details De acuerdo a las especificaciones, los archivos RLE están guardados como ternas
 * [basura][conteo][caracter]. Esta función extrae los pares [conteo][caracter].
 * @param data Puntero a los datos en formato de ternas.
 * @param tamanoOriginal Tamaño del buffer de entrada.
 * @param tamanoNuevo Referencia donde se guardará el tamaño del nuevo buffer (2/3 del original).
 * @return Un nuevo buffer de memoria dinámica con los datos en formato de pares.
 */

    if (tamanoOriginal % 3 != 0) return nullptr;
    tamanoNuevo = (tamanoOriginal / 3) * 2;
    if (tamanoNuevo <= 0) return nullptr;
    char* bufferSalida = new char[tamanoNuevo + 1];
    long indiceEscritura = 0;
    for (long i = 0; i < tamanoOriginal; i += 3) {
        bufferSalida[indiceEscritura++] = data[i + 1];
        bufferSalida[indiceEscritura++] = data[i + 2];
    }
    bufferSalida[tamanoNuevo] = '\0';
    return bufferSalida;
}

char* descomprimirRLE(const char* dataComprimida, long tamanoComprimido, long& tamanoSalida) {

    /**
 * @brief Descomprime datos usando el algoritmo Run-Length Encoding (RLE).
 * @param dataComprimida Puntero a datos en formato de pares [conteo][caracter].
 * @param tamanoComprimido Tamaño del buffer de entrada.
 * @param tamanoSalida Referencia donde se guardará el tamaño del texto descomprimido.
 * @return Un nuevo buffer con los datos descomprimidos, o nullptr si hay un error.
 */
    if (tamanoComprimido % 2 != 0) return nullptr;
    tamanoSalida = 0;
    for (long i = 0; i < tamanoComprimido; i += 2) {
        tamanoSalida += static_cast<unsigned char>(dataComprimida[i]);
    }
    if (tamanoSalida <= 0 || tamanoSalida > LIMITE_DE_MEMORIA) return nullptr;
    char* bufferSalida = new char[tamanoSalida + 1];
    long indiceEscritura = 0;
    for (long i = 0; i < tamanoComprimido; i += 2) {
        unsigned char conteo = dataComprimida[i];
        char caracter = dataComprimida[i + 1];
        if (indiceEscritura + conteo > tamanoSalida) {
            delete[] bufferSalida; return nullptr;
        }
        for (int j = 0; j < conteo; j++) bufferSalida[indiceEscritura++] = caracter;
    }
    bufferSalida[tamanoSalida] = '\0';
    return bufferSalida;
}

char* descomprimirLZ78(const char* dataComprimida, long tamanoComprimido, long& tamanoSalida) {
    /**
 * @brief Descomprime datos usando el algoritmo LZ78.
 * @details Procesa los datos de entrada en ternas [índice de 2 bytes][caracter de 1 byte].
 * Reconstruye el texto original utilizando un diccionario dinámico.
 * @param dataComprimida Puntero a los datos comprimidos.
 * @param tamanoComprimido Tamaño del buffer de entrada.
 * @param tamanoSalida Referencia donde se guardará el tamaño del texto descomprimido.
 * @return Un nuevo buffer con los datos descomprimidos, o nullptr si hay un error.
 */
    if (tamanoComprimido % 3 != 0) return nullptr;
    int capacidadDiccionario = 4096;
    char** diccionario = new char*[capacidadDiccionario]();
    int proximoIndice = 1;
    long capacidadSalida = tamanoComprimido * 15;
    if (capacidadSalida > LIMITE_DE_MEMORIA) capacidadSalida = LIMITE_DE_MEMORIA;
    char* bufferSalida = new char[capacidadSalida];
    long indiceEscritura = 0;

    for (long i = 0; i < tamanoComprimido; i += 3) {
        unsigned short indice = *reinterpret_cast<const unsigned short*>(dataComprimida + i);
        char caracter = dataComprimida[i + 2];
        if (indice >= proximoIndice) {
            for(int j = 1; j < proximoIndice; j++) if(diccionario[j]) delete[] diccionario[j];
            delete[] diccionario; delete[] bufferSalida; return nullptr;
        }
        const char* prefijo = (indice == 0) ? "" : diccionario[indice];
        long longitudPrefijo = mlen(prefijo);
        if (indiceEscritura + longitudPrefijo + 1 >= capacidadSalida) {
            for(int j = 1; j < proximoIndice; j++) if(diccionario[j]) delete[] diccionario[j];
            delete[] diccionario; delete[] bufferSalida; return nullptr;
        }
        copy_mem(bufferSalida + indiceEscritura, prefijo, longitudPrefijo);
        bufferSalida[indiceEscritura + longitudPrefijo] = caracter;
        indiceEscritura += longitudPrefijo + 1;
        if (proximoIndice < capacidadDiccionario) {
            char* nuevaEntrada = new char[longitudPrefijo + 2];
            copy_mem(nuevaEntrada, prefijo, longitudPrefijo);
            nuevaEntrada[longitudPrefijo] = caracter;
            nuevaEntrada[longitudPrefijo + 1] = '\0';
            diccionario[proximoIndice++] = nuevaEntrada;
        }
    }
    tamanoSalida = indiceEscritura;

    for(int i = 1; i < proximoIndice; i++) if(diccionario[i]) delete[] diccionario[i];
    delete[] diccionario;

    char* bufferFinal = new char[tamanoSalida + 1];
    copy_mem(bufferFinal, bufferSalida, tamanoSalida);
    bufferFinal[tamanoSalida] = '\0';
    delete[] bufferSalida;

    return bufferFinal;
}

long mlen(const char* str) {

    /**
 * @brief Calcula la longitud de una cadena de texto terminada en nulo.
 * @param str La cadena de texto a medir.
 * @return El número de caracteres en la cadena, sin incluir el terminador nulo.
 */
    if (str == nullptr) return 0;
    long len = 0;
    while (str[len] != '\0') len++;
    return len;
}

const char* Find_P(const char* texto, const char* patron) {

    /**
 * @brief Busca la primera aparición de una subcadena (patrón) dentro de una cadena de texto.
 * @param texto El texto donde se realizará la búsqueda.
 * @param patron La subcadena a encontrar.
 * @return Un puntero a la primera aparición del patrón en el texto, o 'nullptr' si no se encuentra.
 */
    if (!patron || !texto) return nullptr;
    if (*patron == '\0') return texto;
    const char* p1 = texto;
    while (*p1) {
        const char* p1Begin = p1, *p2 = patron;
        while (*p1 && *p2 && *p1 == *p2) {
            p1++;
            p2++;
        }
        if (!*p2) return p1Begin;
        p1 = p1Begin + 1;
    }
    return nullptr;
}

void copy_mem(char* destino, const char* origen, long n) {

    /**
 * @brief Copia un bloque de memoria de un origen a un destino.
 * @param destino El buffer de memoria donde se copiarán los datos.
 * @param origen El buffer de memoria desde donde se copiarán los datos.
 * @param n El número de bytes a copiar.
 */
    for (long i = 0; i < n; i++) destino[i] = origen[i];
}
