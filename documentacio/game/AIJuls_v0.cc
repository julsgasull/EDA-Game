#include "Player.hh"

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */

#define PLAYER_NAME Juls_v0
#define MAX_INT 100000000


// DISCLAIMER: The following Demo player is *not* meant to do anything
// sensible. It is provided just to illustrate how to use the API.
// Please use AINull.cc as a template for your player.


vector< vector<char> > Board (60, vector<char> (60, '.'));


struct PLAYER_NAME : public Player
{

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

	/*-------------------------------------------------------*/
	/*------------------------ SCAN -------------------------*/
	/*-------------------------------------------------------*/
	void scan()
	{
		int mi = me();
		for (int i = 0; i < 60; i++)
		{
			for (int j = 0; j < 60; j++)
			{
				if (Board[i][j] != 'M' or Board[i][j] != 'W')
				{
					int p = post_owner(i, j);
					int id2 = which_soldier(i, j);
					
					if (what(i, j) == 4) Board[i][j] = 'M';							// mountain -> M
					else if (what(i, j) == 3) Board[i][j] = 'W';					// water -> W
					else if (fire_time(i, j) > 0) Board[i][j] = 'F';				// fire -> F
					else if (id2 > 0 and data(id2).player != mi) Board[i][j] = 'E';	// enemy -> E
					else if (p != -2 and p != mi) Board[i][j] = 'P';				// post -> P
					else Board[i][j] = '.';											// grass or forest -> .
				}
			}
		}
	}
	
	
	
	/*-------------------------------------------------------*/
	/*----------------------- SOLDIER -----------------------*/
	/*-------------------------------------------------------*/

	struct Node	// for a*
	{
		int parent_i, parent_j;
		int f, g, h;
	};
	
