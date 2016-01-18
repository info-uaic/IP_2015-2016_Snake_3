#include <string>
using namespace std;
class Crypter {
public:
	/*Aceasta functie seteaza cheia de verificare a criptarii
	Seedul trebuie sa fie >= 100*/	
	static void setSeed(unsigned seed);
	
	/*Aceasta functie seteaza lungimea sirului criptat*/
	static void setDifficulty(unsigned difficulty);

	/*Aceasta functie cripteaza un numar.
	Rezultatul va fi un string de lungime = dfficulty*/
	static std::string cryptNumber(unsigned number);

	/*Aceasta functie decripteaza un string*/
	static void decrypt(std::string input);
private:
	static unsigned seed, difficulty;
}; 