# Arion Interpreter
Tugas Besar IF2224 - Teori Bahasa Formal dan Automata

Oleh (NIM):
- Faiq Azzam Nafidz (13524003)
- Anindya Naufal Pinasthika (13524013)
- Mahmudia Kimdaro Amin (13524083)
- Jingglang Galih Rinenggan (13524095)

## Deskripsi
Arion merupakan bahasan pemrograman baru yang sedang dikembangkan. Program ini adalah interpreter dari bahasa pemrograman Arion yang dapat melakukan
- _Lexical Analysis_, (v)
- _Syntax Analysis_, (v)
- _Semantic Analysis_, (v) dan
- _Intermediate Code Generation_. (v)

Keempat komponen tersebut dijadikan satu menjadi interpreter utuh yang dapat menjalankan _source code_ berbahasa pemrograman Arion.

Pada rilis ini, _Intermediate Code Generation_ telah diimplementasikan translasi _AST_ (_Abstract Syntax Tree_) yang dihasilkan oleh _semantic analyzer_ menjadi _intermediate code_ sebagai langkah terakhir dalam kompilasi kode sumber menjadi kode mesin yang dapat dibaca oleh _interpreter_. _Interpreter_ juga sekaligus diimplementasikan untuk membaca dan mengeksekusi _intermediate code_ yang dihasilkan tersebut.

## _Requirements_
1. G++ Compiler
2. C++17
3. Make
4. Environment Linux/Unix/MacOS/WSL

## Cara Instalasi Program
1. Clone repository ini ke komputer dengan perintah
```sh
git clone https://github.com/Naufal-Pinasthika/TBF-Tubes-IF2224-2026.git
```
2. Masuk ke direktori hasil clone dengan perintah
```sh
cd TBF-Tubes-IF2224-2026
```

## Cara Penggunaan Program
1. Jalankan perintah `make clean` untuk membersihkan direktori dari hasil kompilasi sebelumnya.
2. Kompilasi program dengan `make`, program akan dikompilasi dan akan disimpan dalam bin/program.exe
3. Untuk melihat hasil kompilasi kode untuk setiap tahap, jalankan `bin/program.exe` dengan format:
```sh
./bin/program.exe [FLAG] < [INPUT_FILE] > [OUTPUT_FILE]
```
Contoh:
```sh
./bin/program.exe -I < test/input/tc1.txt > test/milestone-4/tc1.txt_ic
```
FLAG yang diterima adalah `-L` (_lexer_) untuk _Lexical Analysis_, `-P` (_parser_) untuk _Syntax Analysis_, `-S` (_semantic analyzer_) untuk _Semantic Analysis_. Jika FLAG tidak diberikan, dan `-I` (_intermediate code generation_). INPUT_FILE diisi dengan _path file_ relatif terhadap folder repositori (TBF-Tubes-IF2224-2026) yang berisi kode sumber atau _output_ dari suatu tahap kompilasi. OUTPUT_FILE diisi dengan _path file_ relatif terhadap folder repositori sebagai tempat menyimpan _output_ hasil kompilasi pada tahap yang ditentukan oleh FLAG.

Perlu diperhatikan bahwa _file_ tidak bisa diproses oleh program bila permintaan jenis proses 'di bawah' urutan pemrosesan _file_ tersebut (_file_ pada `test/milestone-3` tidak bisa dilakukan proses _Lexical Analysis_ dan _Syntax Analysis_, sedangkan _file_ pada `test/milestone-2` tidak bisa dilakukan proses _Lexical Analysis_).

4. Untuk mengompilasi sekaligus menginterpretasi kode sumber berbahasa Arion layaknya _interpreter_ pada umumnya, jalankan `bin/program.exe` dengan format:
```sh
./bin/program.exe [INPUT_FILE]
```
Contoh (akan mengambil _file_ di test/input/tc1.txt secara otomatis):
```sh
./bin/program.exe tc1.txt
```
INPUT_FILE berupa nama _file_ (bukan _path_) yang berisi kode sumber berbahasa Arion. Pastikan _file_ berada di dalam direktori tempat menjalankan perintah (di dalam folder TBF-Tubes-IF2224-2026). Dengan perintah ini, program akan menjalankan kode sumber selayaknya _interpreter_ dan hasil keluaran kode akan ditampilkan pada terminal.

## Pembagian Tugas
### _Intermediate Code Generation & Interpreter_
Nama | NIM | Workload | Persentase
--- | --- | --- | ---
Faiq Azzam Nafidz | 13524003 |  |  | 
Anindya Naufal Pinasthika | 13524013 |  |  |
Mahmudia Kimdaro Amin | 13524083 |  |  | 
Jingglang Galih Rinenggan | 13524095 |  |  | 
