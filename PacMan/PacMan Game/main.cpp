#include <array>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace sf;

// sized and speed and map height timing and declared here 
constexpr unsigned char CELL_SIZE = 16;
constexpr unsigned char FONT_HEIGHT = 16;
constexpr unsigned char GHOST_1_CHASE = 2;
constexpr unsigned char GHOST_2_CHASE = 1;
constexpr unsigned char GHOST_3_CHASE = 4;
constexpr unsigned char GHOST_ANIMATION_FRAMES = 6;
constexpr unsigned char GHOST_ANIMATION_SPEED = 4;
constexpr unsigned char GHOST_ESCAPE_SPEED = 4;
constexpr unsigned char GHOST_FRIGHTENED_SPEED = 3;
constexpr unsigned char GHOST_SPEED = 1;
constexpr unsigned char MAP_HEIGHT = 21;
constexpr unsigned char MAP_WIDTH = 21;
constexpr unsigned char PACMAN_ANIMATION_FRAMES = 6;
constexpr unsigned char PACMAN_ANIMATION_SPEED = 4;
constexpr unsigned char PACMAN_DEATH_FRAMES = 12;
constexpr unsigned char PACMAN_SPEED = 2;
constexpr unsigned char SCREEN_RESIZE = 2;

constexpr unsigned short CHASE_DURATION = 1024;
constexpr unsigned short ENERGIZER_DURATION = 512;
constexpr unsigned short FRAME_DURATION = 16667;
constexpr unsigned short GHOST_FLASH_START = 64;
constexpr unsigned short LONG_SCATTER_DURATION = 512;
constexpr unsigned short SHORT_SCATTER_DURATION = 256;
// for the map 
enum Cell
{
   Door,
   Empty,
   Energizer,
   Pellet,
   Wall
};

// position of pacman
struct Position
{
   short x;
   short y;

   bool operator==(const Position& other)
   {
      return x == other.x && y == other.y;
   }
};


class Pacman;
class Ghost;
class GhostManager;

bool map_collision(bool collect_pellets, bool use_door, short x, short y, array<array<Cell, MAP_HEIGHT>, MAP_WIDTH>& map);

// queue implementation for bfs path finding
template<typename T, size_t SIZE>
class SimpleQueue {
    T data[SIZE];
    size_t front, back;
public:
    SimpleQueue() : front(0), back(0) {}
    void push(const T& value) {
        data[back++] = value;
        if(back == SIZE) back = 0;
    }
    void pop() {
        ++front;
        if(front == SIZE) front = 0;
    }
    T& top() { return data[front]; }
    bool empty() const { return front == back; }
};

// Stack implementation for pause/resume functionality
template<typename T, size_t SIZE>
class SimpleStack {
    T data[SIZE];
    size_t top_index;
public:
    SimpleStack() : top_index(0) {}
    void push(const T& value) {
        if (top_index < SIZE) {
            data[top_index++] = value;
        }
    }
    void pop() {
        if (top_index > 0) {
            top_index--;
        }
    }
    T& top() { return data[top_index - 1]; }
    bool empty() const { return top_index == 0; }
    size_t size() const { return top_index; }
};

// using the bfs for pathfinding for ghost and also impemented the queue class for bfs
 // (0:right,1:up,2:left,3:down)
 

int bfs_next_direction(
    const array<array<Cell, MAP_HEIGHT>, MAP_WIDTH>& map,
    Position start,
    Position goal
) {
    // Convert pixel positions to grid cells
    int start_x = start.x / CELL_SIZE;
    int start_y = start.y / CELL_SIZE;
    int goal_x  = goal.x / CELL_SIZE;
    int goal_y  = goal.y / CELL_SIZE;

    static const int dx[4] = {1, 0, -1, 0};
    static const int dy[4] = {0,-1,  0, 1};
    bool visited[MAP_WIDTH][MAP_HEIGHT] = {};
    int  parent_dir[MAP_WIDTH][MAP_HEIGHT] = {};

    SimpleQueue<pair<int,int>, MAP_WIDTH * MAP_HEIGHT> q;
    q.push({start_x, start_y});
    visited[start_x][start_y] = true;

    while (!q.empty()) {
        pair<int, int> current = q.top();
        q.pop();
        int x = current.first;
        int y = current.second;
        if (x == goal_x && y == goal_y) break;
        for (int dir = 0; dir < 4; ++dir) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) continue;
            if (map[nx][ny] == Wall) continue;
            if (visited[nx][ny]) continue;
            visited[nx][ny] = true;
            parent_dir[nx][ny] = (dir + 2) % 4;
            q.push({nx, ny});
        }
    }
    int x = goal_x, y = goal_y;
    if (!visited[x][y]) return -1; // this return -1 for No path
    vector<int> path_dirs;
    while (!(x == start_x && y == start_y)) {
        int dir = parent_dir[x][y];
        path_dirs.push_back((dir + 2) % 4);
        x += dx[dir];
        y += dy[dir];
    }
    return path_dirs.empty() ? -1 : path_dirs.back();
}
//pacman class for direction and position locat
class Pacman
{
   bool animation_over;
   bool dead;
   unsigned char direction;
   unsigned short animation_timer;
   unsigned short energizer_timer;
   Position position;

public:
   Pacman();

   bool get_animation_over();
   bool get_dead();
   unsigned char get_direction();
   unsigned short get_energizer_timer();

   void draw(bool victory, RenderWindow& window);
   void reset();
   void set_animation_timer(unsigned short value);
   void set_dead(bool value);
   void set_position(short x, short y);
   void update(unsigned char level, array<array<Cell, MAP_HEIGHT>, MAP_WIDTH>& map, int& score, sf::Sound& chompSound);

   Position get_position();
};

//ghost class used for here with position and target and direction providing using bfs
class Ghost
{
   bool movement_mode;
   bool use_door;
   unsigned char direction;
   unsigned char frightened_mode;
   unsigned char frightened_speed_timer;
   unsigned char id;
   unsigned short animation_timer;
   Position home;
   Position home_exit;
   Position position;
   Position target;

public:
   explicit Ghost(unsigned char ghost_id);

   bool pacman_collision(const Position& pacman_position);
   float get_target_distance(unsigned char direction_override);

   void draw(bool flash, RenderWindow& window);
   void reset(const Position& in_home, const Position& in_home_exit);
   void set_position(short x, short y);
   void switch_mode();
   void update(unsigned char level, array<array<Cell, MAP_HEIGHT>, MAP_WIDTH>& map, Ghost& ghost0, Pacman& pacman);
   void update_target(unsigned char pacman_direction, const Position& ghost0_position, const Position& pacman_position);

   Position get_position();
};

// managing the ghost mananger 4 ghost ko create kre ga
class GhostManager
{
   unsigned char current_wave;
   unsigned short wave_timer;
   array<Ghost, 4> ghosts;

public:
   GhostManager();

   void draw(bool flash, RenderWindow& window);
   void reset(unsigned char level, const array<Position, 4>& ghost_positions);
   void update(unsigned char level, array<array<Cell, MAP_HEIGHT>, MAP_WIDTH>& map, Pacman& pacman);
};

array<array<Cell, MAP_HEIGHT>, MAP_WIDTH> convert_sketch(const array<string, MAP_HEIGHT>& map_sketch, array<Position, 4>& ghost_positions, Pacman& pacman);
void draw_map(const array<array<Cell, MAP_HEIGHT>, MAP_WIDTH>& map, RenderWindow& window);
void draw_text(bool center, unsigned short x, unsigned short y, const string& text, RenderWindow& window);
void draw_lives_hearts(unsigned char lives, RenderWindow& window);


// Pacman implementation there 
Pacman::Pacman() :
   animation_over(0),
   dead(0),
   direction(0),
   animation_timer(0),
   energizer_timer(0),
   position({0, 0})
{
}

bool Pacman::get_animation_over()
{
   return animation_over;
}

bool Pacman::get_dead()
{
   return dead;
}

unsigned char Pacman::get_direction()
{
   return direction;
}

unsigned short Pacman::get_energizer_timer()
{
   return energizer_timer;
}

