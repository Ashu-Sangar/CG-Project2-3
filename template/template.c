#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h> 
#include <math.h>
#include <string.h>
#include <stdbool.h>
#ifdef __APPLE__  // include Mac OS X verions of headers
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else // non-Mac OS X operating systems
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#endif  // __APPLE__
#include "initShader.h"
#include "tempLib.h"

//prototypes
vec4 map_coords(int x, int y);  
void create_rotation();
void init_block();
void init_grassblock();
void init_texture(float x, float y);
void generate_pyramid(int x_size, int z_size);
void display_sun();
void forward();
void backward();
void slide_left();
void slide_right();
void turn_left();
void turn_right();
bool can_move_inside_maze(int row, int col, int direction);
bool can_reenter_maze(int exit_direction, int current_direction, char movement_type);
void print_location(void);
void shortest_path(int player_row, int player_col, int direction, bool inside_maze);

// mouse, rotation, and scaling variables
float scale_factor = 1.0f; 
int left_button_down = 0;
int right_button_down = 0;
int dragging = 0;
int has_moved = 0;
vec4 previous_point = {0.0f, 0.0f, 0.0f, 1.0f}; // To store the previous point during drag
vec4 initial_point = {0.0f, 0.0f, 0.0f, 1.0f};
vec4 final_point = {0.0f, 0.0f, 0.0f, 1.0f};
mat4 scaling_matrix = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
mat4 rotation_matrix = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
mat4 prev_ctm = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
mat4 ctm = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
GLuint ctm_location;

float lrbt = .75;
float near = -.75;

// Model view and projection variables
GLuint model_view_location;
GLuint projection_location;
mat4 model_view = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
mat4 projection = {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
float eye_x = 0.0;
float eye_y = 0.0;
float eye_z = 0.0;
float at_x = 0.0;
float at_y = 0.0;
float at_z = 0.0;

//cube and platform global variables
float scale_cube = 1.00f;
int num_vertices_per_block = 36; 
int num_blocks = 0; 
int num_vertices = 0;
int x_size;
int z_size; 
int maze_x_size = 0;
int maze_z_size = 0;

//array global variable
vec4 *positions = NULL;
vec2 *tex_coords = NULL;
vec4 *block_positions = NULL; // Store positions for a single block
vec2 *block_tex_coords = NULL; 
vec4 *normals = NULL;

//sun global varibles
vec4 *sun_positions = NULL;
vec2 *sun_tex_coords = NULL;
int num_vertices_sun = 0; // Number of vertices for the sun
vec4 sun_position = {0.0f, 16.0f, 0.0f, 1.0f};
GLuint light_position_location;

//platform reset:
bool resetting = false; // Animation state
int reset_step = 0; // Current step in the reset process
int reset_steps = 30; // Total number of steps for the reset
mat4 delta_ctm; // Incremental change matrix

//player tracking
int player_row = -1; // -1 just for init purposes
int player_col = -1;
int direction = 0; // 0 = North, 1 = East, 2 = South, 3 = West
int exit_direction = -1; // -1 for no direction, but otherwise follows same rules as avove
bool inside_maze = false; // is the player inside the maze? 

//left hand rule
int maze_solving_in_progress = 0;

//lighting stuff
GLuint enable_light_location;
int enable_light = 0;
GLuint ambient_light_location;
int ambient_light = 1;
GLuint diffuse_light_location;
int diffuse_light = 1;
GLuint specular_light_location;
int specular_light = 1;
GLuint no_light_location;
int no_light = 0;

// Animation globals
int is_animating = 0; // Animation state
int num_steps = 0; // Current step
int max_steps = 40; // Max number of steps for animation
int reset_animation = 0;
int forward_animation = 0;
int backward_animation = 0;
int slide_left_animation = 0;
int slide_right_animation = 0;
int look_left_animation = 0;
int look_right_animation = 0;
float eye_target = 0.0;
vec4 lateral_animation_vec;
float target_eye_x, target_eye_z;
float target_at_x, target_at_z;

// gonna make the maze using a 2d array of structs
typedef struct {
    bool top_wall;    // true if the wall exists
    bool bottom_wall;
    bool left_wall;  
    bool right_wall; 
} Cell;

Cell **maze;

//this and the node are for shortest path
// direction offsets: (North, East, South, West) used to calculate position in next cell during BFS
int d_row[] = {-1, 0, 1, 0};
int d_col[] = {0, 1, 0, -1};

typedef struct {
    int row, col;  // current position
    int parent_row, parent_col;  // parent position for path reconstruction
} Node;

//allocate mem for maze size
void allocate_maze(int rows, int cols) {
    maze = (Cell **)malloc(rows * sizeof(Cell *));
    if (!maze) {
        fprintf(stderr, "Failed to allocate memory for maze rows.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows; i++) {
        maze[i] = (Cell *)malloc(cols * sizeof(Cell));
        if (!maze[i]) {
            fprintf(stderr, "Failed to allocate memory for maze columns.\n");
            exit(EXIT_FAILURE);
        }
    }
}

//cleanup the rows and then the pointers for each row (teardown)
void free_maze(int rows) {
    if (maze) {
        for (int i = 0; i < rows; i++) {
            if (maze[i]) free(maze[i]);
        }
        free(maze);
        maze = NULL;
    }
}

// create the maze with bording walls + the entrance and exit and all the walls inside should be disabled for now
void init_maze(int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // set border walls
            maze[i][j].top_wall = (i == 0);              
            maze[i][j].bottom_wall = (i == rows - 1);    
            maze[i][j].left_wall = (j == 0);             
            maze[i][j].right_wall = (j == cols - 1);      
        }
    }

    // create entrance (remove bottom left, bottom wall)
    maze[rows - 1][0].bottom_wall = false;

    // create exit (remove top right, top wall)
    maze[0][cols - 1].top_wall = false;
}

// helper function for picking rand values 
int rand_between(int low, int high) {
    if (low >= high) {
        return low; // avoid invalid range
    }
    return low + rand() % (high - low);
}

//function to randomly generate the maze
void generate_maze(int row_start, int row_end, int col_start, int col_end) {
    // if the chamber is too small, stop recursion here
    if (row_end - row_start < 1 || col_end - col_start < 1) {
        return;
    }

    // randomly pick a dividing row and column
    int divide_row = rand_between(row_start + 1, row_end); // ensure its not on a boundary
    int divide_col = rand_between(col_start + 1, col_end); 

    // add walls vertically and horisontally from that point

    //vert wall
    for (int r = row_start; r <= row_end; r++) {
        maze[r][divide_col - 1].right_wall = true; // east wall of left section
        maze[r][divide_col].left_wall = true;     // west wall of right section
    }

    //horizontal wall
    for (int c = col_start; c <= col_end; c++) {
        maze[divide_row - 1][c].bottom_wall = true; // south wall of top section
        maze[divide_row][c].top_wall = true;       // north wall of bottom section
    }

    // next, randomly open one wall in three of the four sections
    bool opened[4] = {false, false, false, false}; // track which walls have been opened
    int directions[] = {0, 1, 2, 3};              // directions: 0=N, 1=E, 2=S, 3=W
    for (int i = 0; i < 3; i++) {
        int dir_idx = rand_between(0, 4 - i); // pick random direction
        int dir = directions[dir_idx];
        directions[dir_idx] = directions[3 - i]; // remove this direction from pool so its not picked again

        // open a wall in the chosen direction
        if (dir == 0 && !opened[0]) { // north
            int col = rand_between(col_start, divide_col); //pick seg to open
            maze[divide_row - 1][col].bottom_wall = false; //remove the wall for the 2 bordering units
            maze[divide_row][col].top_wall = false; //       ^^
            opened[0] = true;
        } else if (dir == 1 && !opened[1]) { // east
            int row = rand_between(row_start, divide_row);
            maze[row][divide_col].left_wall = false;
            maze[row][divide_col - 1].right_wall = false;
            opened[1] = true;
        } else if (dir == 2 && !opened[2]) { // south
            int col = rand_between(divide_col, col_end + 1);
            maze[divide_row][col].top_wall = false;
            maze[divide_row - 1][col].bottom_wall = false;
            opened[2] = true;
        } else if (dir == 3 && !opened[3]) { // west
            int row = rand_between(divide_row, row_end + 1);
            maze[row][divide_col - 1].right_wall = false;
            maze[row][divide_col].left_wall = false;
            opened[3] = true;
        }
    }

    // recursive call for each chamber made from this division
    generate_maze(row_start, divide_row - 1, col_start, divide_col - 1); // top left
    generate_maze(row_start, divide_row - 1, divide_col, col_end);       // top right
    generate_maze(divide_row, row_end, col_start, divide_col - 1);       // bottom left
    generate_maze(divide_row, row_end, divide_col, col_end);             // bottom right
    
}

// bang. maze time
void make_maze(int rows, int cols) {
    allocate_maze(rows, cols);
    init_maze(rows, cols); 
    generate_maze(0, rows - 1, 0, cols - 1); 
}

//function to print a text version of the maze so i know its correct
void print_maze(int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("+");
            printf(maze[i][j].top_wall ? "---" : "   ");
        }
        printf("+\n");

        for (int j = 0; j < cols; j++) {
            printf(maze[i][j].left_wall ? "|" : " "); 
            printf("   ");
        }

        if (i == rows - 1 && cols > 0) {
            printf(maze[rows - 1][cols - 1].right_wall ? "|" : " ");
        } else {
            printf("|");
        }
        printf("\n");
    }
    
    for (int j = 0; j < cols; j++) {
        printf("+");
        printf(maze[rows - 1][j].bottom_wall ? "---" : "   ");
    }
    printf("+\n");
}

