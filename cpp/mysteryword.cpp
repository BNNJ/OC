#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <ctime>

using namespace std;

#define	MAX_TRY	5


/* /!\ Lisez le fichier texte joint avant la source s'il vous plait ! /!\ */

/*
** Initialise le tableau global "dictionnary" grace à un fichier texte.
** Si le fichier n'existe pas, la possibilité est donné à l'utilisateur de
** spécifier un fichier à utiliser, ou de simplement continuer en mode 2 joueurs.
** A noter qu'aucune vérification n'est faite sur le fichier spécifié.
** En revanche, chaque ligne est vérifiée avant d'être ajoutée au tableau:
** Si elle contient des caractères non imprimables (ascii < 32 et > 126),
** elle est ignorée.
**
** Les éventuels '\r' sont supprimés:
** Il s'agit de characteres spéciaux, les retours chariots. Ils sont ajoutés
** à la fin de chaque ligne dans les fichiers généré sous windows.
** Il faut donc les supprimer avant de stocker le mot dans le tableau
** A noter que les systemes UNIX utilisent un simple '\n' pour le newline,
** alors que Windows utilise '\r' ET '\n': retour en début de ligne,
** puis une nouvelle ligne.
*/

vector<string>	dictionary;

static bool		contains_non_printable(string s)
{
	for (size_t i = 0; i < s.size(); ++i)
		if (s[i] < 32 || s[i] > 126)
			return (true);
	return (false);
}

static bool		init_dictionary()
{
	ifstream		is("./dico.txt");
	string			buffer;

	if (!is)
	{
		cout << "Warning: Failed to open dictionary. 1 player mode unavailable.\n"
			<< "Please specify another file to use as dictionnary,\n"
			<< "or press ENTER to default to 2 players mode and continue...\n> ";
		getline(cin, buffer);
		is.open(buffer, ifstream::in);
		if (!is)
			return (false);
	}
	while (getline(is, buffer))
	{
		if (buffer[buffer.size() - 1] == '\r')
			buffer.erase(buffer.size() - 1, 1);
		if (!contains_non_printable(buffer))
			dictionary.push_back(buffer);
	}
	if (dictionary.empty())
	{
		cout << "The dictionary is empty, 1 player mode unavailable.\n"
			<< "Press ENTER to continue" << endl;
	}
	return (true);
}

static int		get_nbplayers()
{
	string	buffer;

	cout << "Select game mode (type 1 or 2) :\n\t1: 1 player\n\t2: 2 players\n> ";
	getline(cin, buffer);
	while ((buffer != "1" && buffer != "2"))
	{
		cout << "Invalid game mode. Please enter 1 or 2\n> ";
		getline(cin, buffer);
	}
	return (buffer[0] - '0');
}

/*
** Récupère un mot dans un fichier texte pour le mode 1 joueur,
** ou demande à un joueur d'entrer un mot ou une phrase pour le mode 2 joueurs.
** Les éventuels espaces superflus sont supprimés :
** En début et fin de phrase, et plusieurs espace consécutifs entre deux mots
** sont réduits à un seul.
**
** Pour le mode deux joueurs, le mot est remplacé par des étoiles, pour que
** l'autre joueur ne puisse pas le voir.
** C'est fait en utilisant le code \33[A, qui est un code ANSI interprété
** par le terminal, lui demandant de remonter d'une ligne,
** puis par le caractère '\r', qui est un retour chariot, remettant le curseur
** au début de la ligne.
** Il ne reste ensuite plus qu'à afficher les étoiles par dessus le mot.
** Pour avoir le bon nombre d'étoiles, j'utilise la fonction memset() qui prend
** un pointeur vers le premier byte de la zone mémoire à modifier,
** un byte avec lequel remplir la mémoire et le nombre de bytes à modifier :
** Le pointeur est obtenu par la méthode .data() de la classe string,
** le byte est le code ascii de l'ètoile,
** et le nombre de bytes est la taille de la string à remplacer.
** J'ai testé cette méthode sous Linux Manjaro et le powershell de Windows 10.
*/

static string	remove_extra_spaces(string w)
{
	string str;

	while (!w.empty())
	{
		if (w[0] != ' ' || (w[1] != ' ' && w.size() > 1 && !str.empty()))
			str += w[0];
		w.erase(0, 1);
	}
	return (str);
}

static string	get_word(int nb_players)
{
	string					word;

	if (nb_players == 2)
	{
		cout << "Please enter a word for the other player to guess:\n> ";
		do
		{
			getline(cin, word);
			if (word.empty())
				cout << "You didn't enter a word ! Please do so:\n> ";
		} while (word.empty());
		word = remove_extra_spaces(word);
		string	buffer = word;
		memset((void*)buffer.data(), '*', buffer.size());
		cout << "\33[A\r> " << buffer << endl;
	}
	else
		word = remove_extra_spaces(dictionary[rand() % dictionary.size()]);
	return (word);
}

/*
** Les arguments sont une question et un return par defaut.
** L'utilisateur doit répondre à la question par oui ou par non,
** ou juste appuyer sur entrée pour utiliser la réponse par défaut.
*/