void Pacman::draw(bool victory, RenderWindow& window)
{
   unsigned char frame = static_cast<unsigned char>(floor(animation_timer / static_cast<float>(PACMAN_ANIMATION_SPEED)));

Sprite sprite;
Texture texture;

   sprite.setPosition(position.x, position.y);

   if (dead || victory)
   {
      if (animation_timer < PACMAN_DEATH_FRAMES * PACMAN_ANIMATION_SPEED)
      {
         animation_timer++;

         texture.loadFromFile("Resources/Images/PacmanDeath" + to_string(CELL_SIZE) + ".png");

         sprite.setTexture(texture);
         sprite.setTextureRect(IntRect(CELL_SIZE * frame, 0, CELL_SIZE, CELL_SIZE));

         window.draw(sprite);
      }
      else
      {
         animation_over = 1;
      }
   }
   else
   {
      texture.loadFromFile("Resources/Images/Pacman" + to_string(CELL_SIZE) + ".png");

      sprite.setTexture(texture);
      sprite.setTextureRect(IntRect(CELL_SIZE * frame, CELL_SIZE * direction, CELL_SIZE, CELL_SIZE));

      window.draw(sprite);

      animation_timer = (1 + animation_timer) % (PACMAN_ANIMATION_FRAMES * PACMAN_ANIMATION_SPEED);
   }
}

void Pacman::reset()
{
   animation_over = 0;
   dead = 0;
   direction = 0;
   animation_timer = 0;
   energizer_timer = 0;
}

void Pacman::set_animation_timer(unsigned short value)
{
   animation_timer = value;
}

void Pacman::set_dead(bool value)
{
   dead = value;

   if (dead)
   {
      animation_timer = 0;
   }
}

void Pacman::set_position(short x, short y)
{
   position = {x, y};
}

void Pacman::update(unsigned char level, array<array<Cell, MAP_HEIGHT>, MAP_WIDTH>& map, int& score, sf::Sound& chompSound)
{
   array<bool, 4> walls{};
   walls[0] = map_collision(0, 0, PACMAN_SPEED + position.x, position.y, map);
   walls[1] = map_collision(0, 0, position.x, position.y - PACMAN_SPEED, map);
   walls[2] = map_collision(0, 0, position.x - PACMAN_SPEED, position.y, map);
   walls[3] = map_collision(0, 0, position.x, PACMAN_SPEED + position.y, map);

   if (Keyboard::isKeyPressed(Keyboard::Right) && !walls[0])
   {
      direction = 0;
   }
   if (Keyboard::isKeyPressed(Keyboard::Up) && !walls[1])
   {
      direction = 1;
   }
   if (Keyboard::isKeyPressed(Keyboard::Left) && !walls[2])
   {
      direction = 2;
   }
   if (Keyboard::isKeyPressed(Keyboard::Down) && !walls[3])
   {
      direction = 3;
   }

   if (!walls[direction])
   {
      switch (direction)
      {
         case 0: position.x += PACMAN_SPEED; break;
         case 1: position.y -= PACMAN_SPEED; break;
         case 2: position.x -= PACMAN_SPEED; break;
         case 3: position.y += PACMAN_SPEED; break;
      }
   }

   if (position.x <= -CELL_SIZE)
   {
      position.x = CELL_SIZE * MAP_WIDTH - PACMAN_SPEED;
   }
   else if (position.x >= CELL_SIZE * MAP_WIDTH)
   {
      position.x = PACMAN_SPEED - CELL_SIZE;
   }

   
   bool collected = false;
   float cell_x = position.x / static_cast<float>(CELL_SIZE);
   float cell_y = position.y / static_cast<float>(CELL_SIZE);
   for (unsigned char a = 0; a < 4; a++)
   {
      short cx = 0;
      short cy = 0;
      switch (a)
      {
         case 0: cx = static_cast<short>(floor(cell_x)); cy = static_cast<short>(floor(cell_y)); break;
         case 1: cx = static_cast<short>(ceil(cell_x)); cy = static_cast<short>(floor(cell_y)); break;
         case 2: cx = static_cast<short>(floor(cell_x)); cy = static_cast<short>(ceil(cell_y)); break;
         case 3: cx = static_cast<short>(ceil(cell_x)); cy = static_cast<short>(ceil(cell_y)); break;
      }
      if (0 <= cx && 0 <= cy && MAP_HEIGHT > cy && MAP_WIDTH > cx)
      {
         if (map[cx][cy] == Cell::Energizer)
         {
            map[cx][cy] = Cell::Empty;
            score += 50;
            chompSound.play();
         }
         else if (map[cx][cy] == Cell::Pellet)
         {
            map[cx][cy] = Cell::Empty;
            score += 10;
            chompSound.play();
         }
      }
   }
}

Position Pacman::get_position()
{
   return position;
}

// ghost implementation here
Ghost::Ghost(unsigned char i_id) :
   movement_mode(0),
   use_door(0),
   direction(0),
   frightened_mode(0),
   frightened_speed_timer(0),
   id(i_id),
   animation_timer(0),
   home({0, 0}),
   home_exit({0, 0}),
   position({0, 0}),
   target({0, 0})
{
}

bool Ghost::pacman_collision(const Position& pacman_position)
{
   if (position.x > pacman_position.x - CELL_SIZE && position.x < CELL_SIZE + pacman_position.x)
   {
      if (position.y > pacman_position.y - CELL_SIZE && position.y < CELL_SIZE + pacman_position.y)
      {
         return 1;
      }
   }

   return 0;
}

float Ghost::get_target_distance(unsigned char direction_override)
{
   short x = position.x;
   short y = position.y;

   switch (direction_override)
   {
      case 0: x += GHOST_SPEED; break;
      case 1: y -= GHOST_SPEED; break;
      case 2: x -= GHOST_SPEED; break;
      case 3: y += GHOST_SPEED; break;
      default: break;
   }

   return static_cast<float>(sqrt(pow(x - target.x, 2) + pow(y - target.y, 2)));
}

void Ghost::draw(bool flash, RenderWindow& window)
{
   unsigned char body_frame = static_cast<unsigned char>(floor(animation_timer / static_cast<float>(GHOST_ANIMATION_SPEED)));

Sprite body;
Sprite face;

Texture texture;
   texture.loadFromFile("Resources/Images/Ghost" + to_string(CELL_SIZE) + ".png");

   body.setTexture(texture);
   body.setPosition(position.x, position.y);
   body.setTextureRect(IntRect(CELL_SIZE * body_frame, 0, CELL_SIZE, CELL_SIZE));

   face.setTexture(texture);
   face.setPosition(position.x, position.y);

   if (0 == frightened_mode)
   {
      switch (id)
      {
         case 0: body.setColor(Color(255, 0, 0)); break;
         case 1: body.setColor(Color(255, 182, 255)); break;
         case 2: body.setColor(Color(0, 255, 255)); break;
         case 3: body.setColor(Color(255, 182, 85)); break;
      }

      face.setTextureRect(IntRect(CELL_SIZE * direction, CELL_SIZE, CELL_SIZE, CELL_SIZE));

      window.draw(body);
   }
   else if (1 == frightened_mode)
   {
      body.setColor(Color(36, 36, 255));
      face.setTextureRect(IntRect(4 * CELL_SIZE, CELL_SIZE, CELL_SIZE, CELL_SIZE));

      if (flash && 0 == body_frame % 2)
      {
         body.setColor(Color(255, 255, 255));
         face.setColor(Color(255, 0, 0));
      }
      else
      {
         body.setColor(Color(36, 36, 255));
         face.setColor(Color(255, 255, 255));
      }

      window.draw(body);
   }
   else
   {
      face.setTextureRect(IntRect(CELL_SIZE * direction, 2 * CELL_SIZE, CELL_SIZE, CELL_SIZE));
   }

   window.draw(face);

   animation_timer = (1 + animation_timer) % (GHOST_ANIMATION_FRAMES * GHOST_ANIMATION_SPEED);
}

void Ghost::reset(const Position& in_home, const Position& in_home_exit)
{
   movement_mode = 0;
   use_door = 0 < id;
   direction = 0;
   frightened_mode = 0;
   frightened_speed_timer = 0;
   animation_timer = 0;
   home = in_home;
   home_exit = in_home_exit;
   target = in_home_exit;
}

