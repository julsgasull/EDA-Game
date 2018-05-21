#include "Player.hh"

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */

#define PLAYER_NAME Juls_v0


// DISCLAIMER: The following Demo player is *not* meant to do anything
// sensible. It is provided just to illustrate how to use the API.
// Please use AINull.cc as a template for your player.



//void pathFinder(Position& Begin, Position& End, vector< vector<int> >& d, list<Position>& p, bool W)
//{ //if W = treu the unit can fly
//	d = vector< vector<int> >(60, vector<int>(60));
//	vector< vector<int> > S(60, vector<int>(60));
//	priority_queue<Position, vector<Position>, greater<Position> > Q;
//	Q.push(Begin);
//
//	while(not Q.empty())
//	{
//		auto u = Q.top();
//		Q.pop();
//		if(not S[u.i][u.j])
//		{
//			S[u.i][u.j] = true;
//			for (int x = 0; x < 8; ++x)
//			{
//				Position v = u + adj[x];
//				int c = what(v.i, v.j);
//				if( not((W or c == WATER) or c == MOUNTAIN))
//				{
//					if(d[v.i][v.j] > d[u.i][u.j] + 1)
//					{
//						d[v.i] = d[u.i] + 1;
//						d[v.j] = d[u.j] + 1;
//						p.insert(u, p.end());
//					}
//				}
//			}
//		}
//	}
//}


struct PLAYER_NAME : public Player {

	/**
	* Factory: returns a new instance of this class.
	* Do not modify this function.
	*/
	static Player* factory ()
	{
		return new PLAYER_NAME;
	}

	/**
	* Types and attributes for your player can be defined here.
	*/

	static constexpr int I[8] = { 1, 1, 0, -1, -1, -1,  0,  1 };
	static constexpr int J[8] = { 0, 1, 1,  1,  0, -1, -1, -1 };
	