//make the maze flooring
void generate_maze_floor(int maze_x, int maze_z) {
    //plank texture for floor
    init_texture(1.0f, 0.75f);

    // calculate floor dimensions
    int floor_width = maze_x * 5 - (maze_x - 1);  // total blocks in x-axis
    int floor_depth = maze_z * 5 - (maze_z - 1);  // total blocks in z-axis

    // calculate total number of blocks and vertices
    int floor_blocks = floor_width * floor_depth;
    int floor_vertices = floor_blocks * num_vertices_per_block;

    // allocate memory for positions and texture coords
    vec4 *floor_positions = (vec4 *)malloc(sizeof(vec4) * floor_vertices);
    vec2 *floor_tex_coords = (vec2 *)malloc(sizeof(vec2) * floor_vertices);
    if (!floor_positions || !floor_tex_coords) {
        fprintf(stderr, "Failed to allocate memory for maze floor.\n");
        exit(EXIT_FAILURE);
    }

    int index = 0;
    float y_translation = scale_cube * 0.5f;
    float block_size = scale_cube * 0.5f;

    // calculate offsets to center the floor
    float x_offset = -(floor_width / 2.0f) * block_size + block_size / 2.0f;
    float z_offset = -(floor_depth / 2.0f) * block_size + block_size / 2.0f;

    // loop through the maze dimensions and create the floor
    for (int i = 0; i < floor_depth; ++i) {
        for (int j = 0; j < floor_width; ++j) {
            // calculate translation for the current block
            float x_translation = j * block_size + x_offset;
            float z_translation = i * block_size + z_offset;

            // then copy block positions and apply translation
            for (int k = 0; k < num_vertices_per_block; ++k) {
                floor_positions[index] = block_positions[k];
                floor_positions[index].x += x_translation;
                floor_positions[index].y += y_translation;
                floor_positions[index].z += z_translation;
                floor_tex_coords[index] = block_tex_coords[k];
                index++;
            }
        }
    }

    // update global positions and tex_coords with the new floor data
    num_vertices += floor_vertices;
    positions = (vec4 *)realloc(positions, sizeof(vec4) * num_vertices);
    tex_coords = (vec2 *)realloc(tex_coords, sizeof(vec2) * num_vertices);

    if (!positions || !tex_coords) {
        fprintf(stderr, "Failed to reallocate memory for maze floor vertices.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(positions + (num_vertices - floor_vertices), floor_positions, sizeof(vec4) * floor_vertices);
    memcpy(tex_coords + (num_vertices - floor_vertices), floor_tex_coords, sizeof(vec2) * floor_vertices);

    free(floor_positions);
    free(floor_tex_coords);
}

//make the verts or poles
void generate_maze_poles(int maze_x, int maze_z) {
    // cracked stone brick for poles
    init_texture(0.5f, 0.5f);

    // calculate amount of poles
    int num_poles_x = maze_x + 1;
    int num_poles_z = maze_z + 1;

    // pole block size and base height
    float block_size = scale_cube * 0.5f;
    float base_height = scale_cube;

    // calculate offsets to center the poles
    float x_offset = -(maze_x * 5 - (maze_x - 1)) / 2.0f * block_size + block_size / 2.0f;
    float z_offset = -(maze_z * 5 - (maze_z - 1)) / 2.0f * block_size + block_size / 2.0f;

    // temporary lists to store pole vertices and texture coordinates
    int max_pole_blocks = num_poles_x * num_poles_z * 5;  // max of 5 blocks per pole
    int max_pole_vertices = max_pole_blocks * num_vertices_per_block;

    vec4 *temp_positions = (vec4 *)malloc(sizeof(vec4) * max_pole_vertices);
    vec2 *temp_tex_coords = (vec2 *)malloc(sizeof(vec2) * max_pole_vertices);

    if (!temp_positions || !temp_tex_coords) {
        fprintf(stderr, "Failed to allocate memory for maze poles.\n");
        exit(EXIT_FAILURE);
    }

    int index = 0;

    // loop through poles along the grid
    for (int i = 0; i < num_poles_z; ++i) {
        for (int j = 0; j < num_poles_x; ++j) {
            // find base position of pole
            float x_pole = j * (5 * block_size - block_size) + x_offset;
            float z_pole = i * (5 * block_size - block_size) + z_offset;

            // randomize pole height between 3 and 5 blocks
            int pole_height = rand_between(3, 6);  // 6 is exclusive here

            // stack blocks vertically to build the pole
            for (int h = 0; h < pole_height; ++h) {
                // add a block at the current height
                for (int k = 0; k < num_vertices_per_block; ++k) {
                    temp_positions[index] = block_positions[k];
                    temp_positions[index].x += x_pole;
                    temp_positions[index].y += base_height + h * block_size;
                    temp_positions[index].z += z_pole;

                    temp_tex_coords[index] = block_tex_coords[k];
                    index++;
                }
            }
        }
    }

    // update global positions and tex_coords with the new pole data
    int pole_vertices = index;  // total vertices used for poles

    positions = (vec4 *)realloc(positions, sizeof(vec4) * (num_vertices + pole_vertices));
    tex_coords = (vec2 *)realloc(tex_coords, sizeof(vec2) * (num_vertices + pole_vertices));

    if (!positions || !tex_coords) {
        fprintf(stderr, "Failed to reallocate memory for maze pole vertices.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(positions + num_vertices, temp_positions, sizeof(vec4) * pole_vertices);
    memcpy(tex_coords + num_vertices, temp_tex_coords, sizeof(vec2) * pole_vertices);

    num_vertices += pole_vertices;  // update the total vertex count

    free(temp_positions);
    free(temp_tex_coords);
}

//make the walls according to the random maze generation
void generate_maze_walls(int maze_x, int maze_z) {
    // brick texture
    init_texture(1.0f, 0.50f);

    // block size and base height
    float block_size = scale_cube * 0.5f;
    float base_height = scale_cube;

    //adjust offsets to center the walls
    float x_offset = -((maze_x * 5 - (maze_x - 1)) / 2.0f) * block_size + 2.5 * block_size;
    float z_offset = -((maze_z * 5 - (maze_z - 1)) / 2.0f) * block_size + 2.5 * block_size;


    // temp arrays for wall vertices and text coords
    int max_wall_blocks = (maze_x * maze_z * 4) * 15;  // walls are made of 3 segments. segments have max height of 5
    int max_wall_vertices = max_wall_blocks * num_vertices_per_block;

    vec4 *temp_positions = (vec4 *)malloc(sizeof(vec4) * max_wall_vertices);
    vec2 *temp_tex_coords = (vec2 *)malloc(sizeof(vec2) * max_wall_vertices);

    if (!temp_positions || !temp_tex_coords) {
        fprintf(stderr, "Failed to allocate memory for maze walls.\n");
        exit(EXIT_FAILURE);
    }

    int index = 0;

    // loop through maze grid
    for (int i = 0; i < maze_z; ++i) {
        for (int j = 0; j < maze_x; ++j) {
            // find base position of current cell
            float x_cell = j * (5 * block_size - block_size) + x_offset;
            float z_cell = i * (5 * block_size - block_size) + z_offset;

            // add walls based on maze structure
            if (maze[i][j].top_wall) {
                for (int segment = 0; segment < 3; ++segment) { //3 segments per wall
                    int wall_height = rand_between(3, 6); // need random height for each segment
                    for (int h = 0; h < wall_height; ++h) {
                        for (int k = 0; k < num_vertices_per_block; ++k) {
                            temp_positions[index] = block_positions[k];
                            temp_positions[index].x += x_cell + (segment - 1) * block_size;  // offset for segment
                            temp_positions[index].y += base_height + h * block_size;
                            temp_positions[index].z += z_cell - 2.0f * block_size;  // corrected offset for top wall
                            temp_tex_coords[index] = block_tex_coords[k];
                            index++;
                        }
                    }
                }
            }

            if (maze[i][j].bottom_wall) {
                for (int segment = 0; segment < 3; ++segment) {
                    int wall_height = rand_between(3, 6);
                    for (int h = 0; h < wall_height; ++h) {
                        for (int k = 0; k < num_vertices_per_block; ++k) {
                            temp_positions[index] = block_positions[k];
                            temp_positions[index].x += x_cell + (segment - 1) * block_size;
                            temp_positions[index].y += base_height + h * block_size;
                            temp_positions[index].z += z_cell + 2.0f * block_size;  // corrected offset for bottom wall
                            temp_tex_coords[index] = block_tex_coords[k];
                            index++;
                        }
                    }
                }
            }

            if (maze[i][j].left_wall) {
                for (int segment = 0; segment < 3; ++segment) {
                    int wall_height = rand_between(3, 6);
                    for (int h = 0; h < wall_height; ++h) {
                        for (int k = 0; k < num_vertices_per_block; ++k) {
                            temp_positions[index] = block_positions[k];
                            temp_positions[index].x += x_cell - 2.0f * block_size;  //...for left wall
                            temp_positions[index].y += base_height + h * block_size;
                            temp_positions[index].z += z_cell + (segment - 1) * block_size;
                            temp_tex_coords[index] = block_tex_coords[k];
                            index++;
                        }
                    }
                }
            }

            if (maze[i][j].right_wall) {
                for (int segment = 0; segment < 3; ++segment) {
                    int wall_height = rand_between(3, 6);
                    for (int h = 0; h < wall_height; ++h) {
                        for (int k = 0; k < num_vertices_per_block; ++k) {
                            temp_positions[index] = block_positions[k];
                            temp_positions[index].x += x_cell + 2.0f * block_size;  //...for right wall
                            temp_positions[index].y += base_height + h * block_size;
                            temp_positions[index].z += z_cell + (segment - 1) * block_size;
                            temp_tex_coords[index] = block_tex_coords[k];
                            index++;
                        }
                    }
                }
            }
        }
    }

    // update global positions and tex_coords as usual
    int wall_vertices = index; 

    positions = (vec4 *)realloc(positions, sizeof(vec4) * (num_vertices + wall_vertices));
    tex_coords = (vec2 *)realloc(tex_coords, sizeof(vec2) * (num_vertices + wall_vertices));

    if (!positions || !tex_coords) {
        fprintf(stderr, "Failed to reallocate memory for maze wall vertices.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(positions + num_vertices, temp_positions, sizeof(vec4) * wall_vertices);
    memcpy(tex_coords + num_vertices, temp_tex_coords, sizeof(vec2) * wall_vertices);

    num_vertices += wall_vertices;  // update total vert count

    free(temp_positions);
    free(temp_tex_coords);
}

//function to bring all maze parts together and display them
void display_maze(int maze_x, int maze_z) {
    generate_maze_floor(maze_x, maze_z);
    generate_maze_poles(maze_x, maze_z);
    generate_maze_walls(maze_x, maze_z);
}

void init(void)
{
    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    init_grassblock();
    generate_pyramid(x_size, z_size);
    display_maze(maze_x_size, maze_z_size);
    display_sun();

    normals = (vec4 *) malloc(sizeof(vec4) * num_vertices);
    for(int i = 0; i < num_vertices; i += 36){
        for(int j = 0; j < 36; j++){
            if(j <= 5){
                normals[i + j] = (vec4) {0, 0, 1, 0};
            }
            else if(j > 5 && j <= 11){
                normals[i + j] = (vec4) {1, 0, 0, 0};
            }
            else if(j > 11 && j <= 17){
                normals[i + j] = (vec4) {-1, 0, 0, 0};
            }
            else if(j > 17 && j <= 23){
                normals[i + j] = (vec4) {0, 0, -1, 0};
            }
            else if(j > 23 && j <= 29){
                normals[i + j] = (vec4) {0, 1, 0, 0};
            }
            else{
                normals[i + j] = (vec4) {0, -1, 0, 0};
            }
        }
    }

    int tex_width = 64;
    int tex_height = 64;
    GLubyte my_texels[tex_width][tex_height][3];

    FILE *fp = fopen("textures02.raw", "r");
    if(fp != NULL)
    printf("[textureTemplate] Successfully open a texture file.\n");
    fread(my_texels, tex_width * tex_height * 3, 1, fp);
    fclose(fp);

    GLuint mytex[1];
    glGenTextures(1, mytex);
    glBindTexture(GL_TEXTURE_2D, mytex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, my_texels);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    int param;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &param);

    // Calculate sizes for positions and texture coordinates
    size_t size_positions = sizeof(vec4) * num_vertices; 
    size_t size_text_coords = sizeof(vec2) * num_vertices;

    GLuint vao;
    #ifdef __APPLE__
    glGenVertexArraysAPPLE(1, &vao);
    glBindVertexArrayAPPLE(vao);
    #else
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    #endif

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size_positions * 2 + size_text_coords, NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, size_positions, positions);
    glBufferSubData(GL_ARRAY_BUFFER, size_positions, size_positions, normals);
    glBufferSubData(GL_ARRAY_BUFFER, size_positions * 2, size_text_coords, tex_coords);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (0));

    GLuint vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (size_positions));

    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (2 * size_positions));

    GLuint texture_location = glGetUniformLocation(program, "texture");
    glUniform1i(texture_location, 0);

    model_view = look_at((vec4) {0, 0, maze_z_size * 3, 1}, (vec4) {0, 0, maze_z_size * 3 - 1, 1}, (vec4) {0, 1, 0, 0});
    projection = frustum(-1, 1, -1, 1, -1, -200);

    ctm_location = glGetUniformLocation(program, "ctm");
    model_view_location = glGetUniformLocation(program, "model_view");
    projection_location = glGetUniformLocation(program, "projection");

    light_position_location = glGetUniformLocation(program, "light_position");
    glUniform4fv(light_position_location, 1, (GLvoid *) &sun_position);

    enable_light_location = glGetUniformLocation(program, "enable_light");
    glUniform1i(enable_light_location, enable_light);
    ambient_light_location = glGetUniformLocation(program, "ambient_light");
    glUniform1i(ambient_light_location, ambient_light);
    diffuse_light_location = glGetUniformLocation(program, "diffuse_light");
    glUniform1i(diffuse_light_location, diffuse_light);
    specular_light_location = glGetUniformLocation(program, "specular_light");
    glUniform1i(specular_light_location, specular_light);
    no_light_location = glGetUniformLocation(program, "no_light");
    glUniform1i(no_light_location, no_light);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDepthRange(1,0);
}

void init_block(){
    if (block_positions == NULL) {
        block_positions = (vec4 *) malloc(sizeof(vec4) * num_vertices_per_block);
        if (block_positions == NULL) {
            fprintf(stderr, "Failed to allocate memory for block_positions.\n");
            exit(EXIT_FAILURE);
        }
    }
    
    if (block_tex_coords == NULL) {
        block_tex_coords = (vec2 *) malloc(sizeof(vec2) * num_vertices_per_block);
        if (block_tex_coords == NULL) {
            fprintf(stderr, "Failed to allocate memory for block_tex_coords.\n");
            exit(EXIT_FAILURE);
        }
    }

    block_positions[0] = (vec4) {.25, -.25,  0.0,  1.0}; 
    block_positions[1] = (vec4) {-0.25,  0.25,  0.0,  1.0}; 
    block_positions[2] = (vec4) {-0.25, -.25,  0.0,  1.0}; 
    block_positions[3] = (vec4) {.25, -.25,  0.0,  1.0}; 
    block_positions[4] = (vec4) { .25, 0.25,  0.0,  1.0}; 
    block_positions[5] = (vec4) {-0.25,  0.25,  0.0,  1.0}; 

    //ls 
    block_positions[6] = (vec4) {.25, -.25,  -.5,  1.0}; 
    block_positions[7] =  (vec4) {.25, .25,  0.0,  1.0}; 
    block_positions[8] =  (vec4) {.25, -.25,  0.0,  1.0}; 
    block_positions[9] = (vec4) {.25, -.25,  -.5,  1.0}; 
    block_positions[10] =  (vec4) {.25, .25,  -.5,  1.0}; 
    block_positions[11] =  (vec4) {.25, .25,  0.0,  1.0}; 

    //rs 
    block_positions[12] = (vec4) {-.25, -.25,  0,  1.0}; 
    block_positions[13] =  (vec4) {-.25, .25,  0.0,  1.0}; 
    block_positions[14] =  (vec4) {-.25, -.25,  -0.5,  1.0}; 
    block_positions[15] = (vec4) {-.25, .25,  0,  1.0}; 
    block_positions[16] =  (vec4) {-.25, .25,  -.5,  1.0}; 
    block_positions[17] =  (vec4) {-.25, -.25,  -0.5,  1.0}; 

    //bs 
    block_positions[18] = (vec4) {-.25, -.25,  -.5,  1.0}; 
    block_positions[19] =  (vec4) {-.25, .25,  -.5,  1.0}; 
    block_positions[20] =  (vec4) {.25, -.25,  -0.5,  1.0}; 
    block_positions[21] = (vec4) {-.25, .25,  -.5,  1.0}; 
    block_positions[22] =  (vec4) {.25, .25,  -.5,  1.0}; 
    block_positions[23] =  (vec4) {.25, -.25,  -.5,  1.0}; 

    //top
    block_positions[24] = (vec4) {.25, .25,  0,  1.0}; 
    block_positions[25] =  (vec4) {.25, .25,  -.5,  1.0}; 
    block_positions[26] =  (vec4) {-.25, .25,  -0,  1.0}; 
    block_positions[27] = (vec4) {.25, .25,  -.5,  1.0}; 
    block_positions[28] =  (vec4) {-.25, .25,  -.5,  1.0}; 
    block_positions[29] =  (vec4) {-.25, .25,  0,  1.0}; 

    //bottom
    block_positions[30] = (vec4) {.25, -.25,  -.5,  1.0}; 
    block_positions[31] =  (vec4) {.25, -.25,  0,  1.0}; 
    block_positions[32] =  (vec4) {-.25, -.25,  -0.5,  1.0}; 
    block_positions[33] = (vec4) {.25, -.25,  -0,  1.0}; 
    block_positions[34] =  (vec4) {-.25, -.25,  -0,  1.0}; 
    block_positions[35] =  (vec4) {-.25, -.25,  -.5,  1.0}; 

    for (int i = 0; i < num_vertices_per_block; ++i) {
        block_positions[i].x *= scale_cube;
        block_positions[i].y *= scale_cube;
        block_positions[i].z *= scale_cube;
    }
}

void init_grassblock() {
    init_block(); 

    //textures for grassblock
    block_tex_coords[0] = (vec2) {0.75, 1.00};
    block_tex_coords[1] = (vec2) {0.50, 0.75};
    block_tex_coords[2] = (vec2) {0.50, 1.00};
    block_tex_coords[3] = (vec2) {0.75, 1.00};
    block_tex_coords[4] = (vec2) {0.75, 0.75};
    block_tex_coords[5] = (vec2) {0.50, 0.75};
    
    block_tex_coords[6] = (vec2) {0.75, 1.00};
    block_tex_coords[7] = (vec2) {0.50, 0.75};
    block_tex_coords[8] = (vec2) {0.50, 1.00};
    block_tex_coords[9] = (vec2) {0.75, 1.00};
    block_tex_coords[10] = (vec2) {0.75, 0.75};
    block_tex_coords[11] = (vec2) {0.50, 0.75};

    block_tex_coords[12] = (vec2) {0.75, 1.00};
    block_tex_coords[13] = (vec2) {0.75, 0.75};
    block_tex_coords[14] = (vec2) {0.50, 1.00};
    block_tex_coords[15] = (vec2) {0.75, .75};
    block_tex_coords[16] = (vec2) {0.50, 0.75};
    block_tex_coords[17] = (vec2) {0.50, 1.00};

    block_tex_coords[18] = (vec2) {0.75, 1.00};
    block_tex_coords[19] = (vec2) {0.75, 0.75};
    block_tex_coords[20] = (vec2) {0.50, 1.00};
    block_tex_coords[21] = (vec2) {0.75, .75};
    block_tex_coords[22] = (vec2) {0.50, 0.75};
    block_tex_coords[23] = (vec2) {0.50, 1.00};

    block_tex_coords[24] = (vec2) {0.25, .25};
    block_tex_coords[25] = (vec2) {0.25, 0};
    block_tex_coords[26] = (vec2) {0, .25};
    block_tex_coords[27] = (vec2) {0.25, 0};
    block_tex_coords[28] = (vec2) {0, 0};
    block_tex_coords[29] = (vec2) {0, .25};

    block_tex_coords[30] = (vec2) {1, 1};
    block_tex_coords[31] = (vec2) {1,  0.75};
    block_tex_coords[32] = (vec2) {.75, 1};
    block_tex_coords[33] = (vec2) {1, .75};
    block_tex_coords[34] = (vec2) {.75, .75};
    block_tex_coords[35] = (vec2) {0.75, 1};
}

void init_texture(float rcornerX, float rcornerY){
    //front
    block_tex_coords[0] = (vec2) {rcornerX, rcornerY};
    block_tex_coords[1] = (vec2) {rcornerX - .25, rcornerY - .25};
    block_tex_coords[2] = (vec2) {rcornerX - .25, rcornerY};
    block_tex_coords[3] = (vec2) {rcornerX, rcornerY};
    block_tex_coords[4] = (vec2) {rcornerX, rcornerY - .25};
    block_tex_coords[5] = (vec2) {rcornerX - .25, rcornerY - .25};

    //ls
    block_tex_coords[6] = (vec2) {rcornerX, rcornerY};
    block_tex_coords[7] = (vec2) {rcornerX - .25, rcornerY - .25};
    block_tex_coords[8] = (vec2) {rcornerX - .25, rcornerY};
    block_tex_coords[9] = (vec2) {rcornerX, rcornerY};
    block_tex_coords[10] = (vec2) {rcornerX, rcornerY - .25};
    block_tex_coords[11] = (vec2) {rcornerX - .25, rcornerY - .25};

    //rs
    block_tex_coords[12] = (vec2) {rcornerX, rcornerY};
    block_tex_coords[13] = (vec2) {rcornerX, rcornerY - .25};
    block_tex_coords[14] = (vec2) {rcornerX - .25, rcornerY};
    block_tex_coords[15] = (vec2) {rcornerX, rcornerY - .25};
    block_tex_coords[16] = (vec2) {rcornerX - .25, rcornerY - .25};
    block_tex_coords[17] = (vec2) {rcornerX - .25, rcornerY};

    //bs
    block_tex_coords[18] = (vec2) {rcornerX, rcornerY};
    block_tex_coords[19] = (vec2) {rcornerX, rcornerY - .25};
    block_tex_coords[20] = (vec2) {rcornerX - .25, rcornerY};
    block_tex_coords[21] = (vec2) {rcornerX, rcornerY - .25};
    block_tex_coords[22] = (vec2) {rcornerX - .25, rcornerY - .25};
    block_tex_coords[23] = (vec2) {rcornerX - .25, rcornerY};

    //top
    block_tex_coords[24] = (vec2) {rcornerX, rcornerY};
    block_tex_coords[25] = (vec2) {rcornerX, rcornerY - .25};
    block_tex_coords[26] = (vec2) {rcornerX - .25, rcornerY};
    block_tex_coords[27] = (vec2) {rcornerX, rcornerY - .25};
    block_tex_coords[28] = (vec2) {rcornerX - .25, rcornerY - .25};
    block_tex_coords[29] = (vec2) {rcornerX - .25, rcornerY};

    //bottom
    block_tex_coords[30] = (vec2) {rcornerX, rcornerY};
    block_tex_coords[31] = (vec2) {rcornerX, rcornerY - .25};
    block_tex_coords[32] = (vec2) {rcornerX - .25, rcornerY};
    block_tex_coords[33] = (vec2) {rcornerX, rcornerY - .25};
    block_tex_coords[34] = (vec2) {rcornerX - .25, rcornerY - .25};
    block_tex_coords[35] = (vec2) {rcornerX - .25, rcornerY};
}

void generate_pyramid(int x_size, int z_size) {
    int layers_x = x_size;
    int layers_z = z_size;
    int layers = (layers_x < layers_z) ? layers_x : layers_z;

    // Temporary lists to hold positions and texture coordinates
    vec4 *temp_positions = NULL;
    vec2 *temp_tex_coords = NULL;
    int max_blocks = layers_x * layers_z * layers; // Maximum possible blocks
    int allocated_vertices = max_blocks * num_vertices_per_block;

    // Initially allocate memory for the maximum possible size
    temp_positions = (vec4 *)malloc(sizeof(vec4) * allocated_vertices);
    temp_tex_coords = (vec2 *)malloc(sizeof(vec2) * allocated_vertices);

    int index = 0;
    float y_translation = 0.0f;

    float block_size_x = 0.5f * scale_cube;
    float block_size_z = 0.5f * scale_cube;

    for (int layer = 0; layer < layers; ++layer) {
        int blocks_per_layer_x = layers_x - layer * 2;
        int blocks_per_layer_z = layers_z - layer * 2;

        if (blocks_per_layer_x <= 0 || blocks_per_layer_z <= 0) {
            break;
        }

        // Count blocks only for the first layer
        if (layer == 0) {
            printf("First layer - Blocks in X: %d, Blocks in Z: %d\n", blocks_per_layer_x, blocks_per_layer_z);
        }

        float x_start = -((blocks_per_layer_x - 1) / 2.0f) * block_size_x;
        float z_start = -((blocks_per_layer_z - 1) / 2.0f) * block_size_z;

        for (int i = 0; i < blocks_per_layer_x; ++i) {
            for (int j = 0; j < blocks_per_layer_z; ++j) {
                bool is_edge_block = (i == 0 || i == blocks_per_layer_x - 1 || j == 0 || j == blocks_per_layer_z - 1);

                if (layer == 0 && is_edge_block && (rand() % 2 == 0)) {
                    // 50% chance to exclude edge block on layer 0
                    continue; // Skip this block
                }

                // For layers > 0, randomly decide whether to include the block
                if (layer > 0 && (rand() % 100 < 53)) {
                    // 53% chance to exclude block
                    continue; // Skip this block
                }

                // Ensure we have enough space
                if (index + num_vertices_per_block > allocated_vertices) {
                    // Reallocate more memory
                    allocated_vertices *= 2;
                    temp_positions = (vec4 *)realloc(temp_positions, sizeof(vec4) * allocated_vertices);
                    temp_tex_coords = (vec2 *)realloc(temp_tex_coords, sizeof(vec2) * allocated_vertices);
                    if (!temp_positions || !temp_tex_coords) {
                        fprintf(stderr, "Memory reallocation failed\n");
                        exit(EXIT_FAILURE);
                    }
                }

                // Calculate translation for the current block
                float x_translation = x_start + i * block_size_x;
                float z_translation = z_start + j * block_size_z;

                // Copy block positions and apply translation
                for (int k = 0; k < num_vertices_per_block; ++k) {
                    temp_positions[index] = block_positions[k];
                    temp_positions[index].x += x_translation;
                    temp_positions[index].y += y_translation;
                    temp_positions[index].z += z_translation;
                    temp_tex_coords[index] = block_tex_coords[k];
                    index++;
                }
                num_blocks++;
            }
        }

        // Call init_texture(1,1) after completing layer 0
        if (layer == 0) {
            init_texture(1, 1);
        }
        y_translation -= block_size_x; // Move down for the next layer
    }

    num_vertices = index; // Total vertices used

    // Allocate positions and tex_coords to the exact size needed
    positions = (vec4 *)malloc(sizeof(vec4) * num_vertices);
    tex_coords = (vec2 *)malloc(sizeof(vec2) * num_vertices);
    if (!positions || !tex_coords) {
        fprintf(stderr, "Final memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    // Copy the data from temporary arrays to the actual arrays
    memcpy(positions, temp_positions, sizeof(vec4) * num_vertices);
    memcpy(tex_coords, temp_tex_coords, sizeof(vec2) * num_vertices);

    // Free the temporary arrays
    free(temp_positions);
    free(temp_tex_coords);
}

void display_sun() {
    init_block(); // Initialize block geometry for the sun

    // Set the texture coordinates for the sun
    init_texture(0.75f, .75f);

    // Keep track of the starting index of the sun's vertices
    int sun_start_index = num_vertices;

    // Reallocate positions and tex_coords arrays to hold the additional sun vertices
    int new_num_vertices = num_vertices + num_vertices_per_block;
    positions = (vec4 *)realloc(positions, sizeof(vec4) * new_num_vertices);
    tex_coords = (vec2 *)realloc(tex_coords, sizeof(vec2) * new_num_vertices);

    if (!positions || !tex_coords) {
        fprintf(stderr, "Memory allocation failed when adding sun vertices\n");
        exit(EXIT_FAILURE);
    }

    // Copy the sun's block_positions and block_tex_coords to the end of positions and tex_coords
    memcpy(positions + num_vertices, block_positions, sizeof(vec4) * num_vertices_per_block);
    memcpy(tex_coords + num_vertices, block_tex_coords, sizeof(vec2) * num_vertices_per_block);

    num_vertices = new_num_vertices; // Update total number of vertices

    // Store the sun's start index and vertex count for later use
    num_vertices_sun = num_vertices_per_block;
    sun_positions = positions + sun_start_index;
    sun_tex_coords = tex_coords + sun_start_index;

    // Since we will always have sun's location, no need to do the thing with sending the ctm through the pipeline twice.
    // When we rotate the sun, just multiply each vertex by rotation matrix
    
}

void idle() {
    if(is_animating){
        if(reset_animation){

        }
        else if (forward_animation) {
            if (num_steps > max_steps) {
                // end animation
                is_animating = 0;
                forward_animation = 0;

                // finalize values
                eye_x = target_eye_x;
                eye_z = target_eye_z;
                at_x = target_at_x;
                at_z = target_at_z;

                num_steps = 0;
            } else {
                //tried to replicate what was described on the project page
                float alpha = (float)num_steps / max_steps;

                float new_eye_x = (1 - alpha) * eye_x + alpha * target_eye_x;
                float new_eye_z = (1 - alpha) * eye_z + alpha * target_eye_z;
                float new_at_x = (1 - alpha) * at_x + alpha * target_at_x;
                float new_at_z = (1 - alpha) * at_z + alpha * target_at_z;

                // update model_view matrix
                model_view = look_at((vec4){new_eye_x, 2, new_eye_z, 1}, 
                                     (vec4){new_at_x, 2, new_at_z, 1}, 
                                     (vec4){0, 1, 0, 0});

                num_steps++;
            }
        }
        else if (backward_animation) {
            if (num_steps > max_steps) {
                // end animation
                is_animating = 0;
                backward_animation = 0;

                // finalize values
                eye_x = target_eye_x;
                eye_z = target_eye_z;
                at_x = target_at_x;
                at_z = target_at_z;

                num_steps = 0;
            } else {
                float alpha = (float)num_steps / max_steps;

                float new_eye_x = (1 - alpha) * eye_x + alpha * target_eye_x;
                float new_eye_z = (1 - alpha) * eye_z + alpha * target_eye_z;
                float new_at_x = (1 - alpha) * at_x + alpha * target_at_x;
                float new_at_z = (1 - alpha) * at_z + alpha * target_at_z;

                model_view = look_at((vec4){new_eye_x, 2, new_eye_z, 1}, 
                                     (vec4){new_at_x, 2, new_at_z, 1}, 
                                     (vec4){0, 1, 0, 0});

                num_steps++;
            }
        }
        else if (slide_left_animation) {
            if (num_steps > max_steps) {
                // end animation
                is_animating = 0;
                slide_left_animation = 0;

                // finalize values
                eye_x = target_eye_x;
                eye_z = target_eye_z;
                at_x = target_at_x;
                at_z = target_at_z;

                num_steps = 0;
            } else {
                float alpha = (float)num_steps / max_steps;

                float new_eye_x = (1 - alpha) * eye_x + alpha * target_eye_x;
                float new_eye_z = (1 - alpha) * eye_z + alpha * target_eye_z;
                float new_at_x = (1 - alpha) * at_x + alpha * target_at_x;
                float new_at_z = (1 - alpha) * at_z + alpha * target_at_z;

                model_view = look_at((vec4){new_eye_x, 2, new_eye_z, 1}, 
                                     (vec4){new_at_x, 2, new_at_z, 1}, 
                                     (vec4){0, 1, 0, 0});

                num_steps++;
            }
        }
        else if (slide_right_animation) {
            if (num_steps > max_steps) {
                // end animation
                is_animating = 0;
                slide_right_animation = 0;

                // finalize values
                eye_x = target_eye_x;
                eye_z = target_eye_z;
                at_x = target_at_x;
                at_z = target_at_z;

                num_steps = 0;
            } else {
                float alpha = (float)num_steps / max_steps;

                float new_eye_x = (1 - alpha) * eye_x + alpha * target_eye_x;
                float new_eye_z = (1 - alpha) * eye_z + alpha * target_eye_z;
                float new_at_x = (1 - alpha) * at_x + alpha * target_at_x;
                float new_at_z = (1 - alpha) * at_z + alpha * target_at_z;

                model_view = look_at((vec4){new_eye_x, 2, new_eye_z, 1}, 
                                     (vec4){new_at_x, 2, new_at_z, 1}, 
                                     (vec4){0, 1, 0, 0});

                num_steps++;
            }
        }       
        else if(look_left_animation){
            
        }
        else if(look_right_animation){
            
        }
    }
    if (resetting && reset_step < reset_steps) {
        // Incrementally reduce prev_ctm by delta_ctm to approach the identity matrix
        prev_ctm = mm_subtraction(prev_ctm, delta_ctm);

        // Update the current transformation matrix (ctm) with the modified prev_ctm
        ctm = prev_ctm;

        // Increment the step counter to track progress of the reset animation
        ++reset_step;
    } else {
        // Reset process is complete
        resetting = false;
    }
    glutPostRedisplay();
}

void resetPlatform() {
    // Check if a reset is already in progress
    if (!resetting) {
        // Begin the reset process
        resetting = true;

        // Initialize the step counter
        reset_step = 0;

        mat4 current_ctm = ctm;

        // Calculate the difference between the current_ctm and the identity matrix
        mat4 identity = identity_matrix();
        mat4 diff = mm_subtraction(current_ctm, identity);

        // Compute the incremental change (delta_ctm) by dividing the difference by the total steps
        for (int i = 0; i < 4; ++i) {
            delta_ctm.x.x = diff.x.x / reset_steps;
            delta_ctm.x.y = diff.x.y / reset_steps;
            delta_ctm.x.z = diff.x.z / reset_steps;
            delta_ctm.x.w = diff.x.w / reset_steps;

            delta_ctm.y.x = diff.y.x / reset_steps;
            delta_ctm.y.y = diff.y.y / reset_steps;
            delta_ctm.y.z = diff.y.z / reset_steps;
            delta_ctm.y.w = diff.y.w / reset_steps;

            delta_ctm.z.x = diff.z.x / reset_steps;
            delta_ctm.z.y = diff.z.y / reset_steps;
            delta_ctm.z.z = diff.z.z / reset_steps;
            delta_ctm.z.w = diff.z.w / reset_steps;

            delta_ctm.w.x = diff.w.x / reset_steps;
            delta_ctm.w.y = diff.w.y / reset_steps;
            delta_ctm.w.z = diff.w.z / reset_steps;
            delta_ctm.w.w = diff.w.w / reset_steps;
        }

        // Store the current_ctm snapshot for use in the reset animation
        prev_ctm = current_ctm;

        // Register the idle function to handle the reset animation
        //glutIdleFunc(idle);
    }
}
void solve_maze_lh() {
    // Perform one step of the left-hand rule
    if (!inside_maze) {
        if (direction != 0) {
            turn_left();
        } else {
            forward();
        }
        return;
    }

    if (!(player_row == 0 && player_col == maze_x_size - 1)) {
        int left_direction = (direction + 3) % 4;

        if (can_move_inside_maze(player_row, player_col, left_direction)) {
            turn_left();
            forward();
        } else if (can_move_inside_maze(player_row, player_col, direction)) {
            forward();
        } else {
            turn_right();
            if (!can_move_inside_maze(player_row, player_col, direction)) {
                turn_right();
                forward();
            }
        }

        glutPostRedisplay();
        return;
    }

    if (direction != 0) {
        turn_left();
    } else {
        forward();
    }

    glutPostRedisplay();
}


void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the scene
    //mat4 scaling_matrix = scale(scale_factor, scale_factor, scale_factor);
    //mat4 final_ctm = mm_multiplication(scaling_matrix, ctm);
    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *)&ctm);
    //print_matrix(ctm);

    // Draw all vertices except the sun's vertices
    glDrawArrays(GL_TRIANGLES, 0, num_vertices - num_vertices_sun);

    // Draw the sun
    float sun_scale = 3.0f; // The size of the sun
    mat4 sun_scaling = scale(sun_scale, sun_scale, sun_scale);
    mat4 sun_translation = translate(sun_position.x, sun_position.y, sun_position.z);
    mat4 sun_ctm = mm_multiplication(sun_translation, sun_scaling);

    // Apply any global scaling if necessary
    mat4 sun_final_ctm = mm_multiplication(ctm, sun_ctm);

    // Send the sun's ctm to the shader
    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *)&sun_final_ctm);

    // Send the updated model_view matrix to the shader
    
    //model_view = look_at((vec4) {0, 0, 5.5, 1}, (vec4) {0, 0, -1, 1}, (vec4) {0, 1, 0, 0});

    //model_view = look_at((vec4) {0, 0, maze_z_size * 3, 1}, (vec4) {0, 0, maze_z_size * 3 - 1, 1}, (vec4) {0, 1, 0, 0});
    //projection = ortho(-(1.75 * maze_z_size), (1.75 * maze_x_size), -(1.75 * maze_z_size), (1.75 * maze_z_size), -1, -100);
        

    glUniformMatrix4fv(model_view_location, 1, GL_FALSE, (GLfloat *) &model_view);

    // Send the updated projection matrix to the shader
    //projection = frustum(-10, 10, -5, 5, -1, -10);
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, (GLfloat *) &projection);

    glUniform4fv(light_position_location, 1, (GLvoid *) &sun_position);

    // Draw only the sun's vertices
    glDrawArrays(GL_TRIANGLES, num_vertices - num_vertices_sun, num_vertices_sun);

    //glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int mousex, int mousey) {
    float rotation_angle = 20.0f; // Rotation angle in degrees
    if (key == 'o') { // zoom out
        lrbt += 0.1;
        projection = frustum(-lrbt, lrbt, -lrbt, lrbt, -1, -200);
        glutPostRedisplay();
    }
    else if (key == 'i') { // zoom in
        lrbt -= 0.1;
        projection = frustum(-lrbt, lrbt, -lrbt, lrbt, -1, -200);
        glutPostRedisplay();
    }
    else if (key == 'q') {
        #ifndef __APPLE__
        glutLeaveMainLoop();
        #else
        exit(0);
        #endif
    }
    // Rotate sun around the axes using rotation matrices
    else if (key == '1') { // Rotate sun around X-axis (positive angle)
        mat4 rotation = rotate_x(rotation_angle);
        sun_position = mv_multiplication(rotation, sun_position);
        glutPostRedisplay();
    }
    else if (key == '2') { // Rotate sun around X-axis (negative angle)
        mat4 rotation = rotate_x(-rotation_angle);
        sun_position = mv_multiplication(rotation, sun_position);
        glutPostRedisplay();
    }
    else if (key == '3') { // Rotate sun around Y-axis (positive angle)
        mat4 rotation = rotate_y(rotation_angle);
        sun_position = mv_multiplication(rotation, sun_position);
        glutPostRedisplay();
    }
    else if (key == '4') { // Rotate sun around Y-axis (negative angle)
        mat4 rotation = rotate_y(-rotation_angle);
        sun_position = mv_multiplication(rotation, sun_position);
        glutPostRedisplay();
    }
    else if (key == '5') { // Rotate sun around Z-axis (positive angle)
        mat4 rotation = rotate_z(rotation_angle);
        sun_position = mv_multiplication(rotation, sun_position);
        glutPostRedisplay();
    }
    else if (key == '6') { // Rotate sun around Z-axis (negative angle)
        mat4 rotation = rotate_z(-rotation_angle);
        sun_position = mv_multiplication(rotation, sun_position);
        glutPostRedisplay();
    }
    else if (key == 'j') { // testing button
        //model_view = look_at((vec4) {0, 0, maze_z_size * 3, 1}, (vec4) {0, 0, maze_z_size * 3 - 1, 1}, (vec4) {0, 1, 0, 0});
        model_view = look_at((vec4) {0, 0, maze_x_size * 5 - (maze_x_size - 1), 1}, (vec4) {0, 0, maze_z_size * 3 - 1, 1}, (vec4) {0, 1, 0, 0});
        projection = ortho(-maze_x_size, maze_x_size, -maze_x_size, maze_x_size, -1, -100);
        //projection = frustum(-1, 1, -1, 1, -1, -100);
        //projection = ortho(-6, 6, -6, 6, -1, -100);
        
        glutPostRedisplay();
    }
    else if (key == 'f') { // Go to start of maze
        eye_x = -maze_x_size + 1;
        at_x = -maze_x_size + 1;
        eye_z = maze_z_size + 1;
        at_z = maze_z_size;

        // quinn code: init player position tracking outside maze 
        player_row = maze_z_size - 1; // entrance row
        player_col = 0;               // entrance column
        direction = 0;                // start facing snorth
        inside_maze = false;          // player is outside
        exit_direction = 2;           // bc we outside the entrance, exit direction is south

        projection = frustum(-.75, .75, -.75, .75, -1, -200); 
        model_view = look_at((vec4) {eye_x, 2, eye_z, 1}, (vec4) {at_x, 2, at_z, 1}, (vec4) {0, 1, 0, 0});
        glutPostRedisplay();
        print_location();
    }
    else if (key == 'e') {
        turn_left();
    }
    else if (key == 'r') {
        turn_right();
    }
    else if (key == 'z') {
        shortest_path(player_row, player_col, direction, inside_maze);
    }
    else if (key == ' ') { // Reset platform
        resetPlatform();
    }
    else if (key == 'k') {
        solve_maze_lh(); // Solve the maze via left-hand rule
    }
    else if (key == 'l') {
        if(enable_light == 0) enable_light = 1;
        else enable_light = 0;
        glUniform1i(enable_light_location, enable_light);
        glutPostRedisplay();
    }
    else if (key == '7') {
        if(ambient_light == 0) ambient_light = 1;
        else ambient_light = 0;
        glUniform1i(ambient_light_location, ambient_light);
        glutPostRedisplay();
    }
    else if (key == '8') {
        if(diffuse_light == 0) diffuse_light = 1;
        else diffuse_light = 0;
        glUniform1i(diffuse_light_location, diffuse_light);
        glutPostRedisplay();
    }
    else if (key == '9') {
        if(specular_light == 0) specular_light = 1;
        else specular_light = 0;
        glUniform1i(specular_light_location, specular_light);
        glutPostRedisplay();
    }
    else if (key == '0') {
        ambient_light = 0;
        diffuse_light = 0;
        specular_light = 0;
        glUniform1i(ambient_light_location, ambient_light);
        glUniform1i(diffuse_light_location, diffuse_light);
        glUniform1i(specular_light_location, specular_light);
        glutPostRedisplay();
    }
} 

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            dragging = 1;
            has_moved = 0;
            initial_point = map_coords(x, y);
            previous_point = initial_point;
            prev_ctm = ctm; // Store the current transformation matrix
        } else if (state == GLUT_UP) {
            dragging = 0;
        }
    }
}