void Ghost::set_position(short x, short y)
{
   position = {x, y};
}

void Ghost::switch_mode()
{
   movement_mode = 1 - movement_mode;
}

void Ghost::update(unsigned char level, array<array<Cell, MAP_HEIGHT>, MAP_WIDTH>& map, Ghost& ghost0, Pacman& pacman)
{
   bool move = 0;
   unsigned char available_ways = 0;
   unsigned char speed = GHOST_SPEED;

   array<bool, 4> walls{};

   if (0 == frightened_mode && pacman.get_energizer_timer() == ENERGIZER_DURATION / pow(2, level))
   {
      frightened_speed_timer = GHOST_FRIGHTENED_SPEED;
      frightened_mode = 1;
   }
   else if (0 == pacman.get_energizer_timer() && 1 == frightened_mode)
   {
      frightened_mode = 0;
   }

   if (2 == frightened_mode && 0 == position.x % GHOST_ESCAPE_SPEED && 0 == position.y % GHOST_ESCAPE_SPEED)
   {
      speed = GHOST_ESCAPE_SPEED;
   }

   update_target(pacman.get_direction(), ghost0.get_position(), pacman.get_position());

   walls[0] = map_collision(0, use_door, speed + position.x, position.y, map);
   walls[1] = map_collision(0, use_door, position.x, position.y - speed, map);
   walls[2] = map_collision(0, use_door, position.x - speed, position.y, map);
   walls[3] = map_collision(0, use_door, position.x, speed + position.y, map);

   if (1 != frightened_mode)
   {
      unsigned char optimal_direction = 4;
      move = 1;

      for (unsigned char a = 0; a < 4; a++)
      {
         if (a == (2 + direction) % 4)
         {
            continue;
         }
         else if (!walls[a])
         {
            if (4 == optimal_direction)
            {
               optimal_direction = a;
            }

            available_ways++;

            if (get_target_distance(a) < get_target_distance(optimal_direction))
            {
               optimal_direction = a;
            }
         }
      }

      if (available_ways > 1)
      {
         direction = optimal_direction;
      }
      else
      {
         if (4 == optimal_direction)
         {
            direction = static_cast<unsigned char>((2 + direction) % 4);
         }
         else
         {
            direction = optimal_direction;
         }
      }
   }
   else
   {
      unsigned char random_direction = static_cast<unsigned char>(rand() % 4);

      if (0 == frightened_speed_timer)
      {
         move = 1;
         frightened_speed_timer = GHOST_FRIGHTENED_SPEED;

         for (unsigned char a = 0; a < 4; a++)
         {
            if (a == (2 + direction) % 4)
            {
               continue;
            }
            else if (!walls[a])
            {
               available_ways++;
            }
         }

         if (available_ways > 0)
         {
            while (walls[random_direction] || random_direction == (2 + direction) % 4)
            {
               random_direction = static_cast<unsigned char>(rand() % 4);
            }

            direction = random_direction;
         }
         else
         {
            direction = static_cast<unsigned char>((2 + direction) % 4);
         }
      }
      else
      {
         frightened_speed_timer--;
      }
   }

   if (move)
   {
      switch (direction)
      {
         case 0: position.x += speed; break;
         case 1: position.y -= speed; break;
         case 2: position.x -= speed; break;
         case 3: position.y += speed; break;
      }

      if (position.x <= -CELL_SIZE)
      {
         position.x = CELL_SIZE * MAP_WIDTH - speed;
      }
      else if (position.x >= CELL_SIZE * MAP_WIDTH)
      {
         position.x = speed - CELL_SIZE;
      }
   }

   if (pacman_collision(pacman.get_position()))
   {
      if (0 == frightened_mode)
      {
         pacman.set_dead(1);
      }
      else
      {
         use_door = 1;
         frightened_mode = 2;
         target = home;
      }
   }
}

void Ghost::update_target(unsigned char pacman_direction, const Position& ghost0_position, const Position& pacman_position)
{
   if (use_door)
   {
      if (position == target)
      {
         if (home_exit == target)
         {
            use_door = 0;
         }
         else if (home == target)
         {
            frightened_mode = 0;
            target = home_exit;
         }
      }
   }
   else
   {
      if (0 == movement_mode)
      {
         switch (id)
         {
            case 0: target = {CELL_SIZE * (MAP_WIDTH - 1), 0}; break;
            case 1: target = {0, 0}; break;
            case 2: target = {CELL_SIZE * (MAP_WIDTH - 1), CELL_SIZE * (MAP_HEIGHT - 1)}; break;
            case 3: target = {0, CELL_SIZE * (MAP_HEIGHT - 1)}; break;
         }
      }
      else
      {
         switch (id)
         {
            case 0:
            {
               target = pacman_position;
               break;
            }
            case 1:
            {
               target = pacman_position;

               switch (pacman_direction)
               {
                  case 0: target.x += CELL_SIZE * GHOST_1_CHASE; break;
                  case 1: target.y -= CELL_SIZE * GHOST_1_CHASE; break;
                  case 2: target.x -= CELL_SIZE * GHOST_1_CHASE; break;
                  case 3: target.y += CELL_SIZE * GHOST_1_CHASE; break;
                  default: break;
               }

               break;
            }
            case 2:
            {
               target = pacman_position;

               switch (pacman_direction)
               {
                  case 0: target.x += CELL_SIZE * GHOST_2_CHASE; break;
                  case 1: target.y -= CELL_SIZE * GHOST_2_CHASE; break;
                  case 2: target.x -= CELL_SIZE * GHOST_2_CHASE; break;
                  case 3: target.y += CELL_SIZE * GHOST_2_CHASE; break;
                  default: break;
               }

               target.x += target.x - ghost0_position.x;
               target.y += target.y - ghost0_position.y;

               break;
            }
            case 3:
            {
               if (CELL_SIZE * GHOST_3_CHASE <= sqrt(pow(position.x - pacman_position.x, 2) + pow(position.y - pacman_position.y, 2)))
               {
                  target = pacman_position;
               }
               else
               {
                  target = {0, CELL_SIZE * (MAP_HEIGHT - 1)};
               }
               break;
            }
         }
      }
   }
}

Position Ghost::get_position()
{
   return position;
}


GhostManager::GhostManager() :
   current_wave(0),
   wave_timer(LONG_SCATTER_DURATION),
   ghosts({Ghost(0), Ghost(1), Ghost(2), Ghost(3)})
{
}

void GhostManager::draw(bool flash, RenderWindow& window)
{
   for (Ghost& ghost : ghosts)
   {
      ghost.draw(flash, window);
   }
}

void GhostManager::reset(unsigned char level, const array<Position, 4>& ghost_positions)
{
   current_wave = 0;
   wave_timer = static_cast<unsigned short>(LONG_SCATTER_DURATION / pow(2, level));

   for (unsigned char a = 0; a < 4; a++)
   {
      ghosts[a].set_position(ghost_positions[a].x, ghost_positions[a].y);
   }

   for (Ghost& ghost : ghosts)
   {
      ghost.reset(ghosts[2].get_position(), ghosts[0].get_position());
   }
}

void GhostManager::update(unsigned char level, array<array<Cell, MAP_HEIGHT>, MAP_WIDTH>& map, Pacman& pacman)
{
   if (0 == pacman.get_energizer_timer())
   {
      if (0 == wave_timer)
      {
         if (current_wave < 7)
         {
            current_wave++;

            for (Ghost& ghost : ghosts)
            {
               ghost.switch_mode();
            }
         }

         if (1 == current_wave % 2)
         {
            wave_timer = CHASE_DURATION;
         }
         else if (2 == current_wave)
         {
            wave_timer = static_cast<unsigned short>(LONG_SCATTER_DURATION / pow(2, level));
         }
         else
         {
            wave_timer = static_cast<unsigned short>(SHORT_SCATTER_DURATION / pow(2, level));
         }
      }
      else
      {
         wave_timer--;
      }
   }

   for (Ghost& ghost : ghosts)
   {
      ghost.update(level, map, ghosts[0], pacman);
   }
}