	static constexpr int I_h[8] = { 1, 0, -1,  0 };
	static constexpr int J_h[8] = { 0, 1,  0, -1 };

	
	/*
	 1. Si adjacent -> ataco
	 2. En un post meu ocupat per alguns	//TO-DO: quants
	 3. Per a cada soldat que no estigui a un post -> mirar post mes proper:
	 		si no és teu -> hi vas
	 		si és teu -> mirar quanta gent hi ha -> si suficients, buscar seguent post
	 (tenir en compte quanta punts)
	*/
  	void play_soldier(int id)	//mira adjacents i si pot, ataca. si no pot, es mou amb random
	{
		Data in = data(id);
		int i = in.pos.i;
		int j = in.pos.j;
		int player = in.player;
		int ii, jj;
		
		for (int k = 0; k < 8; ++k)	//totes les posicions adjacents
		{
			ii = i + I[k];
		  	jj = j + J[k];
		  	if (pos_ok(ii,jj))
			{
				int id2 = which_soldier(ii, jj);
				if (id2 and data(id2).player != player) //enemic al costat -> ataquem
				{
				  command_soldier(id, ii, jj);
				  return;
				}
		  	}
    	}
		
		//MODIFICAR -> BORRAR
		//pilla un random en cas de no poder atacar
    	ii = i + random(-1, 1);
		jj = j + random(-1, 1);
		
		if (pos_ok(ii, jj)) command_soldier(id, ii, jj);
		
		//EL QUE REALMENT HAIG DE FER
		//buscar cami -> dijkstra o com putes s'escrigui
		//posarli proxima posicio a fer
  	}
	
	
/*
	0			29			59
0	-------------------------
	|			|			|
	|			|			|
	|	  0		|	  1		|
	|			|			|
	|			|			|
29	-------------------------
 	|			|			|
	|			|			|
	|	  2		|	  3		|
	|			|			|
	|			|			|
59	-------------------------
*/
	int quadrant(int i, int j)
	{
		if (( 0 <= i and i <= 29) and ( 0 <= j and j <= 29)) return 0;
		if ((30 <= i and i <= 59) and ( 0 <= j and j <= 29)) return 1;
		if (( 0 <= i and i <= 29) and (30 <= j and j <= 59)) return 2;
		if ((30 <= i and i <= 59) and (30 <= j and j <= 59)) return 3;
		return -1;
	}

	
	/*
	 1. Si no estic al meu quadrant / a sota hi tinc més jugadors -> tiro napalm
	 2. Anar cap als llocs dels enemics de 2 en 2 (adjacents), perque no pots anar en diagonal
	 3. Si arribes al final / muntanya, girar 90 graus.
	*/
	void play_helicopter(int id)	//tira napalm si pot i es va movent endavant (20% de girar clockwise)
	{
		Data in = data(id);
		
		/*------- 1 -------*/
		if (in.napalm == 0) //data(id).napalm = 0 quan no t'has desperar rondes per llençar-lo
		{
			int own = 0;
			int others = 0;
			for (int ii = in.pos.i - 2; ii <= in.pos.i + 2; ii++)
			{
				for (int jj = in.pos.j - 2; jj <= in.pos.j +2; jj++)
				{
					if (pos_ok(ii, jj))
					{
						int s = which_soldier(ii, jj);
						if (s > 0) //hi ha un soldat
						{
							Data d = data(s);
							if (d.player != me()) ++others;
							else ++own;
						}
					}
				}
			}
			if (quadrant(in.pos.i, in.pos.j) != me() or others > own)
			{
				command_helicopter(id, NAPALM);
				return;
			}
		}
		/*------- /1 -------*/
		
		/*------- 2 -------*/
		int ii = in.pos.i;
		int jj = in.pos.j;
		if (pos_ok(ii, jj))
		{
			int o = in.orientation;
			if ((o == 0 and pos_ok(ii - 1, jj) and what(ii - 1, jj) != 4)	//SOUTH
				or
				(o == 1 and pos_ok(ii, jj + 1) and what(ii, jj + 1) != 4)	//EAST
				or
				(o == 2 and pos_ok(ii + 1, jj) and what(ii + 1, jj) != 4)	//NORTH
				or
				(o == 3 and pos_ok(ii, jj - 1) and what(ii, jj - 1) != 4)	//WEST
				)
			{
				command_helicopter(id, FORWARD2); //si no pot fer 2, en fara 1
			}
		/*------- /2 -------*/
		/*------- 3 -------*/
			else
			{
				int q = quadrant(ii, jj);
				if (o == 0 or o == 2)	//SOUTH or NORTH
				{
					//no puc passar -> vaig al de la dreta
					if (q == 0 or q == 2) command_helicopter(id, COUNTER_CLOCKWISE);
					//no puc passar -> vaig al de l'esquerra
					else if (q == 1 or q == 3) command_helicopter(id, CLOCKWISE);
					
				}
				else if (o == 1 or o == 3) //EAST or WEST
				{
					//mo puc passar -> vaig al d'abaix
					if (q == 0 or q == 1) command_helicopter(id, CLOCKWISE);
					//no no puc passar -> vaig al de dalt
					else if (q == 2 or q == 3) command_helicopter(id, COUNTER_CLOCKWISE);
				}
			}
			return;
		}
		/*------- /3 -------*/

		//MODIFICAR
		// With probability 20% we turn counter clockwise,
		// otherwise we try to move forward two steps.
		int c = random(1, 5);
		command_helicopter(id, c == 1 ? COUNTER_CLOCKWISE : FORWARD2);
  	}

	void throw_parachuter(int helicopter_id) //pilla posicio random dins del helicopter, i el tira
	{
		if (round()%2 == 0)	//cada 2 rondes per no haver d'estar tot el rato tirant
		{
			Data in = data(helicopter_id);
			for (int ii = in.pos.i - 2; ii <= in.pos.i + 2; ii++)
			{
				for (int jj = in.pos.j - 2; jj <= in.pos.j + 2; jj++)
				{
					if (fire_time(ii, jj) == 0 and what(ii, jj) != 3)
					{ //no fire and correct position && no water (no pot ser mountain perq no hi van els helicopters)
						int s = which_soldier(ii, jj);
						if (s == 0 or s == -1)	//no hi ha soldats abaix
						{
							command_parachuter(ii, jj);
							return;
						}
					}
				}
			}
		}
  	}
  
	/**
	* Play method, invoked once per each round.
	*/
  	virtual void play ()
	{

    	int player = me();
    	vector<int> H = helicopters(player); 	// helicopters of my player
    	vector<int> S = soldiers(player); 		// soldiers of my player
	
		//1r parachuters
		//MODIFICAR
		// If in a random helicopter I have parachuters, I throw one.
		int helicopter_id = H[random(0, int(H.size()-1))];
		if (not data(helicopter_id).parachuters.empty()) throw_parachuter(helicopter_id);
		
		//2n helicopters
		for (int i = 0; i < int(H.size()); ++i) play_helicopter(H[i]);
		
		//3r soldats
		for (int i = 0; i < int(S.size()); ++i) play_soldier(S[i]);
  	}

};

constexpr int PLAYER_NAME::I[8];
constexpr int PLAYER_NAME::J[8];

/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