void print_location(){
    printf("\nX: %f\n", eye_x);
    printf("Y: 2\n");
    printf("Z: %f\n", eye_z);
    printf("\n--------------------------\n");
}

vec4 map_coords(int x, int y){
    float x_coordinate = (x * 2.0 / 512.0) - 1.0;
    float y_coordinate = 1.0 - (y * 2.0 / 512.0);
    float z_coordinate = sqrt(1.0f - x_coordinate * x_coordinate - y_coordinate * y_coordinate);
    return (vec4){x_coordinate, y_coordinate, z_coordinate, 1};
}

void motion(int x, int y) {
    if (dragging) {
        has_moved = 1;
        // Map the new mouse position
        final_point = map_coords(x, y);

        // Check for NaN in initial_point or final_point
        if (isnan(initial_point.x) || isnan(initial_point.y) || isnan(initial_point.z) ||
            isnan(final_point.x) || isnan(final_point.y) || isnan(final_point.z)) {
            return;
        }
        //create_rotation();
        vec4 u = {initial_point.x, initial_point.y, initial_point.z, 0.0f};
        vec4 v = {final_point.x, final_point.y, final_point.z, 0.0f};

        normalize(u);
        normalize(v);

        // Calculate dot product and clamp between -1 and 1
        float dt = dot_product(u, v);
        float theta_z = acos(dt) * 180.0f / M_PI;

        // Calculate the rotation axis
        vec4 rotation_axis = cross_product(u, v);
        rotation_axis = normalize(rotation_axis);

        float distance = sqrt(rotation_axis.y * rotation_axis.y + rotation_axis.z * rotation_axis.z);
        if(distance != 0){
            float sin_theta_x = rotation_axis.y / distance;
            float cos_theta_x = rotation_axis.z / distance;

            mat4 pos_rx = {{1, 0, 0, 0}, {0, cos_theta_x, sin_theta_x, 0}, {0, -sin_theta_x, cos_theta_x, 0}, {0, 0, 0, 1}};
            mat4 neg_rx = m_transpose(pos_rx);

            float sin_theta_y = rotation_axis.x;
            float cos_theta_y = distance;

            mat4 neg_ry = {{cos_theta_y, 0, sin_theta_y, 0}, {0, 1, 0, 0}, {-sin_theta_y, 0, cos_theta_y, 0}, {0, 0, 0, 1}};
            mat4 pos_ry = m_transpose(neg_ry);

            // Final rotation matrix calculation
            mat4 m1 = mm_multiplication(neg_ry, pos_rx);
            mat4 m2 = mm_multiplication(rotate_z(theta_z), m1);
            mat4 m3 = mm_multiplication(pos_ry, m2);
            mat4 m4 = mm_multiplication(neg_rx, m3);
            
            rotation_matrix = m4;
            // Update the current transformation matrix by applying the incremental rotation
            //print_matrix(rotation_matrix);
            ctm = mm_multiplication(rotation_matrix, prev_ctm);

            // Update previous_point for the next motion event
            previous_point = final_point;
        }

        
    }
    glutPostRedisplay();
}

