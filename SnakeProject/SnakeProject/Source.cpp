#include<windows.h>
#include<iostream>
#include<cmath>
#include<vector>
#include<fstream>
#include<iomanip>
#include<ctime>
#include<bitset>
#include"rlutil.h"

#define CRYPTER_SEED 14
#define c2(nr) -(256-nr)
#define SLEEP_DURATION 30

#define MAP_WIDTH 100	// Default: MAP_WIDTH 85, MAP_HEIGHT 25
#define MAP_HEIGHT 30
using namespace std;

int main();

typedef enum {
	Up,
	Right,
	Down,
	Left
} Direction;

enum Difficulty {
	Easy,
	Hard
} gameDifficulty;

struct SnakePiece {
	unsigned char form;
	unsigned short line, column;
	SnakePiece(unsigned short line, unsigned short column, unsigned char form)
	{
		this->line = line;
		this->column = column;
		this->form = form;
	}
};

class Cryptography
{
private:
	static int matrix4D[4][4][4][4];
	static string base36matrix4D[4][4][4][4];
	static bitset<256> usedASCII;
	static bool base36matrix4Dfilled;

public:
	static void GenerateMatrix()	// genereaza matrix4D ( Apeleaza doar o data!  )
	{

		for (int i = 0; i < 256; i++)
			usedASCII[i] = false;

		for (int w = 0; w < 4; w++)
			for (int z = 0; z < 4; z++)
				for (int y = 0; y < 4; y++)
					for (int x = 0; x < 4; x++)
					{
						int randomNumber = rand() % 256;

						while (usedASCII[randomNumber] == true)
							randomNumber = rand() % 256;

						usedASCII[randomNumber] = true;
						matrix4D[w][z][y][x] = randomNumber;
					}
	}

	static char ToBase36(int input)
	{
		if (input <= 9)
			return (char)(input + 48);
		else
			return (char)(input + 55);
	}

	static void InitializeBase36Matrix4D()
	{
		if (base36matrix4Dfilled == false)
		{
			CreateBase36Matrix4D();
			base36matrix4Dfilled = true;
		}
	}

	static string Encrypt(string input)
	{
		string output;
		InitializeBase36Matrix4D();

		for (int i = 0; i<input.length(); i++)
		{
			int number = (int)(input[i]);

			int w = number % 4; number /= 4;
			int z = number % 4; number /= 4;
			int y = number % 4; number /= 4;
			int x = number % 4;

			output += base36matrix4D[w][z][y][x];
			output += (unsigned char)rand()%26 + 65;
		}

		return output;
	}

	static char getCharOf(string input)
	{
		char output;
		for (int w = 0; w < 4; w++)
			for (int z = 0; z < 4; z++)
				for (int y = 0; y < 4; y++)
					for (int x = 0; x < 4; x++)
						if (input == base36matrix4D[w][z][y][x])
						{
							output = (char)(w + z * 4 + y * 16 + x * 64);
							return output;
						}
		return '0';
	}

	static string Decrypt(string input)
	{
		string output;
		InitializeBase36Matrix4D();
		for (int i = 0; i < input.length() - input.length() % 3; i += 3)
		{
			string value;
			value += (char)input[i];
			value += (char)input[i + 1];

			output += getCharOf(value);
		}
		return output;
	}

	static void CreateBase36Matrix4D()	// nu suprascrie Seed.txt ! O sa pierzi cheia de decriptat datele vechi!
	{
		ifstream input("Seed.txt");
		string seed;
		input >> seed;
		input.close();

		int key = 0;
		for (int w = 0; w < 4; w++)
			for (int z = 0; z < 4; z++)
				for (int y = 0; y < 4; y++)
					for (int x = 0; x < 4; x++, key += 3)
					{
						base36matrix4D[w][z][y][x] += seed[key];
						base36matrix4D[w][z][y][x] += seed[key + 1];
					}
	}

	static string ConvertMatrixToBase36String()
	{
		string cryptString = "";
		for (int w = 0; w < 4; w++)
			for (int z = 0; z < 4; z++)
				for (int y = 0; y < 4; y++)
					for (int x = 0; x < 4; x++)
					{
						int number = matrix4D[w][z][y][x];
						char number36[2]{ 0 };

						number36[1] = ToBase36(number % 36);
						number /= 36;
						number36[0] = ToBase36(number % 36);

						cryptString += number36[0];
						cryptString += number36[1];
						cryptString += (unsigned char) rand()%26+65;
					}
		return cryptString;
	}

