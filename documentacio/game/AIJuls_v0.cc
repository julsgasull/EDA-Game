#include "Player.hh"

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */

#define PLAYER_NAME Juls_v0


// DISCLAIMER: The following Demo player is *not* meant to do anything
// sensible. It is provided just to illustrate how to use the API.
// Please use AINull.cc as a template for your player.


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
		
		//MODIFICAR
		//pilla un random en cas de no poder atacar
    	ii = i + random(-1, 1);
		jj = j + random(-1, 1);
		
		if (pos_ok(ii, jj)) command_soldier(id, ii, jj);
		
		//buscar cami
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
	bool in_my_quadrant(int i, int j)
	{
		int pl = me();
		
		switch (pl) {
			case 0:
				if ((0 <= i and i <= 29) and (0 <= j and j <= 29)) return true;
				return false;
				break;
			case 1:
				if ((30 <= i and i <= 59) and (0 <= j and j <= 29)) return true;
				return false;
				break;
			case 2:
				if ((0 <= i and i <= 29) and (30 <= j and j <= 59)) return true;
				return false;
				break;
			case 3:
				if ((30 <= i and i <= 59) and (30 <= j and j <= 59)) return true;
				return false;
				break;
			default:
				return false;
				break;
		}
	}
	
//	int num_posts_conq(int pl)
//	{
//		int sum = 0;
//		vector<Post> p = posts();
//		for (int i = 0; i < NUM_POSTS; i++)
//		{
//			if (p[i].player == pl) ++sum;
//		}
//		return sum;
//	}
	
	void play_helicopter(int id)	//tira napalm si pot i es va movent endavant (20% de girar clockwise)
	{
		Data in = data(id);
		if (in.napalm == 0) { //data(id).napalm = 0 quan no t'has desperar rondes per llençar-lo
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
			if (not in_my_quadrant(in.pos.i, in.pos.j) or others > own)
			{
				command_helicopter(id, NAPALM);
				return;
			}
		}
		

		//MODIFICAR
		// With probability 20% we turn counter clockwise,
		// otherwise we try to move forward two steps.
		int c = random(1, 5);
		command_helicopter(id, c == 1 ? COUNTER_CLOCKWISE : FORWARD2);
  	}

	void throw_parachuter(int helicopter_id) //pilla posicio random dins del helicopter, i el tira
	{ //MODIFICAR
    	// We get the data of the helicopter...
    	Data in = data(helicopter_id);
    	// ... and try to throw a parachuter, without even EXAMINING THE LAND
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