void special(int key, int x, int y) {
    if(key == GLUT_KEY_UP){
        forward();
        print_location();
    }
    else if(key == GLUT_KEY_DOWN){
        backward();
        print_location();
    }
    else if(key == GLUT_KEY_LEFT){
        slide_left();
        print_location();
    }
    else if(key == GLUT_KEY_RIGHT){
        slide_right();
        print_location();
    }

}

//shortest path alg using breadth-first search algorithm
void shortest_path(int player_row, int player_col, int direction, bool inside_maze) {
    //to start, handle case when player starts outside the map
    if (!inside_maze) {
        // if outside, turn to face north and move forward into the maze
        while (direction != 0) {
            turn_left();
            direction = (direction + 3) % 4; // update direction counterclockwise bc we arent updating global variables in this
        }
        forward();
    }

    // BFS setup
    int queue_size = maze_z_size * maze_x_size;
    Node queue[queue_size]; // stores nodes to visit next
    int front = 0, back = 0; //track front and end of queue

    // visited grid and parent tracking
    bool visited[maze_z_size][maze_x_size]; //keeps track of whether a cell has already been processed to avoid revisiting
    memset(visited, false, sizeof(visited));

    Node parents[maze_z_size][maze_x_size]; //tracks parent of each cell, this is used to reconstruct the path later

    // add starting node to the queue
    queue[back++] = (Node){player_row, player_col, -1, -1}; //add player curr position as first node in queue
    visited[player_row][player_col] = true; //then mark it as visited

    Node exit_node = {-1, -1, -1, -1};
    bool found_exit = false;

    // BFS loop
    while (front < back) {
        Node current = queue[front++];
        int row = current.row;
        int col = current.col;

        // check if we at the exit
        if (row == 0 && col == maze_x_size - 1) { // <-- exit position
            exit_node = current;
            found_exit = true;
            break;
        }

        // not at exit, explore the 4 neighboring units
        for (int dir = 0; dir < 4; dir++) {
            int new_row = row + d_row[dir]; //the d_row and d_col offset arrays find the correct neighboring cells based on the direction we are looking at
            int new_col = col + d_col[dir];

            // check that: its within bounds, it hasnt been visited, and its not blocked by a wall in the given direction
            if (new_row >= 0 && new_row < maze_z_size && new_col >= 0 && new_col < maze_x_size &&
                !visited[new_row][new_col] &&
                ((dir == 0 && !maze[row][col].top_wall) ||    // north wall
                 (dir == 1 && !maze[row][col].right_wall) ||  // east
                 (dir == 2 && !maze[row][col].bottom_wall) || // south
                 (dir == 3 && !maze[row][col].left_wall))) {  // west
                //this is the biggest if statement condition ive ever made haha
                
                //add the valid neighbors to the queue and mark them as visited
                //then update parents array to track how each cell was reached
                queue[back++] = (Node){new_row, new_col, row, col};
                visited[new_row][new_col] = true;
                parents[new_row][new_col] = current;
            }
        }
    }
    //edge case for if the BFS couldnt find the end, this should never happen
    if (!found_exit) {
        fprintf(stderr, "No path to the exit found!\n");
        return;
    }

    // reconstruct the path from exit to start
    Node path[queue_size];
    int path_length = 0;

    //start from exit node, use parents array to trace back to start, store each node in the path array
    for (Node current = exit_node; current.parent_row != -1; current = parents[current.row][current.col]) {
        path[path_length++] = current;
    }

    // reverse the path for navigation
    for (int i = 0; i < path_length / 2; i++) {
        Node temp = path[i];
        path[i] = path[path_length - 1 - i];
        path[path_length - 1 - i] = temp;
    }

    // now navigate thru the path
    for (int i = 0; i < path_length; i++) {
        Node next = path[i];
        int target_row = next.row;
        int target_col = next.col;

        // find the direction needed to face the target
        int target_direction = -1;
        if (target_row < player_row) target_direction = 0; // north
        else if (target_col > player_col) target_direction = 1; // east
        else if (target_row > player_row) target_direction = 2; // south
        else if (target_col < player_col) target_direction = 3; // west

        // then rotate to face the target direction
        while (direction != target_direction) {
            turn_right();
            direction = (direction + 1) % 4; // update direction 
        }

        // once facing traget, move forward into target cell
        forward();

        // then update the player's current position
        player_row = target_row;
        player_col = target_col;
    }

    // the above function should bring us to the exit, but we still have to walk thru the exit
    //if we arent already facing the exit, turn until we are
    while (direction != 0) {
        turn_left();
        direction = (direction + 3) % 4; // update direction
    }

    // once facing the exit, move forward and we are done
    forward();
}

