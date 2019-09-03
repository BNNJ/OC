using System;

/*
** Pas de fioritures :
** On crée un tableau de 7 ints, on génere un nombre aléatoire
** On vérifie si il est déjà dans le tableau, si c'est le cas
** on en génere un nouveau, sinon on le met dans le tableau et on passe
** à la case suivante.
** Quand il est plein, on affiche chaque élément sur une nouvelle ligne,
** parce que c'est plus clair.
**
** Comment fonctionne la vérification des doublons ?
** La méthode Array.Exists() prend un tableau et un prédicat en argument.
** Le prédicat est passé sous forme de lambda expression:
** e => e == n
** En toutes lettres:
** pour chaque élément e (dans tab), si e est égal à n
** C'est facile, clair, et ca marche bien.
*/

class Program
{
	static void Main(string[] args)
	{
		Random	rand = new Random();
		int[]	tab = new int[7];
		int		n;
		
		for (int i = 0; i < 7; ++i)
		{
			do	n = rand.Next(1, 50);
			while (Array.Exists(tab, e => e == n));
			tab[i] = n;
		}
		foreach (int i in tab)
			Console.WriteLine(i);
	}
}
