#include <openssl\aes.h>
#include <openssl\rand.h>
#include <openssl\evp.h>
#include <cstring>
#include <fstream>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include "bass.h"
#include <windows.h>
#include <MMSystem.h>
 

using namespace std;
string odczytZPliku(char *location)
{
	char *buffer = NULL;
	long length;

	FILE *file = fopen(location, "rb");
	if (!file == NULL)
	{
		fseek(file, 0, SEEK_END);
		length = ftell(file);
		fseek(file, 0, SEEK_SET);
		buffer = (char *)malloc(length * sizeof(char));
		if (buffer)
		{
			fread(buffer, 1, length, file);
		}
		fclose(file);
		return string(buffer, length);
	}
	else
	{
		return "";
	}
}
void zapisDoPliku(string do_pliku, string gdzie)
{
	ofstream output1(gdzie, ios::trunc | ios::binary);
	output1 << do_pliku << endl;
	output1.close();
}
void odtwarzacz()
{
	BASS_Init(-1, 44100, 0, 0, NULL);
	BASS_SetVolume(1);
	HSAMPLE sample = BASS_SampleLoad(false, "F:\\Krypto\\Krypto\\odkodowany.mp3", 0, 0, 1, BASS_SAMPLE_MONO);
	HCHANNEL channel = BASS_SampleGetChannel(sample, FALSE);
	BASS_ChannelPlay(channel, FALSE);
}

void generuj_klucze(string sciezka_do_keystore, int ile)
{
	for (int i = 1; i <= ile; i++)
	{
		unsigned char losowy_klucz[128];
		RAND_bytes(losowy_klucz, 128);
		ostringstream ss;
		ss << i;
		string str = ss.str();
		string sciezka = sciezka_do_keystore + "\\" + str + ".key";
		zapisDoPliku(reinterpret_cast<char*>(losowy_klucz), sciezka);
	}
	cout << "Klucze zostaly wygenerowane." << endl;
}

unsigned char* pobierz_klucz(string adres_keystore, char * id)
{
	string sciezka = adres_keystore + "\\" + id + ".key";
	string klucz = odczytZPliku((char *)sciezka.c_str());
	if (klucz != "")
	{
		return (unsigned char *)klucz.c_str();
	}
	else
	{
		cout << "Nie znaleziono klucza" << endl;
		return NULL;
	}

}


BOOL koduj(char *adres_pliku, char *password, unsigned char *klucz)
{
	string do_zakodowania = odczytZPliku(adres_pliku);
	if (klucz == NULL || do_zakodowania == "")
	{
		return false;
	}
	else
	{
		unsigned char *zakoduj = (unsigned char*)do_zakodowania.c_str();
		int outlen1, outlen2;
		unsigned char *zakodowane = new unsigned char[(do_zakodowania.length() + 1) * 2];


		EVP_CIPHER_CTX ctx;
		EVP_EncryptInit(&ctx, EVP_aes_128_cbc(), klucz, (unsigned char*)password);
		EVP_EncryptUpdate(&ctx, zakodowane, &outlen1, zakoduj, (do_zakodowania.length() + 1));
		EVP_EncryptFinal(&ctx, zakodowane + outlen1, &outlen2);

		string to_file = string((char*)zakodowane, outlen1);
		string lokal = string(adres_pliku);
		zapisDoPliku(to_file, "zakodowany" + lokal.substr(lokal.find_last_of('.')));
		return true;
	}
}

BOOL odkoduj(char *adres_pliku, char *password, unsigned char *klucz)
{
	string do_odkodowania = odczytZPliku(adres_pliku);

	if (klucz == NULL || do_odkodowania == "")
	{
		return false;
	}
	else
	{
		unsigned char * zakodowane = (unsigned char *)do_odkodowania.c_str();

		unsigned char *odkodowane = new unsigned char[(do_odkodowania.length() + 1) * 2];
		int outlen1, outlen2;

		EVP_CIPHER_CTX ctx;
		EVP_DecryptInit(&ctx, EVP_aes_128_cbc(), klucz, (unsigned char*)password);
		EVP_DecryptUpdate(&ctx, odkodowane, &outlen1, zakodowane, (do_odkodowania.length() + 1));
		EVP_DecryptFinal(&ctx, odkodowane + outlen1, &outlen2);

		string to_file = string((char*)odkodowane, outlen1);
		string lokal = string(adres_pliku);
		zapisDoPliku(to_file, "odkodowany" + lokal.substr(lokal.find_last_of('.')));
		return true;
	}

}


int main(int argc, char **argv)
{
	//Generowanie kluczy !!
	//generuj_klucze("F:\\Krypto", 20);
/**/
	if (argc != 4)
	{
		cout << "Podaj 3 parametry !";
		return 0;
	}

	string password;
	cout << "Haslo : ";
	cin >> password;

	if (string(argv[1]) == "szyfruj")
	{
		BOOL check = koduj(argv[2], (char *)password.c_str(), pobierz_klucz("F:\\Krypto", argv[3]));
		if (check)
		{
			cout << "zakodowano" << endl;
		}
		else
		{
			cout << "nie udalo sie zakodowac pliku" << endl;
		}
	}
	else if (string(argv[1]) == "deszyfruj")
	{
		BOOL check = odkoduj(argv[2], (char *)password.c_str(), pobierz_klucz("F:\\Krypto", argv[3]));

		if (check)
		{
			cout << "odkodowano \n odtwarzam plik..." << endl;
			odtwarzacz();
		}
		else
		{
			cout << "nie udalo sie odkodowac pliku" << endl;
		}

	}
	/**/
	system("pause");
}