//collision detection
bool can_move_inside_maze(int row, int col, int direction) {
    if (direction == 0 && maze[row][col].top_wall) return false;    // north
    if (direction == 1 && maze[row][col].right_wall) return false; // east
    if (direction == 2 && maze[row][col].bottom_wall) return false; // south
    if (direction == 3 && maze[row][col].left_wall) return false;  // west
    return true;
}

bool can_reenter_maze(int exit_direction, int current_direction, char movement_type) {
    // how this works is that whenever we exit the maze, we can only re-enter from that same location
    //bc of rotation, we want to make sure that the only way we can move while outside the maze is to go back in
    if (exit_direction == 0) { // went out the exit
        if (current_direction == 0 && movement_type == 'B') return true; // backwards
        if (current_direction == 1 && movement_type == 'R') return true; // slide right
        if (current_direction == 2 && movement_type == 'F') return true; // forwards
        if (current_direction == 3 && movement_type == 'L') return true; // slide lLeft
    } else if (exit_direction == 2) { //went out the entrance
        if (current_direction == 0 && movement_type == 'F') return true; // forwards
        if (current_direction == 1 && movement_type == 'L') return true; // slide left
        if (current_direction == 2 && movement_type == 'B') return true; // backwards
        if (current_direction == 3 && movement_type == 'R') return true; // slide right
    }

    return false; // if none of the conditions match, then movement isnt allowed
}