\
bool map_collision(bool collect_pellets, bool use_door, short x, short y, array<array<Cell, MAP_HEIGHT>, MAP_WIDTH>& map)
{
   bool output = 0;

   float cell_x = x / static_cast<float>(CELL_SIZE);
   float cell_y = y / static_cast<float>(CELL_SIZE);

   for (unsigned char a = 0; a < 4; a++)
   {
      short cx = 0;
      short cy = 0;

      switch (a)
      {
         case 0: cx = static_cast<short>(floor(cell_x)); cy = static_cast<short>(floor(cell_y)); break;
         case 1: cx = static_cast<short>(ceil(cell_x)); cy = static_cast<short>(floor(cell_y)); break;
         case 2: cx = static_cast<short>(floor(cell_x)); cy = static_cast<short>(ceil(cell_y)); break;
         case 3: cx = static_cast<short>(ceil(cell_x)); cy = static_cast<short>(ceil(cell_y)); break;
      }

      if (0 <= cx && 0 <= cy && MAP_HEIGHT > cy && MAP_WIDTH > cx)
      {
         if (!collect_pellets)
         {
            if (Cell::Wall == map[cx][cy])
            {
               output = 1;
            }
            else if (!use_door && Cell::Door == map[cx][cy])
            {
               output = 1;
            }
         }
         else
         {
            if (Cell::Energizer == map[cx][cy])
            {
               output = 1;
               map[cx][cy] = Cell::Empty;
            }
            else if (Cell::Pellet == map[cx][cy])
            {
               map[cx][cy] = Cell::Empty;
            }
         }
      }
   }

   return output;
}

array<array<Cell, MAP_HEIGHT>, MAP_WIDTH> convert_sketch(const array<string, MAP_HEIGHT>& map_sketch, array<Position, 4>& ghost_positions, Pacman& pacman)
{
   array<array<Cell, MAP_HEIGHT>, MAP_WIDTH> output_map{};

   for (unsigned char a = 0; a < MAP_HEIGHT; a++)
   {
      for (unsigned char b = 0; b < MAP_WIDTH; b++)
      {
         output_map[b][a] = Cell::Empty;

         switch (map_sketch[a][b])
         {
            case '#':
               output_map[b][a] = Cell::Wall;
               break;
            case '=':
               output_map[b][a] = Cell::Door;
               break;
            case '.':
               output_map[b][a] = Cell::Pellet;
               break;
            case '0':
               ghost_positions[0] = {static_cast<short>(CELL_SIZE * b), static_cast<short>(CELL_SIZE * a)};
               break;
            case '1':
               ghost_positions[1] = {static_cast<short>(CELL_SIZE * b), static_cast<short>(CELL_SIZE * a)};
               break;
            case '2':
               ghost_positions[2] = {static_cast<short>(CELL_SIZE * b), static_cast<short>(CELL_SIZE * a)};
               break;
            case '3':
               ghost_positions[3] = {static_cast<short>(CELL_SIZE * b), static_cast<short>(CELL_SIZE * a)};
               break;
            case 'P':
               pacman.set_position(static_cast<short>(CELL_SIZE * b), static_cast<short>(CELL_SIZE * a));
               break;
            case 'o':
               output_map[b][a] = Cell::Energizer;
               break;
            default:
               break;
         }
      }
   }

   return output_map;
}

void draw_map(const array<array<Cell, MAP_HEIGHT>, MAP_WIDTH>& map, RenderWindow& window)
{
Sprite sprite;
Texture texture;
   texture.loadFromFile("Resources/Images/Map" + to_string(CELL_SIZE) + ".png");

   sprite.setTexture(texture);

   for (unsigned char a = 0; a < MAP_WIDTH; a++)
   {
      for (unsigned char b = 0; b < MAP_HEIGHT; b++)
      {
         sprite.setPosition(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));

         switch (map[a][b])
         {
            case Cell::Door:
               sprite.setTextureRect(IntRect(2 * CELL_SIZE, CELL_SIZE, CELL_SIZE, CELL_SIZE));
               window.draw(sprite);
               break;
            case Cell::Energizer:
               sprite.setTextureRect(IntRect(CELL_SIZE, CELL_SIZE, CELL_SIZE, CELL_SIZE));
               window.draw(sprite);
               break;
            case Cell::Pellet:
               sprite.setTextureRect(IntRect(0, CELL_SIZE, CELL_SIZE, CELL_SIZE));
               window.draw(sprite);
               break;
            case Cell::Wall:
            {
               bool down = 0;
               bool left = 0;
               bool right = 0;
               bool up = 0;

               if (b < MAP_HEIGHT - 1 && Cell::Wall == map[a][1 + b]) { down = 1; }
               if (0 < a)
               {
                  if (Cell::Wall == map[a - 1][b]) { left = 1; }
               }
               else
               {
                  left = 1;
               }

               if (a < MAP_WIDTH - 1)
               {
                  if (Cell::Wall == map[a + 1][b]) { right = 1; }
               }
               else
               {
                  right = 1;
               }

               if (0 < b && Cell::Wall == map[a][b - 1]) { up = 1; }

               sprite.setTextureRect(IntRect(CELL_SIZE * (down + 2 * (left + 2 * (right + 2 * up))), 0, CELL_SIZE, CELL_SIZE));
               window.draw(sprite);
               break;
            }
            default:
               break;
         }
      }
   }
}

void draw_text(bool center, unsigned short x, unsigned short y, const string& text, RenderWindow& window)
{
   short character_x = x;
   short character_y = y;

Sprite sprite;
Texture font_texture;
   font_texture.loadFromFile("Resources/Images/Font.png");

   unsigned char character_width = static_cast<unsigned char>(font_texture.getSize().x / 96);

   sprite.setTexture(font_texture);

   if (center)
   {
      character_x = static_cast<short>(round(0.5f * (CELL_SIZE * MAP_WIDTH - character_width * text.substr(0, text.find_first_of('\n')).size())));
      character_y = static_cast<short>(round(0.5f * (CELL_SIZE * MAP_HEIGHT - FONT_HEIGHT * (1 + count(text.begin(), text.end(), '\n')))));
   }

   for (string::const_iterator it = text.begin(); it != text.end(); ++it)
   {
      if ('\n' == *it)
      {
         if (center)
         {
            const size_t offset = 1 + static_cast<size_t>(it - text.begin());
            const size_t next_break = text.find_first_of('\n', offset);
            const size_t len = (next_break == string::npos ? text.size() : next_break) - offset;
            character_x = static_cast<short>(round(0.5f * (CELL_SIZE * MAP_WIDTH - character_width * text.substr(offset, len).size())));
         }
         else
         {
            character_x = x;
         }

         character_y += FONT_HEIGHT;
         continue;
      }

      sprite.setPosition(character_x, character_y);
      sprite.setTextureRect(IntRect(character_width * (*it - 32), 0, character_width, FONT_HEIGHT));

      character_x += character_width;

      window.draw(sprite);
   }
}

