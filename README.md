# Arion Interpreter
Tugas Besar IF2224 - Teori Bahasa Formal dan Automata

Oleh (NIM):
- Faiq Azzam Nafidz (13524003)
- Anindya Naufal Pinasthika (13524013)
- Mahmudia Kimdaro Amin (13524083)
- Jingglang Galih Rinenggan (13524095)

## Deskripsi
Arion merupakan bahasan pemrograman baru yang sedang dikembangkan. Program ini adalah interpreter dari bahasa pemrograman Arion tersebut yang dapat melakukan
- _Lexical Analysis_, (v)
- _Syntax Analysis_, (v)
- _Semantic Analysis_, (v) dan
- _Intermediate Code Generation_.

Keempat komponen tersebut dijadikan satu menjadi interpreter utuh yang dapat menjalankan _source code_ berbahasa pemrograman Arion.

Pada rilis ini, _Syntax Analysis_ telah diimplementasikan dengan acuan desain _parse tree_ serta penerapan algoritma _recrusive descent_. Proses sekarang akan membaca _source code_ yang mengubah token-token yang sudah dibuat oleh _Lexical Analysis_ menjadi sebuah representasi _parse tree_ yang akan diproses lebih lanjut oleh komponen _interpreter_ selanjutnya.

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
3. Jalankan `bin/program.exe` dengan menerima input yang ada di `test/input` (pilih salah satu nama file di dalamnya) dengan format:
```sh
./bin/program.exe [FLAG] <NAMA_FILE>
```
FLAG yang diterima adalah `-L` (_lexer_) untuk _Lexical Analysis_, `-P` (_parser_) untuk _Syntax Analysis_, dan `-S` (_semantic analyzer_) untuk _Semantic Analysis_. Jika FLAG tidak diberikan, secara _default_ program akan melakukan _Semantic Analysis_.

4. Program akan menampilkan hasil proses dari file yang dipilih sesuai FLAG yang dipilih. FLAG `-L` menghasilkan kumpulan token representasi _source code_ yang dibaca dan disimpan pada `test/milestone-1`. FLAG `-P` sama seperti flag sebelumnya dengan tambahan menghasilkan _parse tree_ yang berupa representasi _source code_ yang dibaca berdasarkan grammar yang ditentukan dan disimpan pada `test/milestone-2`. FLAG `-S` memberikan hasil sama dengan flag sebelumnya dengan tambahan menghasilkan keluaran proses _semantic analysis_ berupa _symbol table_ (simbol-simbol yang muncul dalam kode) dan _decorated abstract syntax tree_ (hirearki sintaks kode) dan disimpan pada `test/milestone-3`.

5. Sebagai alternatif, nama-nama file di dalam folder `test/milestone-1` dan `test/milestone-2` juga bisa dimasukkan dalam NAMA_FILE pada perinah penjalanan program untuk memrosesnya. Perlu diperhatikan bahwa file tidak bisa diproses oleh program bila permintaan jenis proses 'di bawah' urutan pemrosesan file tersebut (file pada `test/milestone-3` tidak bisa dilakukan proses _Lexical Analysis_ dan _Syntax Analysis_, sedangkan file pada `test/milestone-2` tidak bisa dilakukan proses _Lexical Analysis_).

## Pembagian Tugas
### _Semantic Analysis_
Nama | NIM | Workload | Presentase
--- | --- | --- | ---
Faiq Azzam Nafidz | 13524003 | AST (sebagian), Semantic (sebagian), Laporan (Perancangan, Implementasi), README.md, Debugging | 30% | 
Anindya Naufal Pinasthika | 13524013 | Symbol Table, AST (sebagian), Semantic (sebagian), main.cpp, Debugging | 30% |
Mahmudia Kimdaro Amin | 13524083 | AST (sebagian), Semantic (sebagian), Laporan (Teori dasar, Kesimpulan) | 20% | 
Jingglang Galih Rinenggan | 13524095 | Input file Parse Tree, AST (sebagian), Semantic (sebagian) | 20% | 