void forward() {
    if (is_animating) {
        return;
    }

    // flag to track if movement is allowed at the end of all these conditions
    int can_animate = 0;

    if (!inside_maze) {
        // outside the maze, movement is only allowed if going back into maze
        if (can_reenter_maze(exit_direction, direction, 'F')) {
            inside_maze = true;
            exit_direction = -1;// reset exit direction
            // move back into maze but dont update player position bc its still saved as if we are still in the maze
            if (direction == 0) { 
                target_eye_z = eye_z - 2; target_at_z = at_z - 2; 
                target_eye_x = eye_x; target_at_x = at_x; 
            } else if (direction == 2) { 
                target_eye_z = eye_z + 2; target_at_z = at_z + 2; 
                target_eye_x = eye_x; target_at_x = at_x; 
            }
            can_animate = 1;
        }
    } else {
        // check if we are leaving the maze through the exit or entrance
        if ((player_row == 0 && player_col == maze_x_size - 1 && direction == 0) || // leaving thru exit
            (player_row == maze_z_size - 1 && player_col == 0 && direction == 2)) {
            // move depending on entrance or exit
            if (direction == 0) { 
                target_eye_z = eye_z - 2; target_at_z = at_z - 2; 
                target_eye_x = eye_x; target_at_x = at_x; 
            } else if (direction == 2) { 
                target_eye_z = eye_z + 2; target_at_z = at_z + 2; 
                target_eye_x = eye_x; target_at_x = at_x; 
            }
            inside_maze = false; //dont update player position, but mark that we are outside maze now
            exit_direction = direction; // update exit direction
            can_animate = 1;
        } else if (can_move_inside_maze(player_row, player_col, direction)) {
            // normal movement conditions, just move forward if no wall is blocking
            if (direction == 0) { //north
                player_row--; 
                target_eye_z = eye_z - 2; target_at_z = at_z - 2; 
                target_eye_x = eye_x; target_at_x = at_x; 
            } else if (direction == 1) {  //east
                player_col++; 
                target_eye_x = eye_x + 2; target_at_x = at_x + 2; 
                target_eye_z = eye_z; target_at_z = at_z; 
            } else if (direction == 2) { //south
                player_row++; 
                target_eye_z = eye_z + 2; target_at_z = at_z + 2; 
                target_eye_x = eye_x; target_at_x = at_x; 
            } else if (direction == 3) { //west
                player_col--; 
                target_eye_x = eye_x - 2; target_at_x = at_x - 2; 
                target_eye_z = eye_z; target_at_z = at_z; 
            }
            can_animate = 1;
        }
    }

    if (can_animate) {
        // start animation
        forward_animation = 1;
        num_steps = 0;
        is_animating = 1;
    }
}