// drwa lives for chnaces 
void draw_lives_hearts(unsigned char lives, RenderWindow& window)
{
  
   static Texture heartsTexture;
   static bool textureLoaded = false;
   static bool textureAttempted = false;
   static unsigned short heartWidth = 0;  
   static unsigned short heartHeight = 0; 
   static unsigned short totalWidth = 0;   
   
   if (!textureAttempted)
   {
     
      textureLoaded = heartsTexture.loadFromFile("Resources/Images/Heart.png");
      if (!textureLoaded) {
         textureLoaded = heartsTexture.loadFromFile("Resources/Images/Heart.jpeg");
      }
      if (!textureLoaded) {
         textureLoaded = heartsTexture.loadFromFile("Resources/Images/Heart.jpg");
      }
      if (!textureLoaded) {
         textureLoaded = heartsTexture.loadFromFile("Resources/Images/Heart.png");
      }
      if (!textureLoaded) {
         textureLoaded = heartsTexture.loadFromFile("Resources/Images/Heart.jpeg");
      }
      
      if (textureLoaded) {
         totalWidth = static_cast<unsigned short>(heartsTexture.getSize().x);
         heartHeight = static_cast<unsigned short>(heartsTexture.getSize().y);
         // Assuming 3 hearts are arranged horizontally, divide by 3
         heartWidth = totalWidth / 3;
      }
      
      textureAttempted = true;
   }
   
   
   unsigned short start_x = CELL_SIZE * MAP_WIDTH - 8;  
   unsigned short start_y = 4; 
   
   if (textureLoaded && heartWidth > 0)
   {
      
      Sprite heartsSprite;
      heartsSprite.setTexture(heartsTexture);
      
     
     // draw and calcualte the lives and draw hearts accordingly to lives 
      if (lives > 0 && lives <= 3)
      {
         // Set texture rectangle to show only the hearts we have
         unsigned short visibleWidth = heartWidth * lives;
         heartsSprite.setTextureRect(IntRect(0, 0, visibleWidth, heartHeight));
         heartsSprite.setPosition(start_x - visibleWidth, start_y);
         window.draw(heartsSprite);
      }
   }
   else
   {
     
      const unsigned short HEART_SIZE = CELL_SIZE;
      const unsigned short SPACING = HEART_SIZE + 8;
      
      static Texture fallbackTexture;
      static bool fallbackCreated = false;
      
      if (!fallbackCreated)
      {
         Image heartImage;
         heartImage.create(CELL_SIZE, CELL_SIZE, Color(255, 0, 0));
         fallbackTexture.create(CELL_SIZE, CELL_SIZE);
         fallbackTexture.update(heartImage);
         fallbackCreated = true;
      }
      
      for (unsigned char i = 0; i < lives; i++)
      {
         Sprite heartSprite;
         heartSprite.setTexture(fallbackTexture);
         heartSprite.setPosition(start_x - (i * SPACING), start_y);
         window.draw(heartSprite);
      }
   }
}
// for scores records used linked lists bcz it always insert the score in sorting order 
struct ScoreNode {
    std::string name;
    int score;
    ScoreNode* next;
    ScoreNode(const std::string& n, int s) : name(n), score(s), next(nullptr) {}
};

class ScoreList {
    ScoreNode* head;
public:
    ScoreList() : head(nullptr) {}
    ~ScoreList() { clear(); }
    
    // always add the score in sorting order
    void add(const std::string& name, int scr) {
   
        ScoreNode* node = new ScoreNode(name, scr);
        
    
        if (!head || scr > head->score) {
            node->next = head; 
            head = node;
        } else {
            
            ScoreNode* cur = head;
            while (cur->next && cur->next->score >= scr) {
                cur = cur->next;  
            }
            
            node->next = cur->next; 
            cur->next = node;
        }
        
       // always show the top 5 highest scores in sorted order of linked list
        ScoreNode* cur = head; 
        int count = 1;
        while (cur && cur->next) {
            if (count == 5) {
                
                ScoreNode* del = cur->next; 
                cur->next = nullptr;
                while(del) { 
                    ScoreNode* nxt = del->next; 
                    delete del;  // easy to dlt the score
                    del = nxt; 
                }
                break;
            }
            cur = cur->next; 
            ++count;
        }
    }
    
    // displaying the score in view score
    string getListInfo() {
        std::string info = "Linked List: ";
        ScoreNode* cur = head;
        int count = 0;
        while (cur && count < 5) {
            info += cur->name + "(" + std::to_string(cur->score) + ")";
            if (cur->next) info += " -> ";
            cur = cur->next;
            count++;
        }
        if (!head) info += "Empty";
        return info;
    }
    
   //  counting the total no of records
    int getNodeCount() {
        int count = 0;
        ScoreNode* cur = head;
        while (cur) {
            count++;
            cur = cur->next;
        }
        return count;
    }
    
    ScoreNode* getHead() { return head; }  
    
   
    void clear() {
        while(head) { 
            ScoreNode* nxt = head->next; 
            delete head;  
            head = nxt;  
        }
    }
};

// lobby creation
void draw_lobby_text(unsigned short x, unsigned short y, const string& text, RenderWindow& window, bool highlight = false, bool center = false)
{
   short character_x = x;
   short character_y = y;

Sprite sprite;
Texture font_texture;
   font_texture.loadFromFile("Resources/Images/Font.png");

   unsigned char character_width = static_cast<unsigned char>(font_texture.getSize().x / 96);

   sprite.setTexture(font_texture);
   

   float scale = highlight ? 2.5f : 2.0f;
   sprite.setScale(scale, scale);

   // Calculate text width for centering
   float text_width = 0;
   for (string::const_iterator it = text.begin(); it != text.end(); ++it)
   {
      if (*it == '\n') break;
      text_width += character_width * scale;
   }

   
   if (center)
   {
      unsigned short window_width = window.getSize().x;
      character_x = static_cast<short>((window_width - text_width) / 2);
   }

   short start_x = character_x;

   for (string::const_iterator it = text.begin(); it != text.end(); ++it)
   {
      if ('\n' == *it)
      {
        
         if (center)
         {
            
            size_t offset = 1 + static_cast<size_t>(it - text.begin());
            size_t next_break = text.find_first_of('\n', offset);
            size_t len = (next_break == string::npos ? text.size() : next_break) - offset;
            string next_line = text.substr(offset, len);
            float next_line_width = next_line.size() * character_width * scale;
            character_x = static_cast<short>((window.getSize().x - next_line_width) / 2);
         }
         else
         {
            character_x = start_x;
         }
         character_y += FONT_HEIGHT * scale;
         continue;
      }

      sprite.setPosition(character_x, character_y);
      sprite.setTextureRect(IntRect(character_width * (*it - 32), 0, character_width, FONT_HEIGHT));

      character_x += character_width * scale;

      window.draw(sprite);
   }
}