static bool		ask_user(string question, bool d)
{
	string	buffer;

	cout << question << " (Y/N, default " << (d ? "Y)" : "N)") <<"\n> ";
	getline(cin, buffer);
	while ((strchr("YyNn", buffer[0]) == NULL || buffer.size() > 1)
		&& !buffer.empty())
	{
		cout << "Invalid answer, please type Y or N\n> ";
		getline(cin, buffer);
	}
	return (buffer.empty() ? d : buffer[0] == 'Y' || buffer[0] == 'y');
}

/*
** Mélange les lettres de chaque mot, mot par mot,
** en gardant les espaces en place.
** par exemple, "ABCDE 12345 abcde" peut devenir "CEDBA 21354 dbcae"
** Le mélange est fait de la facon suivante:
** 1) calcul de la taille du premier mot de la string word
** 		(donc jusqu'à la fin ou jusqu'à un espace)
** 2) génération d'un nombre n aléatoire entre 0 et la taille du mot
** 3) ajout de la lettre se trouvant l'index n de word à la fin de scrambled
**		grace à ĺ'operateur += de la classe string
** 4) suppression de la lettre dans word grace à la méthode .erase()
** 5) si il y a un espace au début de word, on le supprime et on le met
**		dans scrambled
** Et on recommence jusqu'à ce que word soit vide
*/

static string	scramble(string word)
{
	string	scrambled;
	int		n;

	while (!word.empty())
	{
		int	len = 0;
		while (word[len] && word[len] != ' ')
			++len;
		while (--len >= 0)
		{
			n = len > 0 ? rand() % len : 0;
			scrambled += word[n];
			word.erase(n, 1);
		}
		if (word[0] == ' ')
		{
			scrambled += ' ';
			word.erase(0, 1);
		}
	}
	return (scrambled);
}

/*
** La fonction scramble() mélange les lettres du mot et renvoie le résultat,
** Puis le joueur est invité à entrer un mot.
** Il peut aussi entrer "word" pour que le mot s'affiche, ce qui permet de
** tester que la reconnaissance du mot fonctionne correctement même si
** on arrive pas à deviner le mot tout seul,
** ou entrer "hint" pour avoir un indice (la première lettre du mot,
** puis la suivante, etc).
*/

static void		play(string word)
{
	string			guess;
	int				nb_tries = MAX_TRY;
	int				nb_hint = 1;
	const string	scrambled = scramble(word);

	cout << "The mystery word is: " << scrambled << "\nYour guess:\n";
	while (--nb_tries >= 0)
	{
		cout << "> ";
		getline(cin, guess);
		if (guess == "word")
		{
			cout << "The word is: " << word << endl;
			++nb_tries;
		}
		else if (guess == "hint")
		{
			cout << word.substr(0, nb_hint) << "\nYou have " << nb_tries
				<< (nb_tries == 1 ? " try left" : " tries left") << endl;
			++nb_hint;
		}
		else if (guess == word)
		{
			cout << "Well played, you found the word in " << MAX_TRY - nb_tries
				<< (MAX_TRY - nb_tries == 1 ? " try !" : " tries !") << endl;
			break ;
		}
		else
		{
			if (nb_tries == 0)
				cout << "You're out of tries, sorry, you lost\nthe word was "
					<< word << endl;
			else if (nb_tries == 1)
				cout << "Wrong answer, it's your last try" << endl;
			else
				cout << "Wrong answer, you have " << nb_tries << " tries left"
					<< endl; 
		}
	}
}

/*
** Simplement pour vider le terminal en fonction de l'OS:
** cls pour windows, clear pour les systemes UNIX
*/

static void		clear()
{
	#if defined _WIN32
	    system("cls");
	#elif defined (__LINUX__) || defined (__gnu_linux__) || defined (__linux__)
	    system("clear");
	#elif defined (__APPLE__)
	    system("clear");
	#endif
}

/*
** Tout d'abord, l'initialisation du tableau de mots (qui est une variable
** globale) à partir du fichier dico.txt.
** Si cette initialisation rate, par exemple parce que le fichier n'existe pas,
** Le mode 1 joueur n'est pas disponible
**
** Il ne reste plus qu'à proposer le mode de jeu, jouer, et recommencer si
** les joueurs le veulent. Il est aussi proposé en fin de partie de changer
** le nombre de joueurs, si le mode solo est disponible (c'est à dire si
** le dictionnaire a été chargé correctement)
*/

int				main()
{
	ios::sync_with_stdio(false);
	clear();

	bool	keep_playing = true;
	bool	restart = true;
	int		nb_players = 2;
	bool	dict_loaded = init_dictionary();

	srand(time(NULL));
	while (keep_playing)
	{
		clear();
		if (restart && dict_loaded)
			nb_players = get_nbplayers();
		restart = false;
		play(get_word(nb_players));
		if ((keep_playing = ask_user("Do you want to play again ?", true))
			&& dict_loaded)
			restart = ask_user("Do you want to change the game mode ?", false);
	}
	return (EXIT_SUCCESS);
}
