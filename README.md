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
- _Semantic Analysis_, dan
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
1. Jalankan perintah 'make clean', lalu 'make' untuk mengkompilasi program.
2. Jalankan program dengan 'make', program akan dikompilasi dan akan disimpan dalam bin/program.exe
3. Jalankan bin/program.exe dengan menerima input yang ada di 'test/input' (pilih salah satu nama file dari yang diberikan untuk dijalankan) dengan format:
```sh
./bin/program.exe [FLAGS] [NAMA_FILE]
```
4. Program akan menampilkan hasil _Parse Tree_ (atau _Lexical Analysis_,tergantung pada flag yang dideklarasikan) dari file yang dipilih, berupa representasi parse tree grammer yang terbentuk dari _source code_ yang dibaca.
5. Untuk kasus memroseskan dari _tokenize_ menjadi _parsed_ program tinggal dijalankan dengan cara berikut. (catatan, tokenize akan dibaca melalui direktori Lexer/ saja)
```sh
./bin/program.exe [{NAMA_FILE}.txt_tokenize]
```


## Pembagian Tugas~
### _Syntax Analysis_
Nama | NIM | Workload | Presentase
--- | --- | --- | ---
Faiq Azzam Nafidz | 13524003 | Membuat struktur kelas terkait parser, membuat desain awal parser, membuat fungsi produksi 1–10 + 25–27, membuat fungsi pencetak parse tree, membuat mekanisme error handling, membuat laporan (perancangan & implementasi). | 35% | 
Anindya Naufal Pinasthika | 13524013 | Membuat fungsi produksi 34–43, membuat fitur pemilihan proses program (antara lexer atau parser), memodifikasi README.md, membuat laporan (teori singkat). | 25% |
Mahmudia Kimdaro Amin | 13524083 | Membuat fungsi produksi 21–24 + 28–33, melakukan pengujian program, membuat laporan (pengujian). | 20% | 
Jingglang Galih Rinenggan | 13524095 | Membuat fungsi produksi 11–20, membuat laporan (kesimpulan dan saran). | 20% | 
