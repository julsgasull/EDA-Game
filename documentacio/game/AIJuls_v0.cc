#include "Player.hh"

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */

#define PLAYER_NAME Juls_v0


// DISCLAIMER: The following Demo player is *not* meant to do anything
// sensible. It is provided just to illustrate how to use the API.
// Please use AINull.cc as a template for your player.



/*
 struct Position {
 	int r, c;
 	Position(int r, int c) {
 		this->r = r;
 		this->c = c;
 	}
 };
 */

vector< vector<char> > Board (60, vector<char> (60, '.'));


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

	
	void scan()
	{
		int mi = me();
		for (int i = 0; i < 60; i++)
		{
			for (int j = 0; j < 60; j++)
			{
				if (Board[i][j] != 'X')
				{
					int p = post_owner(i, j);
					int id2 = which_soldier(i, j);
					
					if (what(i, j) > 2) Board[i][j] = 'X';							// water or mountain
					else if (fire_time(i, j) > 0) Board[i][j] = 'F';				// fire
					else if (id2 > 0 and data(id2).player != mi) Board[i][j] = 'E';	// enemy
					else if (p != -2 and p != mi) Board[i][j] = 'P';				// post
					else Board[i][j] = '.';											// grass or forest
				}
			}
		}
	}
	
	struct Node
	{
		int parent_i, parent_j;
		int f, g, h;
	};
	
	bool isValid(int row, int col, int id)	//si soldat pot passar
	{
		auto soldaditus = soldiers(me());
		for (auto& soldadu : soldaditus)
		{
			if (id != soldadu) {
				auto mi_suldadu = data(soldadu);
				if (mi_suldadu.pos.i == row and mi_suldadu.pos.j == col)
					return false;
			}
			
		}
		return (pos_ok(row, col) and what(row, col) <= 2 and fire_time(row, col) == 0);
	}
	
	bool isDestination (const int i, const int j, const Position &destination)
	{
		return i == destination.i and j == destination.j;
	}
	
	int calculateHValue(int row, int col, Position dest)
	{
		return ((row - dest.i)*(row-dest.i) + (col-dest.j)*(col-dest.j));
	}
	
	
	Position tracePath(vector< vector<Node> > &cellDetails, Position &dst, Position &src)
	{
		int row = dst.i;
		int col = dst.j;
		
		// cerr << "tracing path...";
		
		stack<pair<int, int>> path;
		
		while (not (cellDetails[row][col].parent_i == row and cellDetails[row][col].parent_j == col))
		{
			// cerr << "in row: " << row << " col: " << col << endl;
			path.push(make_pair(row, col));
			int temp_row = cellDetails[row][col].parent_i;
			int temp_col = cellDetails[row][col].parent_j;
			row = temp_row;
			col = temp_col;
			// cerr << "out row: " << row << " col: " << col << endl;
		}
		
		Position p;
		p.i = path.top().first;
		p.j = path.top().second;
		
		// cerr << "done";
		
		return p;
	}
	
	
	// A*
	Position aStarSearch(Position src, Position dest, int id)
	{
		if (isDestination(src.i, src.j, dest)) return src;
		if (not isValid (src.i, src.j, id) or not isValid (dest.i, dest.j, id)) return Position(-1, -1);
		vector< vector<bool> > closedList (60, vector<bool> (60, false));
		// cerr << "(" << src.i << " " << src.j << ") (" << dest.i << " " << dest.j << ")" << endl;
		Node n;
		n.parent_i = -1;
		n.parent_j = -1;
		n.f = INT_MAX;
		n.g = INT_MAX;
		n.h = INT_MAX;
		vector< vector<Node> > cellDetails(60, vector<Node> (60, n));
		
		cellDetails[src.i][src.j].f        = 0;
		cellDetails[src.i][src.j].g        = 0;
		cellDetails[src.i][src.j].h        = 0;
		cellDetails[src.i][src.j].parent_i = src.i;
		cellDetails[src.i][src.j].parent_j = src.j;
		
		set< pair<int, pair<int, int> > > openList;
		openList.insert(make_pair(0, make_pair(src.i, src.j)));
		bool foundDest = false;
		
		while (not openList.empty())
		{
			pair< int, pair<int, int> > p = *openList.begin();
			openList.erase(openList.begin());
			int i = p.second.first;
			int j = p.second.second;
			closedList[i][j] = true;
			
			int new_f;
			int new_g;
			int new_h;
			
			/*
			 Generating all the 8 successor of this cell
			 
			 N.W   N   N.E
			  \    |    /
			   \   |   /
			 W----Cell----E
			   /   |   \
			  /    |    \
			 S.W    S   S.E
			 
			 Cell-->Popped Cell 	(i, j)
			 N -->  North       	(i-1, j)
			 S -->  South       	(i+1, j)
			 E -->  East        	(i, j+1)
			 W -->  West         	(i, j-1)
			 N.E--> North-East  	(i-1, j+1)
			 N.W--> North-West  	(i-1, j-1)
			 S.E--> South-East  	(i+1, j+1)
			 S.W--> South-West  	(i+1, j-1)
			 */
			
			
			// NORTH
			if (isValid(i-1, j, id))
			{
				// cerr << "north" << endl;
				if (isDestination(i - 1, j, dest))
				{	// Es mi destino
					cellDetails[i - 1][j].parent_i = i;
					cellDetails[i - 1][j].parent_j = j;
					foundDest = true;
					return tracePath(cellDetails, dest, src);
				}
				else if (not closedList[i-1][j])
				{	// No es mi destino y hay que iterarlo
					new_g = cellDetails[i][j].g + 1;
					new_h = calculateHValue (i-1, j, dest);
					new_f = new_g + new_h;
					
					if (cellDetails[i-1][j].f == INT_MAX or cellDetails[i-1][j].f > new_f)
					{
						openList.insert(make_pair(new_f, make_pair(i - 1, j)));
						
						cellDetails[i - 1][j].f        = new_f;
						cellDetails[i - 1][j].g        = new_g;
						cellDetails[i - 1][j].h        = new_h;
						cellDetails[i - 1][j].parent_i = i;
						cellDetails[i - 1][j].parent_j = j;
					}
				}
			}
			
			// SOUTH
			if (isValid(i+1, j, id))
			{
				// cerr << "south" << endl;
				if (isDestination(i + 1, j, dest))
				{
					cellDetails[i + 1][j].parent_i = i;
					cellDetails[i + 1][j].parent_j = j;
					foundDest = true;
					return tracePath(cellDetails, dest, src);
				}
				else if (not closedList[i+1][j])
				{
					new_g = cellDetails[i][j].g + 1;
					new_h = calculateHValue (i+1, j, dest);
					new_f = new_g + new_h;
					
					if (cellDetails[i+1][j].f == INT_MAX or cellDetails[i+1][j].f > new_f)
					{
						openList.insert(make_pair(new_f, make_pair(i + 1, j)));
						cellDetails[i + 1][j].f        = new_f;
						cellDetails[i + 1][j].g        = new_g;
						cellDetails[i + 1][j].h        = new_h;
						cellDetails[i + 1][j].parent_i = i;
						cellDetails[i + 1][j].parent_j = j;
					}
				}
			}
			
			// EAST
			if (isValid (i, j+1, id))
			{
				// cerr << "east" << endl;
				if (isDestination(i, j+1, dest))
				{
					cellDetails[i][j + 1].parent_i = i;
					cellDetails[i][j + 1].parent_j = j;
					foundDest = true;
					return tracePath(cellDetails, dest, src);
				}
				else if (not closedList[i][j+1])
				{
					new_g = cellDetails[i][j].g + 1;
					new_h = calculateHValue (i, j+1, dest);
					new_f = new_g + new_h;
					
					if (cellDetails[i][j+1].f == INT_MAX or cellDetails[i][j+1].f > new_f)
					{
						openList.insert(make_pair(new_f, make_pair(i, j + 1)));
						cellDetails[i][j + 1].f        = new_f;
						cellDetails[i][j + 1].g        = new_g;
						cellDetails[i][j + 1].h        = new_h;
						cellDetails[i][j + 1].parent_i = i;
						cellDetails[i][j + 1].parent_j = j;
					}
				}
			}
			
			// WEST
			if (isValid (i, j-1, id))
			{
				// cerr << "west" << endl;
				if (isDestination(i, j-1, dest))
				{
					cellDetails[i][j - 1].parent_i = i;
					cellDetails[i][j - 1].parent_j = j;
					foundDest = true;
					return tracePath(cellDetails, dest, src);
				}
				else if (not closedList[i][j-1])
				{
					new_g = cellDetails[i][j].g + 1;
					new_h = calculateHValue (i, j-1, dest);
					new_f = new_g + new_h;
					
					if (cellDetails[i][j-1].f == INT_MAX or cellDetails[i][j-1].f > new_f)
					{
						openList.insert(make_pair(new_f, make_pair(i, j - 1)));
						cellDetails[i][j - 1].f        = new_f;
						cellDetails[i][j - 1].g        = new_g;
						cellDetails[i][j - 1].h        = new_h;
						cellDetails[i][j - 1].parent_i = i;
						cellDetails[i][j - 1].parent_j = j;
					}
				}
			}
			
			// NORTH-EAST
			if (isValid(i-1, j+1, id))
			{
				// cerr << "north east" << endl;
				if (isDestination(i-1, j+1, dest))
				{
					cellDetails[i - 1][j + 1].parent_i = i;
					cellDetails[i - 1][j + 1].parent_j = j;
					foundDest = true;
					return tracePath(cellDetails, dest, src);
				}
				else if (not closedList[i - 1][j + 1])
				{
					new_g = cellDetails[i][j].g + 2;
					new_h = calculateHValue(i - 1, j + 1, dest);
					new_f = new_g + new_h;
					
					if (cellDetails[i-1][j+1].f == INT_MAX or cellDetails[i-1][j+1].f > new_f)
					{
						openList.insert( make_pair (new_f, make_pair(i-1, j+1)));
						
						// Update the details of this cell
						cellDetails[i-1][j+1].f        = new_f;
						cellDetails[i-1][j+1].g        = new_g;
						cellDetails[i-1][j+1].h        = new_h;
						cellDetails[i-1][j+1].parent_i = i;
						cellDetails[i-1][j+1].parent_j = j;
					}
				}
			}
			
			// NORTH-WEST
			if (isValid (i-1, j-1, id))
			{
				// cerr << "north west" << endl;
				if (isDestination (i-1, j-1, dest))
				{
					cellDetails[i-1][j-1].parent_i = i;
					cellDetails[i-1][j-1].parent_j = j;
					foundDest = true;
					return tracePath(cellDetails, dest, src);
				}
				else if (not closedList[i-1][j-1])
				{
					new_g = cellDetails[i][j].g + 2;
					new_h = calculateHValue(i-1, j-1, dest);
					new_f = new_g + new_h;
					
					if (cellDetails[i-1][j-1].f == INT_MAX or cellDetails[i-1][j-1].f > new_f)
					{
						openList.insert( make_pair (new_f, make_pair (i-1, j-1)));
						cellDetails[i-1][j-1].f        = new_f;
						cellDetails[i-1][j-1].g        = new_g;
						cellDetails[i-1][j-1].h        = new_h;
						cellDetails[i-1][j-1].parent_i = i;
						cellDetails[i-1][j-1].parent_j = j;
					}
				}
			}
			
			// SOUTH-EAST
			if (isValid(i+1, j+1, id))
			{
				// cerr << "south east" << endl;
				if (isDestination(i+1, j+1, dest))
				{
					cellDetails[i+1][j+1].parent_i = i;
					cellDetails[i+1][j+1].parent_j = j;
					foundDest = true;
					return tracePath(cellDetails, dest, src);
				}
				else if (not closedList[i+1][j+1])
				{
					new_g = cellDetails[i][j].g + 2;
					new_h = calculateHValue(i+1, j+1, dest);
					new_f = new_g + new_h;
					
					if (cellDetails[i+1][j+1].f == INT_MAX or cellDetails[i+1][j+1].f > new_f)
					{
						openList.insert(make_pair(new_f, make_pair (i+1, j+1)));
						
						cellDetails[i+1][j+1].f        = new_f;
						cellDetails[i+1][j+1].g        = new_g;
						cellDetails[i+1][j+1].h        = new_h;
						cellDetails[i+1][j+1].parent_i = i;
						cellDetails[i+1][j+1].parent_j = j;
					}
				}
			}
			
			// SOUTH-WEST
			if (isValid (i+1, j-1, id))
			{
				// cerr << "south west" << endl;
				if (isDestination(i+1, j-1, dest))
				{
					// cerr << "he llegao" << endl;
					cellDetails[i+1][j-1].parent_i = i;
					cellDetails[i+1][j-1].parent_j = j;
					foundDest = true;
					return tracePath(cellDetails, dest, src);
				}
				else if (not closedList[i+1][j-1])
				{
					new_g = cellDetails[i][j].g + 2;
					new_h = calculateHValue(i+1, j-1, dest);
					new_f = new_g + new_h;
					
					if (cellDetails[i+1][j-1].f == INT_MAX or cellDetails[i+1][j-1].f > new_f)
					{
						openList.insert(make_pair(new_f, make_pair(i+1, j-1)));
						cellDetails[i+1][j-1].f        = new_f;
						cellDetails[i+1][j-1].g        = new_g;
						cellDetails[i+1][j-1].h        = new_h;
						cellDetails[i+1][j-1].parent_i = i;
						cellDetails[i+1][j-1].parent_j = j;
					}
				}
			}
		}
		// cerr << "wtf" << endl;
		return Position(-1,-1);
	}
	
	
	