	static int TextSum(string str)
	{
		int sum = 0;
		for (int i = 0; i < str.length(); i++)
			sum += (int)str[i];
		return sum;
	}
};
int Cryptography::matrix4D[4][4][4][4];
string Cryptography::base36matrix4D[4][4][4][4];
bitset<256> Cryptography::usedASCII;
bool Cryptography::base36matrix4Dfilled = false;

vector<SnakePiece*> snake;
Direction snakeDirection;
unsigned char map[MAP_HEIGHT][MAP_WIDTH];
int score, sleepDuration;
bool foodExists, eaten, lostGame;

void drawMenu()
{
	char menu[21][100] =
	{
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\_','\_','\_','\_','\_','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\_','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\/','\^','\\','\/','\^','\\','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\/','\ ','\ ','\_','\_','\_','\|','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\|','\ ','\|','\ ','\ ','\ ','\ ','\ ','\ ','\ ', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\_','\|','\_','\_','\|','\ ','\ ','\O','\|','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\\','\ ','\`','\-','\-','\.','\ ','\_','\ ','\_','\_','\ ','\ ','\ ','\_','\_','\ ','\_','\|','\ ','\|','\ ','\_','\_','\_','\_','\_','\ ', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\\','\/','\ ','\ ','\ ','\ ','\ ','\/','\~','\ ','\ ','\ ','\ ','\ ','\\','\_','\/','\ ','\\','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\`','\-','\-','\.','\ ','\\','\ ','\'','\_','\ ','\\','\ ','\/','\ ','\_','\`','\ ','\|','\ ','\|','\/','\ ','\/','\ ','\_','\ ','\\', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\\','\_','\_','\_','\_','\|','\_','\_','\_','\_','\_','\_','\_','\_','\_','\_','\/','\ ','\ ','\\','\ ','\ ','\ ','\ ','\ ','\ ','\/','\\','\_','\_','\/','\ ','\/','\ ','\|','\ ','\|','\ ','\|','\ ','\(','\_','\|','\ ','\|','\ ','\ ','\ ','\<','\ ','\ ','\_','\_','\/', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\\','\_','\_','\_','\_','\_','\_','\_','\ ','\ ','\ ','\ ','\ ','\ ','\\','\ ','\ ','\ ','\ ','\\','\_','\_','\_','\_','\/','\|','\_','\|','\ ','\|','\_','\|','\\','\_','\_','\,','\_','\|','\_','\|','\\','\_','\\','\_','\_','\_','\|', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\`','\\','\ ','\ ','\ ','\ ','\ ','\\','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\|','\ ','\ ','\ ','\ ','\ ','\|','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\\', } ,
		{ '\ ','\_','\_','\_','\_','\_','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\/','\ ','\ ','\ ','\ ','\ ','\/','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\\', } ,
		{ '\|','\_','\ ','\ ','\ ','\_','\|','\_','\ ','\_','\_','\_','\ ','\_','\_','\_','\ ','\ ','\_','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\/','\ ','\ ','\ ','\ ','\ ','\/','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\\','\\', } ,
		{ '\ ','\ ','\|','\ ','\|','\/','\ ','\_','\`','\ ','\\','\ ','\V','\ ','\/','\ ','\|','\|','\ ','\|','\ ','\ ','\ ','\ ','\ ','\/','\ ','\ ','\ ','\ ','\ ','\/','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\\','\ ','\\', } ,
		{ 'b','\y','\|','\_','\|','\\','\_','\_','\,','\_','\|','\\','\_','\/','\ ','\\','\_','\,','\ ','\|','\ ','\ ','\ ','\ ','\/','\ ','\ ','\ ','\ ','\ ','\/','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\\','\ ','\ ','\\', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\|','\_','\_','\/','\ ','\ ','\ ','\ ','\/','\ ','\ ','\ ','\ ','\ ','\/','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\_','\-','\-','\-','\-','\_','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\\','\ ','\ ','\\', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\/','\ ','\ ','\ ','\ ','\ ','\/','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\_','\-','\~','\ ','\ ','\ ','\ ','\ ','\ ','\~','\-','\_','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\|','\ ','\ ','\ ','\|', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\(','\ ','\ ','\ ','\ ','\ ','\ ','\(','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\_','\-','\~','\ ','\ ','\ ','\ ','\_','\-','\-','\_','\ ','\ ','\ ','\ ','\~','\-','\_','\ ','\ ','\ ','\ ','\ ','\_','\/','\ ','\ ','\ ','\|', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\\','\ ','\ ','\ ','\ ','\ ','\ ','\~','\-','\_','\_','\_','\_','\-','\~','\ ','\ ','\ ','\ ','\_','\-','\~','\ ','\ ','\ ','\ ','\~','\-','\_','\ ','\ ','\ ','\ ','\~','\-','\_','\-','\~','\ ','\ ','\ ','\ ','\/', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\~','\-','\_','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\_','\-','\~','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\~','\-','\_','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\_','\-','\~','\ ','\ ','\ ', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\~','\-','\-','\_','\_','\_','\_','\_','\_','\-','\~','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\~','\-','\_','\_','\_','\-','\~', } ,
		{ '\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ','\ ',}
	};

	for (register int i = 0; i < 20; i++)
	{
		cout << setw(10);	// indentare stanga
		for (int j = 0; menu[i][j] != '\0'; j++)
		{
			if (i <= 5 && j >= 35)
				rlutil::setColor(14);	// pentru cuvantul "Snake" <- culoare galben
			else if (i >= 8 && j < 20)
				rlutil::setColor(14);	// pentru "byTavy" <- culoare galben
			else
				rlutil::setColor(10);	// pentru restul, lasam verde;
			cout << menu[i][j];
		}
		cout << endl;
	}

	for (register int i = 0; i < 25; i++)
		for (register int j = 0; j < 85; j++)
			map[i][j] = '\0';

	rlutil::setColor(14);
	cout << setw(45) << "Scorul maxim este de ";
	ifstream fin("ScorMaxim.txt");
	string cryptedScore;
	fin >> cryptedScore;
	fin.close();
	rlutil::setColor(10);
	if (Cryptography::TextSum(cryptedScore) % CRYPTER_SEED == 0)
	{
		bitset<32> binaryScore = bitset<32>(Cryptography::Decrypt(cryptedScore));
		cout << binaryScore.to_ulong();
	}
	else cout << 0;
	rlutil::setColor(14);
	
	if(lostGame == false)
	{ 
		cout << " puncte.\n\n";
		cout << setw(68) << "Apasa <1>(usor) / <2>(greu) pentru a incepe un joc nou!";
	}
	else
	{
		cout << " puncte.\n";
		cout << setw(45) << "Scorul tau a fost de ";
		rlutil::setColor(10);
		cout << score;
		rlutil::setColor(14);
		cout << " puncte. \n\n";
		cout << setw(68) << "Apasa <1>(usor) / <2>(greu) pentru a incepe un joc nou!";
	}
	
}

void initASCIIMenu()
{
	ifstream f("ASCIIin.txt");
	ofstream g("ASCIIout.txt");
	char c;
	bool firstTime = true;
	while (f >> noskipws >> c)
	{
		if (firstTime)
		{
			g << "{";
			firstTime = false;
		}
		if (c == '\n')
		{
			g << "} ," << endl;
			firstTime = true;
		}
		else
			g << "'\\" << c << "'" << ",";
	}
}

void initConsole()
{
	system("mode 85,25");   //Set mode to ensure window does not exceed buffer size
	SMALL_RECT WinRect = { 0, 0, 85, 25 };   //New dimensions for window in 8x12 pixel chars
	SMALL_RECT* WinSize = &WinRect;
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), true, WinSize);   //Set new size for window
}

void drawMap()
{
	rlutil::setColor(10);
	for (int j = 0; j < MAP_WIDTH-1; j++)
		cout << map[0][j];
	cout << endl;
	rlutil::setColor(14);
	for (register int i = 1; i < MAP_HEIGHT-2; i++)
	{
		rlutil::setColor(10);
		cout << map[i][0];
		rlutil::setColor(14);
		for (register int j = 1; j < MAP_WIDTH-2; j++)
				cout << map[i][j];
		rlutil::setColor(10);
		cout << map[i][MAP_WIDTH-2] << endl;
	}
	rlutil::setColor(10);

	for (register int j = 0; j < MAP_WIDTH-1; j++)
		cout << map[MAP_HEIGHT-2][j];
	cout << endl;

	cout << "Scor: " << score << "    ";
}

void generateMapBorder()
{
	map[0][0] = 218;
	map[0][MAP_WIDTH-2] = 191;
	map[MAP_HEIGHT-2][0] = 192;
	map[MAP_HEIGHT-2][MAP_WIDTH-2] = 217;
	for (int j = 1; j < MAP_WIDTH-2; j++)
		map[0][j] = map[MAP_HEIGHT-2][j] = 196;
	for (int i = 1; i < MAP_HEIGHT-2; i++)
		map[i][0] = map[i][MAP_WIDTH-2] = 179;

}

void generateSnake()
{
	SnakePiece *piece = new SnakePiece(1, 1, 205); 
	snake.push_back(piece);		
	piece = new SnakePiece(1, 2, 205); // ═ <- tail
	snake.push_back(piece);
	piece = new SnakePiece(1, 3, 205);
	snake.push_back(piece);
	piece = new SnakePiece(1, 4, 216); // ╪ <- head
	snake.push_back(piece);
}

void drawSnake()
{
	for (int i = 0; i < snake.size(); i++)
	{
		map[snake[i]->line][snake[i]->column] = snake[i]->form;
	}
}

void drawFood()
{
	int line, column;
	do {
		line = rand() % (MAP_HEIGHT-3) + 1;
		column = rand() % (MAP_WIDTH-3) + 1;
	} while (map[line][column] != '\0' || sqrt(pow((double)line*1.0 - snake[snake.size() - 1]->line, 2) + pow((double)column - snake[snake.size() - 1]->column, 2)) > 15);
	foodExists = true;
	map[line][column] = 254;
}

void checkSnakeCorners()
{
	if (snake[snake.size() - 1]->form == 215)	// cand sarpele se deplaseaza pe orizontala si isi schimba directia ( sus / jos )
	{
		if (snake[snake.size() - 1]->line - snake[snake.size() - 2]->line == 1)	// cand capul o ia in jos
		{
			if (snake[snake.size() - 3]->column - snake[snake.size() - 2]->column == -1 || snake[snake.size() - 3]->column - snake[snake.size() - 2]->column > 1)	// cand sarpele vine din stanga
				snake[snake.size() - 2]->form = 187;	// ╗
			else if (snake[snake.size() - 3]->column - snake[snake.size() - 2]->column == 1 || snake[snake.size() - 3]->column - snake[snake.size() - 2]->column < -1)	// cand sarpele vine din dreapta
				snake[snake.size() - 2]->form = 201;    // ╔
		}
		else if (snake[snake.size() - 1]->line - snake[snake.size() - 2]->line == -1)	// cand capul o ia in sus
		{
			if (snake[snake.size() - 3]->column - snake[snake.size() - 2]->column == -1 || snake[snake.size() - 3]->column - snake[snake.size() - 2]->column > 1)	// cand coada vine din stanga
				snake[snake.size() - 2]->form = 188;	// ╝
			else if (snake[snake.size() - 3]->column - snake[snake.size() - 2]->column == 1 || snake[snake.size() - 3]->column - snake[snake.size() - 2]->column < -1)	// cand coada vine din dreapta
				snake[snake.size() - 2]->form = 200;    // ╚
		}
	}
	else if (snake[snake.size() - 1]->form == 216) // cand sarpele se deplaseaza pe verticala si isi schimba directia ( stanga / dreapta )
	{
		if (snake[snake.size() - 1]->column - snake[snake.size() - 2]->column == 1) // cand capul o ia in dreapta
		{
			if (snake[snake.size() - 3]->line - snake[snake.size() - 2]->line == 1 || snake[snake.size() - 3]->line - snake[snake.size() - 2]->line < -1)	// cand sarpele vine de jos
				snake[snake.size() - 2]->form = 201;    // ╔
			else if (snake[snake.size() - 3]->line - snake[snake.size() - 2]->line == -1 || snake[snake.size() - 3]->line - snake[snake.size() - 2]->line > 1)	// cand sarpele vine de sus
				snake[snake.size() - 2]->form = 200;    // ╚
		}
		else if (snake[snake.size() - 1]->column - snake[snake.size() - 2]->column == -1) // cand capul o ia in stanga
		{
			if (snake[snake.size() - 3]->line - snake[snake.size() - 2]->line == 1 || snake[snake.size() - 3]->line - snake[snake.size() - 2]->line < -1)	// cand sarpele vine de jos
				snake[snake.size() - 2]->form = 187;    // ╗
			else if (snake[snake.size() - 3]->line - snake[snake.size() - 2]->line == -1 || snake[snake.size() - 3]->line - snake[snake.size() - 2]->line > 1)	// cand sarpele vine de sus
				snake[snake.size() - 2]->form = 188;    // ╝
		}
	}
}

void checkIfGameIsOver()
{
	if (lostGame)
	{
		ifstream fin("ScorMaxim.txt");
		int oldScore;
		string encryptedScore;
		fin >> encryptedScore;
		fin.close();

		if (Cryptography::TextSum(encryptedScore) % CRYPTER_SEED != 0)
		{
			score = 0;
			lostGame = true;
			return;
		}
		else
		{
			bitset<32> binaryScore = bitset<32>(Cryptography::Decrypt(encryptedScore));
			oldScore = binaryScore.to_ulong();
		}

		if (score > oldScore)
		{
			char binaryScoreChar[32];
			string encryptedScore;

			_itoa_s(score, binaryScoreChar, 2);
			do {
				encryptedScore = Cryptography::Encrypt(binaryScoreChar);
				if (Cryptography::TextSum(encryptedScore) % CRYPTER_SEED == 0)
					break;
			} while (1 != 0);

			ofstream fout("ScorMaxim.txt");
			fout << encryptedScore << endl;
			fout.close();
		}
		lostGame = true;
	}
}

void gameDifficultyHandler()
{
	if (gameDifficulty == Easy)
	{
		if (snake[snake.size() - 1]->line == 0)
			snake[snake.size() - 1]->line = MAP_HEIGHT-3;
		else if (snake[snake.size() - 1]->line == MAP_HEIGHT-2)
			snake[snake.size() - 1]->line = 1;

		if (snake[snake.size() - 1]->column == 0)
			snake[snake.size() - 1]->column = MAP_WIDTH-3;
		else if (snake[snake.size() - 1]->column == MAP_WIDTH-2)
			snake[snake.size() - 1]->column = 1;
	}
	else if (snake[snake.size() - 1]->line == 0 || snake[snake.size() - 1]->line == MAP_HEIGHT-2 || snake[snake.size() - 1]->column == 0 || snake[snake.size() - 1]->column == MAP_WIDTH-2)
		lostGame = true;	
}

void shiftSnake(int lineStep, int columnStep)
{
	/*
	╚ = 200
	╔ = 201
	╗ = 187
	╝ = 188
	║ = 186
	═ = 205
	╫ = 215
	╪ = 216
	─ = 196
	│ = 179
	*/
	char tokens[6] = { c2(200), c2(201), c2(187), c2(188), c2(186), c2(205) };
	unsigned char nextObject = map[snake[snake.size() - 1]->line + lineStep][snake[snake.size() - 1]->column + columnStep];
	if (nextObject == 254) // ■
	{
		score++;
		eaten = true;
	}
	else if (nextObject != '\0' && nextObject != 196 && nextObject != 179 && strchr(tokens, nextObject))
		lostGame = true;
	else if (snake[snake.size() - 1]->line + lineStep == MAP_HEIGHT-2 && memchr(tokens, map[1][snake[snake.size() - 1]->column], 6) )
		lostGame = true;
	else if (snake[snake.size() - 1]->column + columnStep == MAP_WIDTH - 2 && memchr(tokens, map[snake[snake.size() - 1]->line][1], 6))
		lostGame = true;
	else if (snake[snake.size() - 1]->line + lineStep == 0 && memchr(tokens, map[MAP_HEIGHT-3][snake[snake.size() - 1]->column], 6))
		lostGame = true;
	else if (snake[snake.size() - 1]->column + columnStep == 0 && memchr(tokens, map[snake[snake.size() - 1]->line][MAP_WIDTH-3], 6))
		lostGame = true;

	if (!eaten)
	{
		map[snake[0]->line][snake[0]->column] = '\0';
		map[snake[1]->line][snake[1]->column] = '\0';
	}
	else
	{
		SnakePiece *piece = new SnakePiece(snake[0]->line, snake[0]->column, map[snake[0]->line][snake[0]->column]);
		snake.push_back(NULL);
		for (int i = snake.size() - 1; i > 0; i--)
			snake[i] = snake[i - 1];
		snake[0] = piece;
		eaten = false;
		foodExists = false;
	}

	for (int i = 0; i < snake.size() - 1; i++)
	{
		snake[i]->line = snake[i+1]->line;
		snake[i]->column = snake[i + 1]->column;
		if(i+1 != snake.size()-1)
			snake[i]->form = snake[i + 1]->form;
		else
		{
			if (snake[snake.size() - 1]->form == 216)
				snake[i]->form = 205;
			else if (snake[snake.size() - 1]->form == 215)
				snake[i]->form = 186;
		}
	}

	snake[snake.size() - 1]->line += lineStep;
	snake[snake.size() - 1]->column += columnStep;

	checkSnakeCorners();
	gameDifficultyHandler();
	checkIfGameIsOver();
}

void checkNextPosition()
{
	switch (snakeDirection) 
	{
	case Up :
		snake[snake.size() - 1]->form = 215;
		shiftSnake(-1, 0);
		break;
	case Right:
		snake[snake.size() - 1]->form = 216;
		shiftSnake(0, 1);
		break;
	case Down:
		snake[snake.size() - 1]->form = 215;
		shiftSnake(1, 0);
		break;
	case Left:
		snake[snake.size() - 1]->form = 216;
		shiftSnake(0, -1);
		break;
	}
}

void initKeyListener()
{
	std::clock_t start;
	double duration;
	bool keyPressed = false;
	start = std::clock();
	do {
		duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
		keyPressed = kbhit();
		if (keyPressed)
			break;
	} while (duration < 1.0*sleepDuration / 1000);
	if (keyPressed)
	{
		char newDirection = (char)_getch();
		if (newDirection == 'W' || newDirection == 'w')		
			snakeDirection = Up;
		else if (newDirection == 'D' || newDirection == 'd')
			snakeDirection = Right;
		else if (newDirection == 'S' || newDirection == 's')
			snakeDirection = Down;
		else if (newDirection == 'A' || newDirection == 'a')
			snakeDirection = Left;
		else if (newDirection == ' ')
			system("Pause");
		Sleep(sleepDuration - duration * 1000); // conversion from seconds to microseconds
	}
	
	//if (sleepDuration > 50 && (score+1) % 10 == 0)
	{
	///	sleepDuration -= 25;
		//score = score + (gameDifficulty == Hard?3:1);
	}
}
void setConsoleDimension()
{
	string mode = "mode " + to_string(MAP_WIDTH) + ',' + to_string(MAP_HEIGHT);
	system(mode.c_str());   //Set mode to ensure window does not exceed buffer size
	SMALL_RECT WinRect = { 0, 0, MAP_WIDTH, MAP_HEIGHT };   //New dimensions for window in 8x12 pixel chars
	SMALL_RECT* WinSize = &WinRect;
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), true, WinSize);   //Set new size for window
}

void clearConsole()
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	//COORD Position = { 0,0 };
	//hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hOut, { 0,0 }); //Position);
}

void startGame()
{
	char difficulty;
	do {
		difficulty = (char) _getch();
		if (difficulty == '1')
			gameDifficulty = Easy;
		else if (difficulty == '2')
			gameDifficulty = Hard;
	} while (difficulty != '1' && difficulty != '2');
	system("cls");

	for (int i = 0; i < MAP_HEIGHT; i++)
		for (int j = 0; j < MAP_WIDTH; j++)
			map[i][j] = '\0';

	setConsoleDimension();
	lostGame = false;
	foodExists = false;
	eaten = false;
	score = 0;
	snake.clear();
	generateMapBorder();
	generateSnake();
	snakeDirection = Right;
	sleepDuration = SLEEP_DURATION;

	do {
		drawSnake();
		if(!foodExists)
			drawFood();
		if (lostGame)
		{
			cout << "Ai pierdut! Scorul tau este: " << score << endl;
			system("Pause");
			break;
		}
		drawMap();
		initKeyListener();
		checkNextPosition();
		clearConsole();
		if (lostGame == true)
			Sleep(1000);
	} while (!lostGame);
	main();
}

void drawAscii()
{
	for (int i = 0; i < 256; i++)
		cout << i << " -> " << char(i) << endl;
}

void initSeedFile()
{
	Cryptography::GenerateMatrix();
	ofstream f("seed.txt");
	f<<Cryptography::ConvertMatrixToBase36String();
}

int main()
{
	initConsole();
	drawMenu();
	srand(time(0));
	startGame();
	cin.ignore();
	return 0;
}