void backward() {
    if (is_animating) {
        return;
    }

    // flag to track if movement is allowed
    int can_animate = 0;

    if (!inside_maze) {
        // outside the maze, movement is only allowed if going back into maze
        if (can_reenter_maze(exit_direction, direction, 'B')) {
            inside_maze = true;
            exit_direction = -1; // reset exit direction
            // move back into maze but dont update player position bc its still saved as if we are still in the maze
            if (direction == 0) {
                target_eye_z = eye_z + 2; target_at_z = at_z + 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            } else if (direction == 2) {
                target_eye_z = eye_z - 2; target_at_z = at_z - 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            }
            can_animate = 1;
        }
    } else {
        // check if we are leaving the maze or moving within
        if ((player_row == maze_z_size - 1 && player_col == 0 && direction == 0) || // leaving thru entrance
            (player_row == 0 && player_col == maze_x_size - 1 && direction == 2)) { // leaving thru exit
            // then move depending on entrance or exit
            if (direction == 0) {
                target_eye_z = eye_z + 2; target_at_z = at_z + 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            } else if (direction == 2) {
                target_eye_z = eye_z - 2; target_at_z = at_z - 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            }
            inside_maze = false;
            exit_direction = (direction + 2) % 4; // update exit direction to the opposite of current direction (bc if we are facing north(0) and we back up out of the entrance, then our exit direction is 2 bc we were moving in the south(2) direction)
            can_animate = 1;
        } else if (can_move_inside_maze(player_row, player_col, (direction + 2) % 4)) { //when calling can_move(), we use (direction + 2) % 4) because we want to check the wall behind us
            // normal movement conditions, move if no wall is blocking
            if (direction == 0) { //moving south
                player_row++; 
                target_eye_z = eye_z + 2; target_at_z = at_z + 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            } else if (direction == 1) { //moving west
                player_col--; 
                target_eye_x = eye_x - 2; target_at_x = at_x - 2; 
                target_eye_z = eye_z; target_at_z = at_z;
            } else if (direction == 2) { //moving north
                player_row--; 
                target_eye_z = eye_z - 2; target_at_z = at_z - 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            } else if (direction == 3) { //moving east
                player_col++; 
                target_eye_x = eye_x + 2; target_at_x = at_x + 2; 
                target_eye_z = eye_z; target_at_z = at_z;
            }
            can_animate = 1;
        }
    }

    if (can_animate) {
        // start backward animation
        backward_animation = 1;
        num_steps = 0;
        is_animating = 1;
    }
}

