#ifndef _TEMPLIB_H_
#define _TEMPLIB_H_

typedef struct {
    float x;
    float y;
    float z;
    float w;
} vec4;

typedef struct {
    float x;
    float y;
    float z;
} vec3;

typedef struct {
    float x;
    float y;
} vec2;


typedef struct {
    vec4 x;
    vec4 y;
    vec4 z;
    vec4 w;
} mat4;

typedef struct {
    vec3 x;
    vec3 y;
    vec3 z;
} mat3;

//function for printing vectors
void print_v4(vec4);

//vector operaitons functions
vec4 sv_multiplication(vec4 v, float s);
vec4 vv_addition(vec4 v1, vec4 v2);
vec4 vv_subtraction(vec4 v1, vec4 v2);
float v_magnitude(vec4 v);
vec4 normalize(vec4 v);
float dot_product(vec4 v1, vec4 v2);
vec4 cross_product(vec4 v1, vec4 v2);

//function to print matrix
void print_matrix(mat4 m);

//matrix operations
mat4 sm_multiplication(mat4 m1, float s);
mat4 mm_addition(mat4 m1, mat4 m2);
mat4 mm_subtraction(mat4 m1, mat4 m2);
mat4 mm_multiplication(mat4 m1, mat4 m2);
mat4 m_transpose(mat4 m);
vec4 mv_multiplication(mat4 m, vec4 v);

//inverse operations
mat4 inverse(mat4 m);
mat4 m_minor(mat4 m);
mat4 m_cofactor(mat4 m);
float determinant(mat3 m);
float m4_determinant(mat4 m1, mat4 m2);
mat4 identity_matrix();

//transformation operations
vec4 transformation(mat4 m, vec4 v);
mat4 translate(float alpha_x, float alpha_y, float alpha_z);
mat4 scale(float beta_x, float beta_y, float beta_z);


//rotation functions
float degrees_to_radians(float degrees);
mat4 rotate_y(float theta);
mat4 rotate_x(float theta);
mat4 rotate_z(float theta);

//model_view
mat4 look_at(vec4 eye, vec4 at, vec4 up);

//projection functions
mat4 ortho(float left, float right, float bottom, float top, float near, float far);
mat4 frustum(float left, float right, float bottom, float top, float near, float far);

#endif