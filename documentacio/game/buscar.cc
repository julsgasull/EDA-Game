struct Pos
{
  int x;
  int y;
  
  Pos(int new_x, int new_y)
  {
    this->x = new_x;
    this->y = new_y;
  }
  Pos operator =(Pos new_pos)
  {
    this->x = new_pos.x;
    this->y = new_pos.y; 
  }
}

void dfs(Pos& Begin, Pos& End, bool W)
{
  int n = 60;
  list<Pos> L;
  queue<Pos> Q;
  vector< vector<bool> > enc (n, vectro<bool>(m, false));
  
  for(int i = 0; i < n; ++i)
  {
    for(int j = 0; j < n; ++i)
    {
      if(not enc[i][j] and (W or what(i, j) != WATER) and what(i, j) != MOUNTAIN)
      {
        while(not Q.empty())
        {
          Pos v = Q.front();
          Q.pop();
          L.push_back(v);
          for(int y = 0; y < n; ++y)
          {
            for(int x = 0; x < n; ++x)
            {
              if(not enc[x][y])
              {
                Q.push(Pos(x, y));
                enc[x][y] = true;
              }
            }
          }
        }
      }
    }
  }
}