void slide_left() {
    if (is_animating) {
        return;
    }

    // flag to track if movement is allowed
    int can_animate = 0;

    if (!inside_maze) {
        // outside the maze, movement is only allowed if going back into maze
        if (can_reenter_maze(exit_direction, direction, 'L')) {
            inside_maze = true;
            exit_direction = -1;
            if (direction == 1) {
                target_eye_z = eye_z - 2; target_at_z = at_z - 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            } else if (direction == 3) {
                target_eye_z = eye_z + 2; target_at_z = at_z + 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            }
            can_animate = 1;
        }
    } else {
        // check if we are leaving the maze through the exit or entrance
        if ((player_row == maze_z_size - 1 && player_col == 0 && direction == 3) || // leaving thru entrance
            (player_row == 0 && player_col == maze_x_size - 1 && direction == 1)) { // leaving thru exit
            // leaving the maze
            if (direction == 1) {
                target_eye_z = eye_z - 2; target_at_z = at_z - 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            } else if (direction == 3) {
                target_eye_z = eye_z + 2; target_at_z = at_z + 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            }
            inside_maze = false;
            exit_direction = (direction + 3) % 4; // exit direction is left of current direction
            can_animate = 1;
        } else if (can_move_inside_maze(player_row, player_col, (direction + 3) % 4)) {
            // normal movement conditions, move if no wall is blocking
            if (direction == 0) { //moving west
                player_col--; 
                target_eye_x = eye_x - 2; target_at_x = at_x - 2; 
                target_eye_z = eye_z; target_at_z = at_z;
            } else if (direction == 1) { //moving north
                player_row--; 
                target_eye_z = eye_z - 2; target_at_z = at_z - 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            } else if (direction == 2) { //east
                player_col++; 
                target_eye_x = eye_x + 2; target_at_x = at_x + 2; 
                target_eye_z = eye_z; target_at_z = at_z;
            } else if (direction == 3) { //south
                player_row++; 
                target_eye_z = eye_z + 2; target_at_z = at_z + 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            }
            can_animate = 1;
        }
    }

    if (can_animate) {
        // start slide left animation
        slide_left_animation = 1;
        num_steps = 0;
        is_animating = 1;
    }
}

void slide_right() {
    if (is_animating) {
        return;
    }

    // flag to track if movement is allowed
    int can_animate = 0;

    if (!inside_maze) {
        // outside the maze, movement is only allowed if going back into maze
        if (can_reenter_maze(exit_direction, direction, 'R')) {
            inside_maze = true;
            exit_direction = -1;
            if (direction == 1) {
                target_eye_z = eye_z + 2; target_at_z = at_z + 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            } else if (direction == 3) {
                target_eye_z = eye_z - 2; target_at_z = at_z - 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            }
            can_animate = 1;
        }
    } else {
        // check if we are leaving the maze through the exit or entrance
        if ((player_row == maze_z_size - 1 && player_col == 0 && direction == 1) || 
            (player_row == 0 && player_col == maze_x_size - 1 && direction == 3)) {
            // Leaving the maze
            if (direction == 1) {
                target_eye_z = eye_z + 2; target_at_z = at_z + 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            } else if (direction == 3) {
                target_eye_z = eye_z - 2; target_at_z = at_z - 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            }
            inside_maze = false;
            exit_direction = (direction + 1) % 4; // exit direction is right of current direction
            can_animate = 1;
        } else if (can_move_inside_maze(player_row, player_col, (direction + 1) % 4)) {
            // normal movement conditions, move if no wall is blocking
            if (direction == 0) { //moving east
                player_col++; 
                target_eye_x = eye_x + 2; target_at_x = at_x + 2; 
                target_eye_z = eye_z; target_at_z = at_z;
            } else if (direction == 1) { //moving south
                player_row++; 
                target_eye_z = eye_z + 2; target_at_z = at_z + 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            } else if (direction == 2) { //moving west
                player_col--; 
                target_eye_x = eye_x - 2; target_at_x = at_x - 2; 
                target_eye_z = eye_z; target_at_z = at_z;
            } else if (direction == 3) { //moving north
                player_row--; 
                target_eye_z = eye_z - 2; target_at_z = at_z - 2; 
                target_eye_x = eye_x; target_at_x = at_x;
            }
            can_animate = 1;
        }
    }

    if (can_animate) {
        // start slide right animation
        slide_right_animation = 1;
        num_steps = 0;
        is_animating = 1;
    }
}

void turn_left(){
    direction = (direction + 3) % 4; // updates player direction counterclockwise THIS NEEDS TO BE HERE

    if (direction == 0) { // facing North
        at_x = eye_x;
        at_z = eye_z - 1;
    } else if (direction == 1) { // facing East
        at_x = eye_x + 1;
        at_z = eye_z;
    } else if (direction == 2) { // facing South
        at_x = eye_x;
        at_z = eye_z + 1;
    } else if (direction == 3) { // facing West
        at_x = eye_x - 1;
        at_z = eye_z;
    }
    model_view = look_at((vec4){eye_x, 2, eye_z, 1}, (vec4){at_x, 2, at_z, 1}, (vec4){0, 1, 0, 0});
    //model_view = look_at((vec4) {eye_x, 2, eye_z, 1}, (vec4) {at_x, 2, at_z, 1}, (vec4) {0, 1, 0, 0});
    glutPostRedisplay();
}

void turn_right(){
    direction = (direction + 1) % 4; //updates player direction clockwise THIS NEEDS TO BE HERE

    if (direction == 0) { // facing North
        at_x = eye_x;
        at_z = eye_z - 1;
    } else if (direction == 1) { // facing East
        at_x = eye_x + 1;
        at_z = eye_z;
    } else if (direction == 2) { // facing South
        at_x = eye_x;
        at_z = eye_z + 1;
    } else if (direction == 3) { // facing West
        at_x = eye_x - 1;
        at_z = eye_z;
    }
    model_view = look_at((vec4){eye_x, 2, eye_z, 1}, (vec4){at_x, 2, at_z, 1}, (vec4){0, 1, 0, 0});
    /*
    mat4 rot = rotate_y((-90.0 * 180.0) / M_PI);
    vec4 new_at_point = mv_multiplication(rot, (vec4) {at_x, 2, at_z, 1});
    //mat4 tran = translate(0, 0, 0);
    //vec4 new_eye_point = mv_multiplication(tran, (vec4) {eye_x, 2, eye_z, 1});
    //mat4 m = look_at((vec4) {0, 0, 0, 1}, new_at_point, (vec4) {0, 1, 0, 0});
    mat4 m1 = look_at((vec4) {0, 0, 0, 1}, (vec4) {at_x, 2, at_z, 1}, (vec4) {0, 1, 0, 0});
    mat4 m2 = look_at((vec4) {0, 0, 0, 1}, new_at_point, (vec4) {0, 1, 0, 0});
    mat4 m3 = mm_multiplication(m2, m1);
    
    model_view = m3;
    //model_view = mm_multiplication(look_at((vec4) {eye_x, 2, eye_z, 1}, new_at_point, (vec4) {0, 1, 0, 0}), m3);
    */
    glutPostRedisplay();
    
}

void create_rotation() {
    vec4 u = {initial_point.x, initial_point.y, initial_point.z, 0.0f};
    vec4 v = {final_point.x, final_point.y, final_point.z, 0.0f};

    normalize(u);
    normalize(v);

    // Calculate dot product and clamp between -1 and 1
    float dt = dot_product(u, v);
    float theta_z = acos(dt) * 180.0f / M_PI;

    // Calculate the rotation axis
    vec4 rotation_axis = cross_product(u, v);
    rotation_axis = normalize(rotation_axis);

    float distance = sqrt(rotation_axis.y * rotation_axis.y + rotation_axis.z * rotation_axis.z);

    float sin_theta_x = rotation_axis.y / distance;
    float cos_theta_x = rotation_axis.z / distance;

    mat4 pos_rx = {{1, 0, 0, 0}, {0, cos_theta_x, sin_theta_x, 0}, {0, -sin_theta_x, cos_theta_x, 0}, {0, 0, 0, 1}};
    mat4 neg_rx = m_transpose(pos_rx);

    float sin_theta_y = rotation_axis.x;
    float cos_theta_y = distance;

    mat4 neg_ry = {{cos_theta_y, 0, sin_theta_y, 0}, {0, 1, 0, 0}, {-sin_theta_y, 0, cos_theta_y, 0}, {0, 0, 0, 1}};
    mat4 pos_ry = m_transpose(neg_ry);

    // Final rotation matrix calculation
    mat4 m1 = mm_multiplication(neg_ry, pos_rx);
    mat4 m2 = mm_multiplication(rotate_z(theta_z), m1);
    mat4 m3 = mm_multiplication(pos_ry, m2);
    mat4 m4 = mm_multiplication(neg_rx, m3);
    
    rotation_matrix = m4;
}

void prompt_user() {
    printf("Enter the maze width along the x-axis (positive integer): ");
    if (scanf("%d", &maze_x_size) != 1 || maze_x_size <= 0) {
        fprintf(stderr, "Invalid input. Maze width must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }
    printf("Enter the maze depth along the z-axis (positive integer): ");
    if (scanf("%d", &maze_z_size) != 1 || maze_z_size <= 0) {
        fprintf(stderr, "Invalid input. Maze depth must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }
    x_size = (maze_x_size * 3 + maze_x_size + 1) + 10;
    z_size = (maze_z_size * 3 + maze_z_size + 1) + 10;
}

void cleanup() {
    if (positions) free(positions);
    if (tex_coords) free(tex_coords);
    if (block_positions) free(block_positions);
    if (block_tex_coords) free(block_tex_coords);
    free_maze(maze_z_size);
}

int main(int argc, char **argv)
{
    prompt_user();
    srand(time(NULL));

    //generate and print the maze
    make_maze(maze_z_size, maze_x_size);
    printf("Generated Maze:\n");
    print_maze(maze_z_size, maze_x_size);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Project2/3");
    #ifndef __APPLE__
    glewInit();
    #endif
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutSpecialFunc(special);
    glutIdleFunc(idle);
    glutMainLoop();

    cleanup();
    return 0;
}