int showLobby(ScoreNode* head) {
   
RenderWindow lobbyWindow(VideoMode(800, 700), "Pac-Man Lobby", Style::Titlebar | Style::Close);

    // adding the music
    Music lobbyMusic;
    if (!lobbyMusic.openFromFile("Resources/Music/pacman_theme.wav")) {
        // Try alternative format
        if (!lobbyMusic.openFromFile("Resources/Music/pacman_theme.wav")) {
            std::cerr << "Failed to load lobby theme music.\n";
        }
    }
    if (lobbyMusic.openFromFile("Resources/Music/pacman_theme.wav") || 
        lobbyMusic.openFromFile("Resources/Music/pacman_theme.wav")) {
        lobbyMusic.setLoop(true);
        lobbyMusic.play();
    }

    // background image 
    const string lobbyBackgroundImage = "Resources/Images/Lobby.jpg";

    // Load background image
Texture bgTexture;
Sprite bgSprite;
    bool imageLoaded = bgTexture.loadFromFile(lobbyBackgroundImage);

    if (imageLoaded) {
        bgSprite.setTexture(bgTexture);

        float windowWidth  = lobbyWindow.getSize().x;
        float windowHeight = lobbyWindow.getSize().y;

        float textureWidth  = bgTexture.getSize().x;
        float textureHeight = bgTexture.getSize().y;

        float scaleX = windowWidth / textureWidth;
        float scaleY = windowHeight / textureHeight;

        bgSprite.setScale(scaleX, scaleY);
    }

   
Color bgColor(25, 25, 112);  // dark blue

    
    unsigned short startButtonWidth = 300;  
    unsigned short startButtonHeight = 60;  
    unsigned short startButtonY = 0;  
    unsigned short startButtonX = 0;  

    unsigned short viewScoreButtonY = 0;  
    unsigned short viewScoreButtonX = 0;  

    bool startClicked = false;
    bool viewScoreClicked = false;
    bool startButtonHovered = false;
    bool viewScoreButtonHovered = false;

    while (lobbyWindow.isOpen()) {
Event event;
        while (lobbyWindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                lobbyMusic.stop();  // Stop music when closing
                lobbyWindow.close();
                return 0;
            }

Vector2i mousePos = Mouse::getPosition(lobbyWindow);

            // Start button click
            if (event.type == Event::MouseButtonPressed &&
                event.mouseButton.button == Mouse::Left) {
                
                if (mousePos.x >= startButtonX && mousePos.x <= startButtonX + startButtonWidth &&
                    mousePos.y >= startButtonY && mousePos.y <= startButtonY + startButtonHeight) {
                    lobbyMusic.stop();  // Stop lobby music when starting game
                    startClicked = true;
                    lobbyWindow.close();
                }
                else if (mousePos.x >= viewScoreButtonX && mousePos.x <= viewScoreButtonX + startButtonWidth &&
                         mousePos.y >= viewScoreButtonY && mousePos.y <= viewScoreButtonY + startButtonHeight) {
                    viewScoreClicked = true;
                    lobbyWindow.close();
                }
            }

            // Press Enter to start, V to view scores
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Enter) {
                    lobbyMusic.stop();  // Stop lobby music when starting game
                    startClicked = true;
                    lobbyWindow.close();
                }
                else if (event.key.code == Keyboard::V) {
                    viewScoreClicked = true;
                    lobbyWindow.close();
                }
            }

            // Hover detection
            if (event.type == Event::MouseMoved) {
                startButtonHovered = (mousePos.x >= startButtonX &&
                                     mousePos.x <= startButtonX + startButtonWidth &&
                                     mousePos.y >= startButtonY &&
                                     mousePos.y <= startButtonY + startButtonHeight);
                
                viewScoreButtonHovered = (mousePos.x >= viewScoreButtonX &&
                                         mousePos.x <= viewScoreButtonX + startButtonWidth &&
                                         mousePos.y >= viewScoreButtonY &&
                                         mousePos.y <= viewScoreButtonY + startButtonHeight);
            }
        }

        // DRAW SECTION
        lobbyWindow.clear(bgColor);

        if (imageLoaded) {
            lobbyWindow.draw(bgSprite);
        }

        unsigned short window_width = lobbyWindow.getSize().x;
        unsigned short window_height = lobbyWindow.getSize().y;

        // Title - centered horizontally, near top
        draw_lobby_text(0, 30, "PAC-MAN", lobbyWindow, true, true);

        // Calculate centered vertical positions for buttons
        unsigned short center_y = window_height / 2;
        unsigned short startButtonY_pos = center_y- 80;  // Above center
        unsigned short viewScoreButtonY_pos = center_y + 10;  // Below center

        // Update button click areas to match text positions (centered horizontally)
        startButtonX = (window_width - startButtonWidth) / 2;
        startButtonY = startButtonY_pos;
        viewScoreButtonX = (window_width - startButtonWidth) / 2;
        viewScoreButtonY = viewScoreButtonY_pos;

        // Start Button - centered both horizontally and vertically
        if (startButtonHovered)
            draw_lobby_text(0, startButtonY_pos, "START GAME", lobbyWindow, true, true);
        else
            draw_lobby_text(0, startButtonY_pos, "START GAME", lobbyWindow, false, true);

        // View Score Button - centered both horizontally and vertically
        if (viewScoreButtonHovered)
            draw_lobby_text(0, viewScoreButtonY_pos, "VIEW SCORE", lobbyWindow, true, true);
        else
            draw_lobby_text(0, viewScoreButtonY_pos, "VIEW SCORE", lobbyWindow, false, true);

        // Instructions - centered horizontally, near bottom
        draw_lobby_text(0, window_height - 80, "Press START/Enter or VIEW SCORE/V", lobbyWindow, false, true);

        lobbyWindow.display();
    }

    lobbyMusic.stop();  // Stop music before returning
    if (viewScoreClicked) return 2;
    if (startClicked) return 1;
    return 0;
}


void showScoresScreen(Font& font, const std::string& bgImagePath, ScoreNode* head) {
RenderWindow win(VideoMode(340, 280), "Top Scores - Linked List");
Texture bgTexture; bgTexture.loadFromFile(bgImagePath); Sprite bgSprite(bgTexture);
Text title("TOP 5 SCORES", font, 26); title.setFillColor(Color::Yellow); title.setPosition(60, 10);
    
  // displaying the linked list in gui interfrence
    string listInfo = "Linked List Traversal:";
Text infoText(listInfo, font, 14); 
    infoText.setFillColor(Color::Cyan);
    infoText.setPosition(20, 40);
    
    vector<Text> lines;
    
   
    ScoreNode* cur = head;  
    int node_count = 0;
    for (int i=0; i<5; ++i) {
        string row = to_string(i+1) + ". ";
        if (cur) { 
            // LINKED LIST: Access current node data
            row += cur->name + " - " + to_string(cur->score);
            if (cur->next) {
                row += " -> next";
            } else {
                row += " -> null";
            }
            cur = cur->next;  
            node_count++;
        }
        else row += "[empty]";
        lines.emplace_back(row, font, 18);
        lines.back().setPosition(30, 65 + i*35);
    }
    
    // Show node count
    string countText = "Nodes: " + std::to_string(node_count);
Text countDisplay(countText, font, 16);
    countDisplay.setFillColor(Color::Green);
    countDisplay.setPosition(20, 250);
    while (win.isOpen()) {
Event e;
        while (win.pollEvent(e)) {
            if (e.type == Event::Closed || (e.type == Event::KeyPressed)) win.close();
        }
        win.clear(); 
        win.draw(bgSprite); 
        win.draw(title);
        win.draw(infoText);
        win.draw(countDisplay);
        for (auto& l : lines) win.draw(l);
        win.display();
    }
}

std::string askName(Font& font, const std::string& bgImagePath) {
RenderWindow window(VideoMode(350, 100), "Enter Name");
Texture bgTexture; bgTexture.loadFromFile(bgImagePath); Sprite bgSprite(bgTexture);
Text prompt("Enter your name:", font, 20); prompt.setPosition(8, 10);
    std::string nameStr; Text nameDisplay("", font, 22); nameDisplay.setPosition(8, 44);
    while (window.isOpen()) {
Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) window.close();
            if (event.type == Event::TextEntered) {
                if (event.text.unicode == '\b') { if (!nameStr.empty()) nameStr.pop_back();
                } else if (event.text.unicode == '\r' || event.text.unicode == '\n') {
                    if (!nameStr.empty()) { window.close(); return nameStr; }
                } else if (event.text.unicode < 128 && nameStr.size() < 12 && event.text.unicode >= 32)
                    nameStr += static_cast<char>(event.text.unicode);
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter)
                if (!nameStr.empty()) { window.close(); return nameStr; }
        }
        nameDisplay.setString(nameStr + '_');
        window.clear(); window.draw(bgSprite); window.draw(prompt); window.draw(nameDisplay); window.display();
    }
    return "Player";
}

void playGame(const std::string& playerName, Font& font, const std::string& bgImagePath) {
RenderWindow win(VideoMode(360, 120), "Playing Game!");
Texture bgTexture; bgTexture.loadFromFile(bgImagePath); Sprite bgSprite(bgTexture);
Text info("Playing as: " + playerName, font, 20); info.setPosition(18, 38);
    int frame = 0;
    while (win.isOpen() && frame < 70) {
Event e; while (win.pollEvent(e)) if (e.type == Event::Closed) win.close();
        win.clear(); win.draw(bgSprite); win.draw(info); win.display(); ++frame;
    }
}

// getiing the name of player here
string ask_player_name(RenderWindow& window) {
Font font;
    font.loadFromFile("Resources/Images/Font.ttf"); 
Text prompt("Enter your name (max 10 chars):", font, 22);
    prompt.setFillColor(Color::White);
    prompt.setPosition(20, 40);
Text input_text("", font, 22);
    input_text.setFillColor(Color::Yellow);
    input_text.setPosition(20, 80);
    
    std::string name;
    bool done = false;
    while (!done && window.isOpen()) {
Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) window.close();
            if (event.type == Event::TextEntered) {
                if (event.text.unicode == 8) { // backspace
                    if (!name.empty()) name.pop_back();
                } else if (event.text.unicode == 13) { // enter
                    done = !name.empty();
                } else if (event.text.unicode < 128 && isalnum(event.text.unicode) && name.size() < 10) {
                    name += static_cast<char>(event.text.unicode);
                }
            }
        }
        window.clear(Color::Black);
        window.draw(prompt);
        input_text.setString(name + '_');
        window.draw(input_text);
        window.display();
    }
    return name.empty() ? "Player" : name;
}

// Saving score to file named the  scores.txxt
void save_score(const string& name, int score) {
    ofstream ofs("scores.txt", ios::app);
    if (ofs.is_open()) {
        ofs << name << ' ' << score << '\n';
        ofs.close();
    }
}

