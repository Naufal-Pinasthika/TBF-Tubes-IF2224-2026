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
- _Syntax Analysis_,
- _Semantic Analysis_, dan
- _Intermediate Code Generation_.

Keempat komponen tersebut dijadikan satu menjadi interpreter utuh yang dapat menjalankan _source code_ berbahasa pemrograman Arion.

Pada rilis ini, _Lexical Analysis_ telah terimplementasi dengan acuan desain _deterministic finite automata_ dalam pembacaan _source code_ yang diubah menjadi token-token yang dapat digunakan oleh komponen _interpreter_ selanjutnya.

## _Requirements_
1. G++ Compiler
2. C++17
3. Make
4. Environment Linux/Unix/MacOS/WSL

## Cara Instalasi Program
1. Clone repository ini ke komputer dengan perintah `git clone [ini linknya apa]`
2. Masuk ke direktori hasil clone dengan perintah `cd [nama folder hasil clone]`

## Cara Penggunaan Program
1. Jalankan perintah 'make clean', lalu 'make' untuk mengkompilasi program.
2. Jalankan program dengan 'make run', saat ini program hanya menerima input yang ada di 'test/input'. Pilih salah satu nama file dari yang diberikan untuk dijalankan.
3. Program akan menampilkan hasil _lexical analysis_ dari file yang dipilih, berupa token-token yang terbentuk dari _source code_ yang dibaca.


## Pembagian Tugas
### _Lexical Analysis_
Nama | NIM | Workload | Presentase
--- | --- | --- | ---
Faiq Azzam Nafidz | 13524003 |  | 
Anindya Naufal Pinasthika | 13524013 |  | 
Mahmudia Kimdaro Amin | 13524083 |  | 
Jingglang Galih Rinenggan | 13524095 |  | 