//	//mat contains the index's previous position that leads you to fin
//	//returns the next position you must go from curr to go to fin
//	Position getNextPos(vector< vector<Position> > &mat, Position fin, Position curr)
//	{
//		while(curr.i != fin.i and curr.j != fin.j)
//		{
//			if (mat[curr.i][curr.j].i == fin.i and mat[curr.i][curr.j].j == fin.j) return curr;
//			curr.i = mat[curr.i][curr.j].i;
//			curr.j = mat[curr.i][curr.j].j;
//		}
//		if(curr.i == fin.i and curr.j == fin.j) return curr;
//		return Position(-1,-1);
//	}
//
//	/*Returns the post position*/
//	Position bfs(const Position &inicial)
//	{
//		Position ini = inicial;
//		vector<vector<bool>> mat_vis (60, vector<bool>(60, false));						// Visited position matrix
//		vector<vector<Position>> mat_pos (60, vector<Position>(60, Position(-1,-1))); 	// Previous Pos matrix (each index of the matrix contains its predecesor)
//		queue<Position> cua_pos;														// Queue that contains the Pos's to visit
//		cua_pos.push(ini);
//		int mi = me();
//
//		while (!cua_pos.empty())
//		{
//			Position currPos = cua_pos.front();
//			cua_pos.pop();
//			if (pos_ok(currPos) and not mat_vis[currPos.i][currPos.j])
//			{
//				mat_vis[currPos.i][currPos.j] = true;
//
//				int winplr = 0;
//				for (int pl = 1; pl < NUM_PLAYERS; ++pl)
//				{
//					if (pl != mi and total_score(winplr) <= total_score(pl)) winplr = pl;
//				}
//
//				int c = post_owner(currPos.i, currPos.j);		//POST
//				if (c != -2 and c != mi)
//				{
//					if (random (0, 3))
//					{
//						Position nextPos = getNextPos(mat_pos, ini, currPos);
//						return nextPos;
//					}
//					else if (c == winplr or c == -1)	// si guanyo o no esta ocupat
//					{
//						Position nextPos = getNextPos(mat_pos, ini, currPos);
//						return nextPos;
//					}
//				}
//
//				for (int k = 0; k < 8; ++k)
//				{
//					Position p;
//					p.i = currPos.i + I[k];
//					p.j = currPos.j + J[k];
//
//					if (pos_ok(p) and not mat_vis[p.i][p.j])
//					{
//						cua_pos.push(p);
//						mat_pos[p.i][p.j] = currPos;
//					}
//				}
//			}
//
//		}
//		return Position(-1,-1);
//	}
	

	
//	/*Returns the post position*/
//	Position bfs(const Position &inicial)
//	{
//		Position ini = inicial;
//		vector<vector<bool>> mat_vis (60, vector<bool>(60, false));						// Visited position matrix
//		vector<vector<Position>> mat_pos (60, vector<Position>(60, Position(-1,-1))); 	// Previous Pos matrix (each index of the matrix contains its predecesor)
//		queue<Position> cua_pos;														// Queue that contains the Pos's to visit
//		cua_pos.push(ini);
//		int mi = me();
//
//		while (!cua_pos.empty())
//		{
//			Position currPos = cua_pos.front();
//			cua_pos.pop();
//			if (pos_ok(currPos) and not mat_vis[currPos.i][currPos.j])
//			{
//				mat_vis[currPos.i][currPos.j] = true;
//
//				int winplr = 0;
//				for (int pl = 1; pl < NUM_PLAYERS; ++pl) if (pl != mi and total_score(winplr) <= total_score(pl)) winplr = pl;
//
//				//POST or ENEMY
//				if (Board[currPos.i][currPos.j] == 'P' or Board[currPos.i][currPos.j] == 'E')
//				{
//					Position nextPos = getNextPos(mat_pos, ini, currPos);
//					return nextPos;
//					/*
//					if (random (0, 3))
//					{
//						Position nextPos = getNextPos(mat_pos, ini, currPos);
//						return nextPos;
//					}
//					else if (Board[currPos.i][currPos.j] == 'P' and (post_owner(currPos.i, currPos.j) == winplr or post_owner(currPos.i, currPos.j) == -1))	// si guanyo o no esta ocupat
//					{
//						Position nextPos = getNextPos(mat_pos, ini, currPos);
//						return nextPos;
//					}
//					 */
//				}
//				for (int k = 0; k < 8; ++k)
//				{
//					Position p;
//					p.i = currPos.i + I[k];
//					p.j = currPos.j + J[k];
//
//					int w = what(p.i, p.j);
//					if ((w == 1 or w == 2) and not mat_vis[p.i][p.j])
//					{
//						cua_pos.push(p);
//						mat_pos[p.i][p.j] = currPos;
//					}
//				}
//			}
//		}
//		return Position(-1,-1);
//	}
//

	
	/*
	 1. Si adjacent -> ataco
	 2. En un post meu ocupat per alguns	//TO-DO: quants
	 3. Per a cada soldat que no estigui a un post -> mirar post mes proper:
	 		si no és teu -> hi vas
	 		si és teu -> mirar quanta gent hi ha -> si suficients, buscar seguent post
	 (tenir en compte quanta punts)
	*/
	Position find_nearest_post(const Position& src)
	{
		auto postes = posts();
		int min_distance = INT_MAX;
		Position min_distance_pos = src;
		for (auto& post : postes)
		{
			int distance = calculateHValue(src.i, src.j, post.pos);
			if (distance < min_distance and post_owner(post.pos.i, post.pos.j) != -2 and post_owner(post.pos.i, post.pos.j) != me())
			{
				min_distance = distance;
				min_distance_pos = post.pos;
			}
		}
		return min_distance_pos;
	}
	
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
		//    	ii = i + random(-1, 1);
		//		jj = j + random(-1, 1);
		//
		//		if (pos_ok(ii, jj)) command_soldier(id, ii, jj);
		
		//EL QUE REALMENT HAIG DE FER
		//buscar cami -> dijkstra o com putes s'escrigui
	
		Position nearest_post = find_nearest_post(in.pos);
		Position next = aStarSearch(in.pos, nearest_post, id);
		command_soldier(id, next.i, next.j);
  	}
	
	
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
		scan();
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