// get the scores for scores.txt
void load_scores_from_file(ScoreList& score_list) {
    ifstream ifs("scores.txt");
    if (ifs.is_open()) {
        string name;
        int score;
        while (ifs >> name >> score) {
            score_list.add(name, score);
        }
        ifs.close();
    }
}

// Read top 5 scores from file
vector<pair<string, int>> get_top_scores() { // just read the top 5 highest scores
    vector<pair<string, int>> scores;
    ifstream ifs("scores.txt");
    string name;
    int score;
    while (ifs >> name >> score) {
        scores.emplace_back(name, score);
    }
    std::sort(scores.begin(), scores.end(), [](auto &a, auto &b) { return a.second > b.second; });
    if (scores.size() > 5) scores.resize(5);
    return scores;
}


void showViewScoreScreen(ScoreNode* head) {
RenderWindow viewWindow(VideoMode(600, 500), "Top 5 Scores", Style::Titlebar | Style::Close);
    
  
    const std::string viewScoreBackgroundImage = "Resources/Images/ViewScore.png";
    

Texture bgTexture;
Sprite bgSprite;
    bool imageLoaded = bgTexture.loadFromFile(viewScoreBackgroundImage);
    
    if (imageLoaded) {
        bgSprite.setTexture(bgTexture);
        
        float windowWidth  = viewWindow.getSize().x;
        float windowHeight = viewWindow.getSize().y;
        
        float textureWidth  = bgTexture.getSize().x;
        float textureHeight = bgTexture.getSize().y;
        
        float scaleX = windowWidth / textureWidth;
        float scaleY = windowHeight / textureHeight;
        
        bgSprite.setScale(scaleX, scaleY);
    }
    
    
Color bgColor(25, 25, 112);
    
    while (viewWindow.isOpen()) {
Event event;
        while (viewWindow.pollEvent(event)) {
            if (event.type == Event::Closed || 
                (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)) {
                viewWindow.close();
            }
        }
        
        viewWindow.clear(bgColor);
        
        // Draw background image if loaded
        if (imageLoaded) {
            viewWindow.draw(bgSprite);
        }
        
        
        draw_lobby_text(0, 50, "TOP 5 SCORES", viewWindow, true, true);
        
        ScoreNode* cur = head;
        for (int i = 0; i < 5; ++i) {
            string row = to_string(i + 1) + ". ";
            
            if (cur) {
                row += cur->name + " - " + to_string(cur->score);
                cur = cur->next;
            } else {
                row += "[No score yet]";
            }
            
            draw_lobby_text(0, 150 + i * 50, row, viewWindow, false, true);
        }
        
        draw_lobby_text(0, viewWindow.getSize().y - 50, "Press ESC to close", viewWindow, false, true);
        
        viewWindow.display();
    }
}

// Ask for player name using bitmap font
std::string ask_player_name_bitmap() {
RenderWindow nameWindow(VideoMode(500, 200), "Enter Your Name", Style::Titlebar | Style::Close);
Color bgColor(0, 0, 0);
    
    std::string name = "";
    bool done = false;
    
    while (nameWindow.isOpen() && !done) {
Event event;
        while (nameWindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                nameWindow.close();
                return "Player";
            }
            
            if (event.type == Event::TextEntered) {
                if (event.text.unicode == 8) { // Backspace
                    if (!name.empty()) name.pop_back();
                } else if (event.text.unicode == 13 || event.text.unicode == 10) { // Enter
                    if (!name.empty()) {
                        done = true;
                        nameWindow.close();
                    }
                } else if (event.text.unicode >= 32 && event.text.unicode < 128 && name.size() < 15) {
                    name += static_cast<char>(event.text.unicode);
                }
            }
        }
        
        nameWindow.clear(bgColor);
        
        draw_lobby_text(0, 30, "Enter your name:", nameWindow, false, true);
        draw_lobby_text(0, 100, name + "_", nameWindow, false, true);
        draw_lobby_text(0, nameWindow.getSize().y - 40, "Press Enter when done", nameWindow, false, true);
        
        nameWindow.display();
    }
    
    return name.empty() ? "Player" : name;
}