	bool isValid(int row, int col, int id)	// soldier can go there
	{
		Data id2 = data(id);
		for (int pl = 0; pl < NUM_PLAYERS; pl++)
		{
			auto soldiers_vector = soldiers(pl);
			for (auto& soldiers_it : soldiers_vector)
			{
				if (id != soldiers_it)
				{
					auto act_soldier = data(soldiers_it);
					
					if ((pl == me() and act_soldier.pos.i == row and act_soldier.pos.j == col) or
						(pl != me() and act_soldier.pos.i == row and act_soldier.pos.j == col and act_soldier.life > id2.life))
					{
						return false;
					}
				}
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
		// // cerr << "(" << src.i << " " << src.j << ") (" << dest.i << " " << dest.j << ")" << endl;
		Node n;
		n.parent_i = -1;
		n.parent_j = -1;
		n.f = MAX_INT;
		n.g = MAX_INT;
		n.h = MAX_INT;
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
			 
			 Cell --> Popped Cell 	(i   ,   j)
			 N    --> North       	(i-1 ,   j)
			 S    --> South       	(i+1 ,   j)
			 E    --> East        	(i   , j+1)
			 W    --> West         	(i   , j-1)
			 N.E  --> North-East  	(i-1 , j+1)
			 N.W  --> North-West  	(i-1 , j-1)
			 S.E  --> South-East  	(i+1 , j+1)
			 S.W  --> South-West  	(i+1 , j-1)
			 */
			
			
			// NORTH
			if (isValid(i-1, j, id))
			{
				// // cerr << "north" << endl;
				if (isDestination(i - 1, j, dest))			// my destination
				{
					cellDetails[i - 1][j].parent_i = i;
					cellDetails[i - 1][j].parent_j = j;
					foundDest = true;
					return tracePath(cellDetails, dest, src);
				}
				else if (not closedList[i-1][j])			// not my destination -> increm.
				{
					new_g = cellDetails[i][j].g + 1;
					new_h = calculateHValue (i-1, j, dest);
					new_f = new_g + new_h;
					
					if (cellDetails[i-1][j].f == MAX_INT or cellDetails[i-1][j].f > new_f)
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
				// // cerr << "south" << endl;
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
					
					if (cellDetails[i+1][j].f == MAX_INT or cellDetails[i+1][j].f > new_f)
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
				// // cerr << "east" << endl;
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
					
					if (cellDetails[i][j+1].f == MAX_INT or cellDetails[i][j+1].f > new_f)
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
				// // cerr << "west" << endl;
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
					
					if (cellDetails[i][j-1].f == MAX_INT or cellDetails[i][j-1].f > new_f)
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
				// // cerr << "north east" << endl;
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
					
					if (cellDetails[i-1][j+1].f == MAX_INT or cellDetails[i-1][j+1].f > new_f)
					{
						openList.insert( make_pair (new_f, make_pair(i-1, j+1)));
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
				// // cerr << "north west" << endl;
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
					
					if (cellDetails[i-1][j-1].f == MAX_INT or cellDetails[i-1][j-1].f > new_f)
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
				// // cerr << "south east" << endl;
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
					
					if (cellDetails[i+1][j+1].f == MAX_INT or cellDetails[i+1][j+1].f > new_f)
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
				// // cerr << "south west" << endl;
				if (isDestination(i+1, j-1, dest))
				{
					// // cerr << "he llegao" << endl;
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
					
					if (cellDetails[i+1][j-1].f == MAX_INT or cellDetails[i+1][j-1].f > new_f)
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
		// // cerr << "wtf" << endl;
		return Position(-1,-1);
	}
	
	
	Position find_nearest_post_or_enemy(const Position& src)
	{
		int min_distance = MAX_INT;
		Position min_distance_pos = src;
		
		for (int i = 0; i < 60; i++)
		{
			for (int j = 0; j < 60; j++)
			{
				if (Board[i][j] == 'E' or Board[i][j] == 'P')
				{
					int distance = calculateHValue(src.i, src.j, Position(i,j));
					if (distance < min_distance)
					{
						min_distance = distance;
						min_distance_pos = Position(i,j);
					}
				}
			}
		}
		return min_distance_pos;
	}
	
  	void play_soldier(int id)
	{
		Data in = data(id);

		for (int k = 0; k < 8; ++k)		// adj enemy or post -> go
		{
			int ii = in.pos.i + I[k];
		  	int jj = in.pos.j + J[k];
		  	if (pos_ok(ii,jj))
			{
				if (Board[ii][jj] == 'E' or Board[ii][jj] == 'P')
				{
					command_soldier(id, ii, jj);
					return;
				}
		  	}
    	}
		Position nearest_post = find_nearest_post_or_enemy(in.pos);
		Position next = aStarSearch(in.pos, nearest_post, id);
		command_soldier(id, next.i, next.j);
  	}
	
	
	
	/*--------------------------------------------------------*/
	/*---------------------- HELICOPTER ----------------------*/
	/*--------------------------------------------------------*/
	
//	struct Node_HEL
//	{
//		int parent_i, parent_j;
//		int parent_o;
//		int f, g, h;
//	};
//
//	bool isValid_HEL(int row, int col, int id)	//si helicopter pot passar
//	{
//		bool valid = true;
//		for (int i = row - 2 ; i <= row + 2; i++)
//		{
//			for (int j = col - 2; j <= col + 2; j++)
//			{
//				if (valid)
//				{
//					if (not pos_ok(i, j) or what(i, j) == 4) valid = false;
//				}
//			}
//		}
//		return valid;
//	}
//
//	int calculateHValue_HEL(int row, int col, Position dest)
//	{
//		return (abs(row - col) + abs(dest.i - dest.j));		//MANHATTAN DISTANCE
//	}
//
//	bool isDestination_HEL(const int i, const int j, const Position &destination)
//	{
//		return i == destination.i and j == destination.j;
//	}
//
//	Position tracePath_HEL(vector< vector<Node_HEL> > &cellDetails, const Position &dst, const Position &src)
//	{
//		int row = dst.i;
//		int col = dst.j;
//
//		// // cerr << "tracing path...";
//
//		stack<pair<int, int>> path;
//
//		while (not (cellDetails[row][col].parent_i == row and cellDetails[row][col].parent_j == col))
//		{
//			// // cerr << "in row: " << row << " col: " << col << endl;
//			path.push(make_pair(row, col));
//			int temp_row = cellDetails[row][col].parent_i;
//			int temp_col = cellDetails[row][col].parent_j;
//			row = temp_row;
//			col = temp_col;
//			// // cerr << "out row: " << row << " col: " << col << endl;
//		}
//
//		Position p;
//		p.i = path.top().first;
//		p.j = path.top().second;
//
//		// // cerr << "done";
//		return p;
//	}
//
//
//	// A* hel
//	pair<Position, int> aStarSearch_HEL(const Position &src, const Position &dest, int id)		//retorna un parell de: <posicio, oridentacio>
//	{
//		Data id2 = data(id);
//		int orient = id2.orientation;
//
//		if (isDestination_HEL(src.i, src.j, dest)) return make_pair(src, orient);
//		if (not isValid_HEL(src.i, src.j, id) or not isValid_HEL(dest.i, dest.j, id)) return make_pair(Position(-1,-1), -1);
//
//		vector< vector<int> > closedList (60, vector<int> (60, 0));	//matriu de mantisses -> explicat a sota
//		// cerr << "Source: (" << src.i << " " << src.j << ")      Dest: (" << dest.i << " " << dest.j << ")" << endl;
//		// cerr << "Orientation src: " << orient << endl;
//
//		Node_HEL n;
//		n.parent_o = -1;
//		n.parent_i = -1;
//		n.parent_j = -1;
//		n.f = MAX_INT;
//		n.g = MAX_INT;
//		n.h = MAX_INT;
//		vector< vector<Node_HEL> > cellDetails(60, vector<Node_HEL> (60, n));
//
//		cellDetails[src.i][src.j].f        = 0;
//		cellDetails[src.i][src.j].g        = 0;
//		cellDetails[src.i][src.j].h        = 0;
//		cellDetails[src.i][src.j].parent_i = src.i;
//		cellDetails[src.i][src.j].parent_j = src.j;
//		cellDetails[src.i][src.j].parent_o = orient;
//
//		set< pair<int, pair< pair<int, int>, int> > > openList;		// f , pair (position, orientation)
//		openList.insert(make_pair(0, make_pair(make_pair(src.i, src.j), orient)));
//		bool foundDest = false;
//
//		while (not openList.empty())
//		{
//			pair<int, pair< pair<int, int>, int> > p = *(openList.begin());
//			openList.erase(openList.begin());
//			int i = p.second.first.first;
//			int j = p.second.first.second;
//
//			/*			orientation		 mantisa
//			 	south 		= 0			0001 =  1
//			 	east  		= 1			0010 =  2
//			 	north 		= 2			0100 =  4
//			 	west  		= 3			1000 =  8
//			*/
//
//			int o = cellDetails[i][j].parent_o;
//			if      (o == 0) closedList[i][j] |= 1; 	// south
//			else if (o == 1) closedList[i][j] |= 2;		// east
//			else if (o == 2) closedList[i][j] |= 4;		// north
//			else if (o == 3) closedList[i][j] |= 8;		// west
//
//			// cerr << "----------------------" << closedList[i][j] << endl;
//
//			int new_f;
//			int new_g;
//			int new_h;
//
//			/*
//			 Generating all the 8 successor of this cell
//
//			 N.W    N     N.E
//			  \     |     /
//			   \    |    /
//			 W---- Cell ----E
//			   /    |    \
//			  /     |     \
//			 S.W    S     S.E
//
//			 Cell-->Popped Cell 	(i, j)
//			 N -->  North       	(i-1, j)
//			 S -->  South       	(i+1, j)
//			 E -->  East        	(i, j+1)
//			 W -->  West         	(i, j-1)
//			 N.E--> North-East  	(i-1, j+1)
//			 N.W--> North-West  	(i-1, j-1)
//			 S.E--> South-East  	(i+1, j+1)
//			 S.W--> South-West  	(i+1, j-1)
//			 */
//
//			int new_i = -1; int new_j = -1;
//			int new_o = o;
//			int andpersand = 0;
//
//			// FORWARD 1
//			if      (o == 0) { new_i = i + 1; new_j = j; andpersand = 1;}			// SOUTH
//			else if (o == 1) { new_i = i; new_j = j + 1; andpersand = 2;}			// EAST
//			else if (o == 2) { new_i = i - 1; new_j = j; andpersand = 4;} 			// NORTH
//			else if (o == 3) { new_i = i; new_j = j - 1; andpersand = 8;} 			// WEST
//
//
//			if (isValid_HEL(new_i, new_j, id))
//			{
//				if (isDestination(new_i, new_j, dest))
//				{	// Es mi destino
//					cellDetails[new_i][new_j].parent_i = i;
//					cellDetails[new_i][new_j].parent_j = j;
//					cellDetails[new_i][new_j].parent_o = o;
//					foundDest = true;
//					// cerr << "Destino forward1" << endl;
//					return make_pair(tracePath_HEL(cellDetails, dest, src), new_o);
//				}
//				else if ((closedList[new_i][new_j]&andpersand) == 0) // orientacio no visitada
//				{	// No es mi destino y hay que iterarlo
//					new_g = cellDetails[i][j].g + 1;
//					new_h = calculateHValue_HEL(new_i, new_j, dest);
//					new_f = new_g + new_h;
//					// cerr << "No visitado forward1" << endl;
//					if (cellDetails[new_i][new_j].f == MAX_INT or cellDetails[new_i][new_j].f >= new_f)
//					{
//						// cerr << "Inserto forward1" << endl;
//						openList.insert(make_pair(new_f, make_pair(make_pair(new_i, new_j), new_o)));
//						cellDetails[new_i][new_j].f        = new_f;
//						cellDetails[new_i][new_j].g        = new_g;
//						cellDetails[new_i][new_j].h        = new_h;
//						cellDetails[new_i][new_j].parent_i = i;
//						cellDetails[new_i][new_j].parent_j = j;
//						cellDetails[new_i][new_j].parent_o = o;
//					}
//				}
//			}
//
//			// FORWARD 2
//			if      (o == 0) { new_i = i + 2; new_j = j; andpersand = 1;}			// SOUTH
//			else if (o == 1) { new_i = i; new_j = j + 2; andpersand = 2;}			// EAST
//			else if (o == 2) { new_i = i - 2; new_j = j; andpersand = 4;} 			// NORTH
//			else if (o == 3) { new_i = i; new_j = j - 2; andpersand = 8;} 			// WEST
//
//			if (isValid_HEL(new_i, new_j, id))
//			{
//				if (isDestination(new_i, new_j, dest))
//				{	// Es mi destino
//					cellDetails[new_i][new_j].parent_i = i;
//					cellDetails[new_i][new_j].parent_j = j;
//					cellDetails[new_i][new_j].parent_o = o;
//					foundDest = true;
//					// cerr << "Destino forward2" << endl;
//					return make_pair(tracePath_HEL(cellDetails, dest, src), new_o);
//				}
//				else if ((closedList[new_i][new_j]&andpersand) == 0) // orientacio no visitada
//				{	// No es mi destino y hay que iterarlo
//					new_g = cellDetails[i][j].g + 1;
//					new_h = calculateHValue_HEL(new_i, new_j, dest);
//					new_f = new_g + new_h;
//					// cerr << "No visitado forward2" << endl;
//					if (cellDetails[new_i][new_j].f == MAX_INT or cellDetails[new_i][new_j].f >= new_f)
//					{
//						// cerr << "Inserto forward2" << endl;
//						openList.insert(make_pair(new_f, make_pair(make_pair(new_i, new_j), new_o)));
//						cellDetails[new_i][new_j].f        = new_f;
//						cellDetails[new_i][new_j].g        = new_g;
//						cellDetails[new_i][new_j].h        = new_h;
//						cellDetails[new_i][new_j].parent_i = i;
//						cellDetails[new_i][new_j].parent_j = j;
//						cellDetails[new_i][new_j].parent_o = o;
//					}
//				}
//			}
//
//
//
//			// CLOCKWISE
//			if      (o == 0) {new_o = 3; andpersand = 8;}			// SOUTH -> west
//			else if (o == 1) {new_o = 0; andpersand = 1;}			// EAST -> south
//			else if (o == 2) {new_o = 1; andpersand = 2;}			// NORTH -> east
//			else if (o == 3) {new_o = 2; andpersand = 4;} 			// WEST -> north
//			new_i = i; new_j = j; 			//per girar al mateix lloc però orientacio diferent
//
//			if (isValid_HEL(new_i, new_j, id))
//			{
//				if (isDestination(new_i, new_j, dest))
//				{
//					// cerr << "Destino clock" << endl;
//					cellDetails[new_i][new_j].parent_i = i;
//					cellDetails[new_i][new_j].parent_j = j;
//					cellDetails[new_i][new_j].parent_o = o;
//					foundDest = true;
//					return make_pair(tracePath_HEL(cellDetails, dest, src), new_o);
//				}
//				else if ((closedList[new_i][new_j]&andpersand) == 0) // orientacio no visitada
//				{	// No es mi destino y hay que iterarlo
//					new_g = cellDetails[i][j].g + 1;
//					new_h = calculateHValue_HEL(new_i, new_j, dest);
//					new_f = new_g + new_h;
//					// cerr << "No visitado clock" << endl;
//					if (cellDetails[new_i][new_j].f == MAX_INT or cellDetails[new_i][new_j].f >= new_f)
//					{
//						// cerr << "Inserto clock" << endl;
//						openList.insert(make_pair(new_f, make_pair(make_pair(new_i, new_j), new_o)));
//						cellDetails[new_i][new_j].f        = new_f;
//						cellDetails[new_i][new_j].g        = new_g;
//						cellDetails[new_i][new_j].h        = new_h;
//						cellDetails[new_i][new_j].parent_i = i;
//						cellDetails[new_i][new_j].parent_j = j;
//						cellDetails[new_i][new_j].parent_o = o;
//					}
//				}
//			}
//
//			// COUNTER CLOCKWISE
//			if      (orient == 0) {new_o = 1; andpersand = 2;}		// SOUTH -> east
//			else if (orient == 1) {new_o = 2; andpersand = 4;}		// EAST -> north
//			else if (orient == 2) {new_o = 3; andpersand = 8;}		// NORTH -> west
//			else if (orient == 3) {new_o = 0; andpersand = 1;}		// WEST -> south
//			new_i = i; new_j = j; 			//per girar al mateix lloc però orientacio diferent
//
//			if (isValid_HEL(new_i, new_j, id))
//			{
//				if (isDestination(new_i, new_j, dest))
//				{	// Es mi destino
//					// cerr << "Destino anti clock" << endl;
//					cellDetails[new_i][new_j].parent_i = i;
//					cellDetails[new_i][new_j].parent_j = j;
//					cellDetails[new_i][new_j].parent_o = o;
//					foundDest = true;
//					return make_pair(tracePath_HEL(cellDetails, dest, src), new_o);
//				}
//				else if ((closedList[new_i][new_j]&andpersand) == 0) // orientacio no visitada
//				{	// No es mi destino y hay que iterarlo
//					new_g = cellDetails[i][j].g + 1;
//					new_h = calculateHValue_HEL(new_i, new_j, dest);
//					new_f = new_g + new_h;
//					// cerr << "No visitado anti clock" << endl;
//					if (cellDetails[new_i][new_j].f == MAX_INT or cellDetails[new_i][new_j].f >= new_f)
//					{
//						// cerr << "Inserto anti clock" << endl;
//						openList.insert(make_pair(new_f, make_pair(make_pair(new_i, new_j), new_o)));
//						cellDetails[new_i][new_j].f        = new_f;
//						cellDetails[new_i][new_j].g        = new_g;
//						cellDetails[new_i][new_j].h        = new_h;
//						cellDetails[new_i][new_j].parent_i = i;
//						cellDetails[new_i][new_j].parent_j = j;
//						cellDetails[new_i][new_j].parent_o = o; //sobreescriu?
//					}
//				}
//			}
//		}
//		// cerr << "wtf" << endl;
//		return make_pair(Position(-2,-2), -2);
//	}
//
//
//	Position find_nearest_post_HEL(const Position& src)	// Returns neerest post
//	{
//		auto postes = posts();
//		int min_distance = MAX_INT;
//		Position min_distance_pos = src;
//		for (auto& post : postes)
//		{
//			int distance = calculateHValue(src.i, src.j, post.pos);
//			if (distance < min_distance and post_owner(post.pos.i, post.pos.j) != -2 and post_owner(post.pos.i, post.pos.j) != me())
//			{
//				min_distance = distance;
//				min_distance_pos = post.pos;
//			}
//		}
//		return min_distance_pos;
//	}

	int quadrant(int i, int j)	// returns whitch quadrant is it
	{
		if (( 0 <= i and i <= 29) and ( 0 <= j and j <= 29)) return 0;
		if ((30 <= i and i <= 59) and ( 0 <= j and j <= 29)) return 1;
		if (( 0 <= i and i <= 29) and (30 <= j and j <= 59)) return 2;
		if ((30 <= i and i <= 59) and (30 <= j and j <= 59)) return 3;
		return -1;
	}
	
	

	void play_helicopter(int id)
	{
		Data in = data(id);
		
		if (in.napalm == 0) // napalm available
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
						if (s > 0) // there's a soldier
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
		
		int ii = in.pos.i;
		int jj = in.pos.j;
		int oo = in.orientation;
		
		// TO-DO: ir hacia post que no es tuyo -> napalm + conquistar
		if ((round() == 0) or (oo == 0 and ii == 56) or (oo == 1 and jj == 56) or (oo == 2 and ii == 3) or (oo == 3 and jj == 3))  command_helicopter(id, CLOCKWISE);
		else command_helicopter(id, FORWARD2);
		
//		if (pos_ok(ii, jj))
//		{
//			Position nearest_post = find_nearest_post_HEL(in.pos);
//			pair<Position, int> nextp = aStarSearch_HEL(in.pos, nearest_post, id);
//			//Position next = nextp.first;
//			int next_o = nextp.second;
//
//			// cerr << "ACTUAL: (" << ii << ", " << jj << ") ORIENT: "  << oo << endl;
//			// cerr << "NEXT: (" << next.i << ", " << next.j << ") ORIENT: "  << next_o << endl;
//
//			if (oo != next_o)
//			{
//				switch (oo)
//				{
//					case 0:
//						if (next_o == 1) command_helicopter(id, COUNTER_CLOCKWISE);
//						else command_helicopter(id, CLOCKWISE);
//						break;
//					case 1:
//						if (next_o == 2) command_helicopter(id, COUNTER_CLOCKWISE);
//						else command_helicopter(id, CLOCKWISE);
//						break;
//					case 2:
//						if (next_o == 3) command_helicopter(id, COUNTER_CLOCKWISE);
//						else command_helicopter(id, CLOCKWISE);
//						break;
//					default:		//3
//						if (next_o == 0) command_helicopter(id, COUNTER_CLOCKWISE);
//						else command_helicopter(id, CLOCKWISE);
//						break;
//				}
//				// cerr << "GIRO" << endl;
//				return;
//			}
//			else // if (ii != next.i or jj != next.j)
//			{
//				command_helicopter(id, FORWARD2);
//				// cerr << "AVANÇO" << endl;
//				return;
//			}
//		}
//
//		if (oo == 0 and isValid_HEL(ii-2, jj, id)) command_helicopter(id, FORWARD2);
//		else if (oo == 1 and isValid_HEL(ii, jj+2, id)) command_helicopter(id, FORWARD2);
//		else if (oo == 2 and isValid_HEL(ii+2, jj, id)) command_helicopter(id, FORWARD2);
//		else if (oo == 3 and isValid_HEL(ii, jj-2, id)) command_helicopter(id, FORWARD2);
//		else command_helicopter(id, CLOCKWISE);
//		return;
  	}
	
	/*--------------------------------------------------------*/
	/*---------------------- PARACHUTER ----------------------*/
	/*--------------------------------------------------------*/
	
	void throw_parachuter(int helicopter_id)
	{
		if (round() % 2 == 0) 	//every 2 rounds ("random")
		{
			Data in = data(helicopter_id);
			
			for (int ii = in.pos.i - 2; ii <= in.pos.i + 2; ii++) {
				for (int jj = in.pos.j - 2; jj <= in.pos.j + 2; jj++)
				{
					if (fire_time(ii, jj) == 0 and what(ii, jj) <= 2) //no fire && valid
					{
						int s = which_soldier(ii, jj);
						if (s == 0 or s == -1)	// no soldiers below
						{
							command_parachuter(ii, jj);
							return;
						}
					}
				}
			}
		}
  	}
  

	/*--------------------------------------------------------*/
	/*------------------------- PLAY -------------------------*/
	/*--------------------------------------------------------*/
  	virtual void play ()
	{

    	int player = me();
		scan();
    	vector<int> H = helicopters(player); 	// helicopters of my player
    	vector<int> S = soldiers(player); 		// soldiers of my player
	
		// PARACHUTERS
		int helicopter_id = H[random(0, int(H.size()-1))];	// If in a random helicopter I have parachuters, I throw one.
		if (not data(helicopter_id).parachuters.empty()) throw_parachuter(helicopter_id);
		
		// HELICOPTERS
		for (int i = 0; i < int(H.size()); ++i) play_helicopter(H[i]);
		
		// SOLDIERS
		for (int i = 0; i < int(S.size()); ++i) play_soldier(S[i]);
  	}
	
};

constexpr int PLAYER_NAME::I[8];
constexpr int PLAYER_NAME::J[8];

/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
