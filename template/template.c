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

//cube and platform global variables
float scale_cube = 0.25f;
int num_vertices_per_block = 36; 
int num_blocks = 0; 
int num_vertices = 0;
int x_size = 5;
int z_size = 5; 

//array global variable
vec4 *positions = NULL;
vec2 *tex_coords = NULL;
vec4 *block_positions = NULL; // Store positions for a single block
vec2 *block_tex_coords = NULL; 

//sun global varibles
vec4 *sun_positions = NULL;
vec2 *sun_tex_coords = NULL;
int num_vertices_sun = 0; // Number of vertices for the sun
vec4 sun_position = {0.0f, 4.0f, 0.0f, 1.0f};

//platform reset:
bool resetting = false; // Animation state
int reset_step = 0; // Current step in the reset process
int reset_steps = 100; // Total number of steps for the reset
mat4 delta_ctm; // Incremental change matrix

void init(void)
{
    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    init_grassblock();
    generate_pyramid(x_size, z_size);
    display_sun();

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
    glGenVertexArraysAPPLE(1, &vao);
    glBindVertexArrayAPPLE(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size_positions + size_text_coords, NULL,
    GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size_positions, positions);
    glBufferSubData(GL_ARRAY_BUFFER, size_positions, size_text_coords,
    tex_coords);
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (0));
    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
    (GLvoid *) (size_positions));

    GLuint texture_location = glGetUniformLocation(program, "texture");
    glUniform1i(texture_location, 0);

    ctm_location = glGetUniformLocation(program, "ctm");

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
            init_texture(.25, .75);
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
}

void idleReset() {
    if (resetting && reset_step < reset_steps) {
        // Incrementally reduce CTM
        ctm = mm_subtraction(ctm, delta_ctm);
        ++reset_step;

        // Redisplay after each step
        glutPostRedisplay();
    } else {
        // Reset complete
        resetting = false;
        glutIdleFunc(NULL); // Stop the idle function
    }
}

void resetPlatform() {
    if (!resetting) {
        resetting = true;
        reset_step = 0;

        // Calculate the difference between CTM and identity
        mat4 identity = {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}};
        mat4 diff = mm_subtraction(ctm, identity);

        // Divide by steps to get the delta matrix
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

        // Register the idle function
        glutIdleFunc(idleReset);
    }
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the scene
    mat4 scaling_matrix = scale(scale_factor, scale_factor, scale_factor);
    mat4 final_ctm = mm_multiplication(scaling_matrix, ctm);
    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *)&final_ctm);

    // Draw all vertices except the sun's vertices
    glDrawArrays(GL_TRIANGLES, 0, num_vertices - num_vertices_sun);

    // Draw the sun
    float sun_scale = 3.0f; // The size of the sun
    mat4 sun_scaling = scale(sun_scale, sun_scale, sun_scale);
    mat4 sun_translation = translate(sun_position.x, sun_position.y, sun_position.z);
    mat4 sun_ctm = mm_multiplication(sun_translation, sun_scaling);

    // Apply any global scaling if necessary
    mat4 sun_final_ctm = mm_multiplication(final_ctm, sun_ctm);

    // Send the sun's ctm to the shader
    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat *)&sun_final_ctm);

    // Draw only the sun's vertices
    glDrawArrays(GL_TRIANGLES, num_vertices - num_vertices_sun, num_vertices_sun);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int mousex, int mousey) {
    float rotation_angle = 20.0f; // Rotation angle in degrees
    if (key == 'i') { // Zoom In
        float zoom_increment = 1.15f;
        scale_factor *= zoom_increment;
        glutPostRedisplay();
    }
    else if (key == 'o') { // Zoom Out
        float zoom_decrement = 1.0f / 1.15f;
        scale_factor *= zoom_decrement;
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
    else if (key == ' ') { // Reset platform
        resetPlatform();
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
        create_rotation();

        // Update the current transformation matrix by applying the incremental rotation
        ctm = mm_multiplication(rotation_matrix, prev_ctm);

        // Update previous_point for the next motion event
        previous_point = final_point;
        glutPostRedisplay();
    }
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
    printf("Enter the platform width along the x-axis (positive integer): ");
    if (scanf("%d", &x_size) != 1 || x_size <= 0) {
        fprintf(stderr, "Invalid input. Platform width must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }
    printf("Enter the platform depth along the z-axis (positive integer): ");
    if (scanf("%d", &z_size) != 1 || z_size <= 0) {
        fprintf(stderr, "Invalid input. Platform depth must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }
}

void cleanup() {
    if (positions) free(positions);
    if (tex_coords) free(tex_coords);
    if (block_positions) free(block_positions);
    if (block_tex_coords) free(block_tex_coords);
}

int main(int argc, char **argv)
{
    prompt_user();
    srand(time(NULL));
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
    glutMainLoop();

    cleanup();
    return 0;
}