// main function start ----//
int main()
{
   // create the score object for 
   ScoreList score_list;  
   string player_name = "Player";  
   int current_score = 0;  // track the score 
   unsigned score_timer = 0;  // timer
   
   
   load_scores_from_file(score_list);
   
   
   while (true) {
     
      int lobby_result = showLobby(score_list.getHead());
      
      if (lobby_result == 0) {
        
         break;
      }
      else if (lobby_result == 2) {
        
         showViewScoreScreen(score_list.getHead());
         continue;
      }
      
   
      player_name = ask_player_name_bitmap();
   
 
      bool game_won = 0;
      unsigned lag = 0;
      unsigned char level = 0;
      unsigned char lives = 3;  
      chrono::time_point<chrono::steady_clock> previous_time;

   array<string, MAP_HEIGHT> map_sketch = {
      " ################### ",
      " #........#........# ",
      " #o##.###.#.###.##o# ",
      " #.................# ",
      " #.##.#.#####.#.##.# ",
      " #....#...#...#....# ",
      " ####.### # ###.#### ",
      "    #.#   0   #.#    ",
      "#####.# ##=## #.#####",
      "     .  #123#  .   ",
      "#####.# ##### #.#####",
      "    #.#       #.#    ",
      " ####.# ##### #.#### ",
      " #........#........# ",
      " #.##.###.#.###.##.# ",
      " #o.#.....P.....#.o# ",
      " ##.#.#.#####.#.#.## ",
      " #....#...#...#....# ",
      " #.######.#.######.# ",
      " #.................# ",
      " ################### "
   };

   array<array<Cell, MAP_HEIGHT>, MAP_WIDTH> map{};
   array<Position, 4> ghost_positions;
Event event;

RenderWindow window(VideoMode(CELL_SIZE * MAP_WIDTH * SCREEN_RESIZE, (FONT_HEIGHT + CELL_SIZE * MAP_HEIGHT) * SCREEN_RESIZE), "Pac-Man", Style::Close);
   window.setView(View(FloatRect(0, 0, CELL_SIZE * MAP_WIDTH, FONT_HEIGHT + CELL_SIZE * MAP_HEIGHT)));

   GhostManager ghost_manager;
   Pacman pacman;

   srand(static_cast<unsigned>(time(0)));

   map = convert_sketch(map_sketch, ghost_positions, pacman);
   ghost_manager.reset(level, ghost_positions);

   previous_time = chrono::steady_clock::now();

 
   sf::Music backgroundMusic;
   if (!backgroundMusic.openFromFile("Resources/Music/pacman_theme.wav")) {
      std::cerr << "Failed to load background music.\n";
   }
   backgroundMusic.setLoop(true);
   backgroundMusic.play();

   sf::SoundBuffer chompBuffer;
   if (!chompBuffer.loadFromFile("Resources/Music/pacman_chomp.wav")) {
      std::cerr << "Failed to load chomping sound.\n";
   }
   sf::Sound chompSound;
   chompSound.setBuffer(chompBuffer);

  
   sf::Music deathMusic;
   if (!deathMusic.openFromFile("Resources/Music/pacman_death.wav")) {
      std::cerr << "Failed to load death music.\n";
   }
   deathMusic.setLoop(false); 
   bool deathSoundPlayed = false;  
   
   // Stack for pause/resume functionality
   SimpleStack<bool, 10> pauseStack;
   bool isPaused = false;
   bool pauseKeyPressed = false;

   while (window.isOpen())
   {
      unsigned delta_time = static_cast<unsigned>(chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - previous_time).count());
      lag += delta_time;
      previous_time += chrono::microseconds(delta_time);

      while (FRAME_DURATION <= lag)
      {
         lag -= FRAME_DURATION;

         while (window.pollEvent(event))
         {
            if (Event::Closed == event.type)
            {
             
               if (current_score > 0) {
                  score_list.add(player_name, current_score);
                  save_score(player_name, current_score);
               }
               window.close();
               break;  
            }
            
            // Pause/Resume functionality using stack
            if (event.type == Event::KeyPressed)
            {
               if (event.key.code == Keyboard::P)
               {
                  if (!pauseKeyPressed)
                  {
                     pauseKeyPressed = true;
                     if (!isPaused)
                     {
                        // Pause the game - push to stack
                        pauseStack.push(true);
                        isPaused = true;
                        backgroundMusic.pause();
                     }
                     else
                     {
                        // Resume the game - pop from stack
                        if (!pauseStack.empty())
                        {
                           pauseStack.pop();
                           isPaused = !pauseStack.empty();
                           if (!isPaused)
                           {
                              backgroundMusic.play();
                           }
                        }
                     }
                  }
               }
            }
            
            if (event.type == Event::KeyReleased)
            {
               if (event.key.code == Keyboard::P)
               {
                  pauseKeyPressed = false;
               }
            }
         }
         
        
         if (!window.isOpen()) {
            break;
         }

        
         static bool wasDead = false;
         
         if (pacman.get_dead() && !wasDead)
         {
           
            backgroundMusic.stop();
            deathMusic.play();
            deathSoundPlayed = true;
            wasDead = true;
         }

       
         if (pacman.get_dead() && pacman.get_animation_over())
         {
            
            if (wasDead && lives > 0) {
               lives--;
               wasDead = false;  //reset the flag
            }
            
            if (lives > 0)
            {
               
               map = convert_sketch(map_sketch, ghost_positions, pacman);
               ghost_manager.reset(level, ghost_positions);
               pacman.reset();
               deathSoundPlayed = false;  
               
               
               deathMusic.stop();
               backgroundMusic.play();
            }
            else
            {
               
            }
         }
         else if (!pacman.get_dead())
         {
            wasDead = false;  
         }

         // Only update game if not paused
         if (!isPaused && !game_won && !pacman.get_dead())
         {
            game_won = 1;

            pacman.update(level, map, current_score, chompSound);
            ghost_manager.update(level, map, pacman);
            
           

            for (const array<Cell, MAP_HEIGHT>& column : map)
            {
               for (const Cell& cell : column)
               {
                  if (Cell::Pellet == cell)
                  {
                     game_won = 0;
                     break;
                  }
               }

               if (!game_won)
               {
                  break;
               }
            }

            if (game_won)
            {
                   
               pacman.set_animation_timer(0);
            }
         }
         else if (!isPaused && Keyboard::isKeyPressed(Keyboard::Enter))
         {
            
            if (pacman.get_dead() && pacman.get_animation_over())
            {
               if (lives == 0)
               {
                  
                  score_list.add(player_name, current_score);
                  save_score(player_name, current_score);
                  current_score = 0;
                  level = 0;
                  lives = 3;  
                  deathSoundPlayed = false;
                  deathMusic.stop();
                  window.close();
                  break;
               }
               
            }
            else if (game_won)
            {
              // if all pellets eat then next level will come
               game_won = 0;
               level++;
             
               current_score += 5000;  // for every completing the 5000 bonus will be given

               map = convert_sketch(map_sketch, ghost_positions, pacman);
               ghost_manager.reset(level, ghost_positions);
               pacman.reset();
            }
         }

         if (FRAME_DURATION > lag)
         {
         
            bool is_game_over = pacman.get_dead() && pacman.get_animation_over() && lives == 0;
            
            if (is_game_over)
            {
               
               window.clear(Color(135, 206, 250));  
               backgroundMusic.stop();
            }
            else
            {
               window.clear();
            }

            if (!game_won && !pacman.get_dead())
            {
               draw_map(map, window); //display the score at end
               ghost_manager.draw(GHOST_FLASH_START >= pacman.get_energizer_timer(), window);
               draw_text(0, 0, CELL_SIZE * MAP_HEIGHT, "Level: " + to_string(1 + level), window);
               
               draw_text(0, 0, CELL_SIZE * MAP_HEIGHT + FONT_HEIGHT, "Score: " + to_string(current_score), window);
               
               draw_lives_hearts(lives, window);
            }

            pacman.draw(game_won, window);

            if (pacman.get_animation_over())
            {
               if (game_won)
               {
                  draw_text(1, 0, 0, "Next level!", window);
               }
               else if (lives == 0)
               {
                 
Texture font_texture;
                  font_texture.loadFromFile("Resources/Images/Font.png");
                  unsigned char character_width = static_cast<unsigned char>(font_texture.getSize().x / 96);
                  
                  unsigned short screen_width = CELL_SIZE * MAP_WIDTH;
                  unsigned short screen_height = CELL_SIZE * MAP_HEIGHT;
                  
                  
                  unsigned short center_y = screen_height / 2;
                  unsigned short line_spacing = FONT_HEIGHT * 8;  
                  
                 // gameover interefrence
                  std::string gameOverText = "Game over";
                  unsigned short gameOverX = (screen_width - gameOverText.length() * character_width) / 2;
                  unsigned short gameOverY = center_y - line_spacing * 2;  
                  draw_text(0, gameOverX, gameOverY, gameOverText, window);
                  
                  
                  string finalScore = "Final Score: " + std::to_string(current_score);
                  unsigned short scoreX = (screen_width - finalScore.length() * character_width) / 2;
                  unsigned short scoreY = center_y - line_spacing / 2;  
                  draw_text(0, scoreX, scoreY, finalScore, window);
                  
                 
                  string playerInfo = "Player: " + player_name;
                  unsigned short playerX = (screen_width - playerInfo.length() * character_width) / 2;
                  unsigned short playerY = center_y + line_spacing / 2; 
                  draw_text(0, playerX, playerY, playerInfo, window);
                  
                 
                  string instructionText = "Press Enter to return";
                  unsigned short instX = (screen_width - instructionText.length() * character_width) / 2;
                  unsigned short instY = center_y + line_spacing * 2;  
                  draw_text(0, instX, instY, instructionText, window);
               }
               else if (lives > 0)
               {
                  // lives for respawn pacman ko phirse zinda karo
                  Texture font_texture;
                  font_texture.loadFromFile("Resources/Images/Font.png");
                  unsigned char character_width = static_cast<unsigned char>(font_texture.getSize().x / 96);
                  
                  unsigned short screen_width = CELL_SIZE * MAP_WIDTH;
                  unsigned short screen_height = CELL_SIZE * MAP_HEIGHT;
                  
                  unsigned short center_y = screen_height / 2;
                  
                  string respawnText = "Lives left: " + std::to_string(lives);
                  unsigned short respawnX = (screen_width - respawnText.length() * character_width) / 2;
                  unsigned short respawnY = center_y - FONT_HEIGHT * 2;
                  draw_text(0, respawnX, respawnY, respawnText, window);
                  
                  string pressEnterText = "Press Enter to continue";
                  unsigned short enterX = (screen_width - pressEnterText.length() * character_width) / 2;
                  unsigned short enterY = center_y;
                  draw_text(0, enterX, enterY, pressEnterText, window);
               }
            }
            
            // Display pause menu if game is paused
            if (isPaused)
            {
               Texture font_texture;
               font_texture.loadFromFile("Resources/Images/Font.png");
               unsigned char character_width = static_cast<unsigned char>(font_texture.getSize().x / 96);
               
               unsigned short screen_width = CELL_SIZE * MAP_WIDTH;
               unsigned short screen_height = CELL_SIZE * MAP_HEIGHT;
               
               // Draw semi-transparent overlay
               RectangleShape overlay;
               overlay.setSize(Vector2f(screen_width, screen_height));
               overlay.setFillColor(Color(0, 0, 0, 180));
               window.draw(overlay);
               
               unsigned short center_y = screen_height / 2;
               unsigned short line_spacing = FONT_HEIGHT * 4;
               
               // Pause text
               string pauseText = "GAME PAUSED";
               unsigned short pauseX = (screen_width - pauseText.length() * character_width) / 2;
               unsigned short pauseY = center_y - line_spacing;
               draw_text(0, pauseX, pauseY, pauseText, window);
               
               // Resume instruction
               string resumeText = "Press P to Resume";
               unsigned short resumeX = (screen_width - resumeText.length() * character_width) / 2;
               unsigned short resumeY = center_y;
               draw_text(0, resumeX, resumeY, resumeText, window);
            }

            window.display();
         }
      }
   }
}}
