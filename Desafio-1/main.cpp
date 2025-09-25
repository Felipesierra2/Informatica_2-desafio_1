#include <iostream>
#include <fstream> // para lectura de archivo txt
#include <cstdio> //  impresion con formato sprintf (txt)

using namespace std;

char* readfile(const char* namefile, long& sizeofile);

int main() {
    int numeroDeArchivos;
    cout << "Ingrese el numero de archivos a procesar: ";
    cin >> numeroDeArchivos;

    // Bucle que se repetirá para cada par de archivos Encript y pist (1, 2, ..., n)
    for (int i = 1; i <= numeroDeArchivos; ++i) {

        cout << "\n---------- Procesando Par de Archivos #" << i << "----------" << '\n' << endl;

      /* Construcción dinámica de los nombres de archivo ---
        Creamos arreglos de char para guardar los nombres.
        25 caracteres es más que suficiente para "Encriptado10.txt"¨*/

        char nombreEncriptado[25];
        char nombrePista[25];

        /* Usamos sprintf para "imprimir" el nombre en el buffer.
        %d es un marcador de posición que se reemplaza por el valor de 'i'.*/
        sprintf(nombreEncriptado, "Encriptado%d.txt", i);
        sprintf(nombrePista, "pista%d.txt", i);

        cout << "Leyendo archivos: " << nombreEncriptado << " y " << nombrePista << '\n' <<endl;

        // --- Lectura del archivo encriptado ---
        long tamanoEncriptado = 0;
        char* datosEncriptados = readfile(nombreEncriptado, tamanoEncriptado);

        if (datosEncriptados == nullptr) {
            cout << "Fallo al leer " << nombreEncriptado << ". Saltando al siguiente par." << endl;
            continue; // 'continue' salta a la siguiente iteración del bucle.
        }

        // --- Lectura del archivo de pista ---
        long tamanoPista = 0;
        char* datosPista = readfile(nombrePista, tamanoPista);

        if (datosPista == nullptr) {
            cout << "Fallo al leer " << nombrePista << ". Saltando al siguiente par." << endl;
            delete[] datosEncriptados; // Liberamos la memoria que sí se leyó.
            continue;
        } datosPista[tamanoPista]='\0';
          datosEncriptados[tamanoEncriptado]='\0'; // para que no se impriman mas caracteres de los que deberían

        cout << "Texto encriptado leido: " << datosEncriptados << '\n' << endl;
        cout << "Dimension del texto encriptado: " << tamanoEncriptado << "bytes. " << '\n'<< endl;
        cout << "Pista leida : "<< datosPista << '\n' <<endl;
        cout << "Dimension de la pista: " << tamanoPista << " bytes. " << endl;



        // Procesamiento de textos





        // Limpieza de memoria para cada par de archivos

        delete[] datosEncriptados;
        delete[] datosPista;
    }

    cout << "\n--- Proceso completado. ---" << endl;
    return 0;
}


char* readfile(const char* namefile, long& sizeofile){
/*
 * Función: readfile
 * ------------------
 * Esta función abre un archivo de texto o binario, calcula su tamaño y copia
 * todo su contenido en un arreglo dinámico de caracteres (char*) para que
 * podamos usarlo en el programa.
 *
 * Parámetros:
 *   - namefile : el nombre del archivo a leer (ej: "datos.txt").
 *   - sizeofile: una referencia donde guardaremos el tamaño real del archivo
 *                en bytes (cantidad de letras, espacios o símbolos que tiene).
 *
 * Funcionamiento paso a paso (explicado fácil):
 *   1. Intenta abrir el archivo. Si no existe o hay un error, avisa y devuelve nullptr.
 *   2. Se mueve al final del archivo para medir su tamaño total (como contar cuántas
 *      fichas hay en una fila sin mirarlas una por una).
 *   3. Si el archivo está vacío (tamaño 0), también devuelve nullptr.
 *   4. Vuelve al inicio del archivo para empezar a leerlo desde el principio.
 *   5. Reserva memoria dinámica (new char[]) con el tamaño exacto del archivo (+1
 *      por seguridad).
 *   6. Copia todo el contenido del archivo dentro de ese arreglo.
 *   7. Si la lectura falla, libera la memoria y devuelve nullptr.
 *   8. Si todo salió bien, devuelve el puntero al arreglo con el contenido del archivo.
 *
 * Qué devuelve:
 *   - Un puntero (char*) que apunta al contenido del archivo en memoria.
 *   - nullptr si hubo un error (archivo no existe, vacío o fallo al leer).
 *
 * Nota importante:
 *   - Como se usa memoria dinámica, el programador debe liberar el arreglo con
 *     delete[] cuando ya no se necesite, para evitar fugas de memoria.

*/

    // Abrir archivo
    ifstream archivo (namefile,  ios::in |ios::binary| ios::ate); // at the end ate, coloca el cursor del archivo al final de ltexto para luego usar tellg y saber el tamañao del archivo

        if (!archivo.is_open()){ // si el file no se encuentra (Fail)

        cout << "Error al abrir el archivo" << endl;

        return nullptr;

        }

    // Tamaño de archivo

        sizeofile= archivo.tellg();//tellg() nos da la posición actuall, que es el final.

        if (sizeofile == 0 ){

            cout<< "El archivo esta vacío" <<endl;
            archivo.close();
                return nullptr;
        }

        archivo.seekg(0,ios::beg);

        char* dataread = new char [sizeofile +1 ]; // arreglo dinámico que guarda cada caracter (byte) del archivo de texto plano de manera dinámica y además en binario.

        if(!archivo.read(dataread,sizeofile)){
            cout<< "Error al leer el contenido del archivo." <<endl;
            archivo.close();
            delete[] dataread; // Si falla la lectura, liberamos la memoria antes de salir.
            return nullptr;
        }
        archivo.close();
        return dataread;
}
