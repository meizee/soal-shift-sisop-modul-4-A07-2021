# soal-shift-sisop-modul-4-A07-2021

Soal Shift Modul 4 |  Sistem Operasi 2021

1. Zahra Dyah Meilani 05111940000069
2. Ghifari Astaudi' Ukmullah 05111940000012
3. Fairuz Hasna Rofifah 05111940000003

## Soal 1

Membuat encoding dengan ketentuan seperti berikut:

a. Jika sebuah direktori dibuat dengan awalan “AtoZ\_”, maka direktori tersebut akan menjadi direktori ter-encode.

b. Jika sebuah direktori di-rename dengan awalan “AtoZ\_”, maka direktori tersebut akan menjadi direktori ter-encode.

c. Apabila direktori yang terenkripsi di-rename menjadi tidak ter-encode, maka isi direktori tersebut akan terdecode.

d. Setiap pembuatan direktori ter-encode (mkdir atau rename) akan tercatat ke sebuah log. Format : **/home/[USER]/Downloads/[Nama Direktori]** → **/home/[USER]/Downloads/AtoZ\_[Nama Direktori]**

e. Metode encode pada suatu direktori juga berlaku terhadap direktori yang ada di dalamnya (rekursif).

### Penjelasan

Pertama, untuk mengecek apakah sebuah direktori dari folder memiliki prefiks 'AtoZ' atau tidak dengan fungsi sebagai berikut:
```C
bool isAtoZ(const char *path) {
    int len = strlen(path);
    for (int i = 0; i < len - 5 + 1; i++) {
        if (path[i] == 'A' && path[i+1] == 't' && path[i+2] == 'o' && path[i+3] == 'Z' && path[i+4] == '_') return true;
    }
    return false;
}
```

Selanjutnya, jika sebuah folder tadi benar diawali "AtoZ" maka akan di-encode dengan Atbash Cipher, fungsi encode-nya adalah seperti berikut:
```C
void encode1(char* strEnc1) { 
	if(strcmp(strEnc1, ".") == 0 || strcmp(strEnc1, "..") == 0)
        return;
    
    int strLength = strlen(strEnc1);
    for(int i = 0; i < strLength; i++) {
		if(strEnc1[i] == '/') 
            continue;
		if(strEnc1[i]=='.')
            break;
        
		if(strEnc1[i]>='A'&&strEnc1[i]<='Z')
            strEnc1[i] = 'Z'+'A'-strEnc1[i];
        if(strEnc1[i]>='a'&&strEnc1[i]<='z')
            strEnc1[i] = 'z'+'a'-strEnc1[i];
    }
}
```

Di dalam fungsi di atas, akan dicek setiap karakter dalam char path-nya, jika sama dengan `\` akan di-continue karena tidak di-encode dan ketika sama dengan `.` akan di break, karena setelah tanda titik merupakan ekstensi file sehingga tidak perlu di-encode.

Sedangkan untuk decode-nya, hampir sama seperti fungsi encode karena rumus dari Atbash cipher sendiri sama untuk encode dan decode-nya:
```C
void decode1(char * strDec1){ //decrypt encv1_
	if(strcmp(strDec1, ".") == 0 || strcmp(strDec1, "..") == 0 || strstr(strDec1, "/") == NULL) 
        return;
    
    int strLength = strlen(strDec1), s=0;
	for(int i = strLength; i >= 0; i--){
		if(strDec1[i]=='/')break;

		if(strDec1[i]=='.'){//nyari titik terakhir
			strLength = i;
			break;
		}
	}
	for(int i = 0; i < strLength; i++){
		if(strDec1[i]== '/'){
			s = i+1;
			break;
		}
	}
    for(int i = s; i < strLength; i++) {
		if(strDec1[i] =='/'){
            continue;
        }
        if(strDec1[i]>='A'&&strDec1[i]<='Z'){
            strDec1[i] = 'Z'+'A'-strDec1[i];
        }
        if(strDec1[i]>='a'&&strDec1[i]<='z'){
            strDec1[i] = 'z'+'a'-strDec1[i];
        }
    }
	
}
```

Kemudian untuk melakukan logging dari soal ini adalah sebagai berikut:
```C
void logging2(const char* old, char* new) {
	char* filename = basename(new);

	if(isAtoZ(filename)) {
		FILE * logFile = fopen("/home/hasna/fs.log", "a");
    	fprintf(logFile, "%s → %s\n", old, new);
    	fclose(logFile);
	}
		
}
```
Jika terdapat pembuatan folder atau rename di folder yang berawalan AtoZ akan tercatat pada log dengan format : **/home/[USER]/Downloads/[Nama Direktori]** → **/home/[USER]/Downloads/AtoZ\_[Nama Direktori]**.

## Soal 4

Untuk memudahkan dalam memonitor kegiatan pada filesystem mereka Sin dan Sei
membuat sebuah log system dengan spesifikasi sebagai berikut.

a. Log system yang akan terbentuk bernama “SinSeiFS.log” pada direktori home
pengguna (/home/[user]/SinSeiFS.log). Log system ini akan menyimpan daftar
perintah system call yang telah dijalankan pada filesystem.

b. Karena Sin dan Sei suka kerapian maka log yang dibuat akan dibagi menjadi dua
level, yaitu INFO dan WARNING.

c. Untuk log level WARNING, digunakan untuk mencatat syscall rmdir dan unlink.

d. Sisanya, akan dicatat pada level INFO.

e. Format untuk logging yaitu:

[Level]::[dd][mm][yyyy]-[HH]:[MM]:[SS]:[CMD]::[DESC :: DESC]

Level : Level logging, dd : 2 digit tanggal, mm : 2 digit bulan, yyyy : 4 digit tahun,
HH : 2 digit jam (format 24 Jam),MM : 2 digit menit, SS : 2 digit detik, CMD :

System Call yang terpanggil, DESC : informasi dan parameter tambahan
INFO::28052021-10:00:00:CREATE::/test.txt
INFO::28052021-10:01:00:RENAME::/test.txt::/rename.txt

### Pembahasan

Untuk pembuatan log, akan dibuat dua tipe yaitu INFO dan WARNING, untuk mendapatkan tanggal kami menggunakan fungsi dari time_t sebagai berikut.

```C
void logging(char* c, int type){
    FILE * logFile = fopen("/home/hasna/SinSeiFS.log", "a");
	time_t currTime;
	struct tm * time_info;
	time ( &currTime );
	time_info = localtime (&currTime);

    int yr=time_info->tm_year + 1900;
	int mon=time_info->tm_mon + 1;
	int day=time_info->tm_mday;
	int hour=time_info->tm_hour;
	int min=time_info->tm_min;
	int sec=time_info->tm_sec;

    if(type==1){//info
        fprintf(logFile, "INFO::%02d%02d%d-%d:%d:%d:%s\n", day, mon, yr, hour, min, sec, c);
    }
    else if(type==2){ //warning
        fprintf(logFile, "WARNING::%02d%02d%d-%d:%d:%d:%s\n", day, mon, yr, hour, min, sec, c);
    }
    fclose(logFile);
}
```


