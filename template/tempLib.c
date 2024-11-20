#include "tempLib.h"
#include <stdio.h>
#include <math.h>
#include <OpenGL/gltypes.h>

void print_v4(vec4 v) {
    printf("[ %7.3f %7.3f %7.3f %7.3f ]\n", v.x, v.y, v.z, v.w);
}

//scalar multiplication
vec4 sv_multiplication(vec4 v, float s){
    vec4 result;
    result.x = (v.x) * (s);
    result.y = (v.y) * (s);
    result.z = (v.z) * (s);
    result.w = (v.w) * (s);
    return result;
}

//vector additions
vec4 vv_addition(vec4 v1, vec4 v2){
    vec4 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    result.w = v1.w + v2.w;
    return result;
}

//vector subtraction
vec4 vv_subtraction(vec4 v1, vec4 v2){
    vec4 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    result.w = v1.w - v2.w;
    return result;
}

//vector magnitude
float v_magnitude(vec4 v){
    return sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w));
}

//normalize
vec4 normalize(vec4 v){
    float mag = v_magnitude(v);

    //prevents divide by 0.
    if  (mag == 0) return v;

    vec4 result;
    result.x = v.x/mag;
    result.y = v.y/mag;
    result.z = v.z/mag;
    result.w = v.w/mag;
    return result;
}

//dot product
float dot_product(vec4 v1, vec4 v2){
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
}

//cross product 
vec4 cross_product(vec4 v1, vec4 v2){
    vec4 result;
    result.x = (v1.y * v2.z) - (v1.z * v2.y);
    result.y = (v1.z * v2.x) - (v1.x * v2.z);
    result.z = (v1.x * v2.y) - (v1.y * v2.x);
    result.w = 0;
    return result;
}

//prints matrix for debugging
void print_matrix(mat4 m){
    printf("[ %7.3f %7.3f %7.3f %7.3f ]\n", m.x.x, m.y.x, m.z.x, m.w.x);
    printf("[ %7.3f %7.3f %7.3f %7.3f ]\n", m.x.y, m.y.y, m.z.y, m.w.y);
    printf("[ %7.3f %7.3f %7.3f %7.3f ]\n", m.x.z, m.y.z, m.z.z, m.w.z);
    printf("[ %7.3f %7.3f %7.3f %7.3f ]\n", m.x.w, m.y.w, m.z.w, m.w.w);
}

//matrix operations
mat4 sm_multiplication(mat4 m, float s){
    mat4 result;

    //vector 1 ---> x.x implies first vectors first element so on and so forth.
    result.x.x = s * m.x.x;
    result.x.y = s * m.x.y;
    result.x.z = s * m.x.z; 
    result.x.w = s * m.x.w; 

    //vector 2
    result.y.x = s * m.y.x;
    result.y.y = s * m.y.y;
    result.y.z = s * m.y.z;
    result.y.w = s * m.y.w;

    //vector 3
    result.z.x = s * m.z.x;
    result.z.y = s * m.z.y;
    result.z.z = s * m.z.z;
    result.z.w = s * m.z.w;

    //vector 4
    result.w.x = s * m.w.x;
    result.w.y = s * m.w.y;
    result.w.z = s * m.w.z;
    result.w.w = s * m.w.w;

    return result;
}

mat4 mm_addition(mat4 m1, mat4 m2){
    mat4 result;
    
    //vector 1
    result.x.x = m1.x.x + m2.x.x;
    result.x.y = m1.x.y + m2.x.y;
    result.x.z = m1.x.z + m2.x.z;
    result.x.w = m1.x.w + m2.x.w;

    //vector 2
    result.y.x = m1.y.x + m2.y.x;
    result.y.y = m1.y.y + m2.y.y;
    result.y.z = m1.y.z + m2.y.z;
    result.y.w = m1.y.w + m2.y.w;
    
    //vector 3
    result.z.x = m1.z.x + m2.z.x;
    result.z.y = m1.z.y + m2.z.y;
    result.z.z = m1.z.z + m2.z.z;
    result.z.w = m1.z.w + m2.z.w;

    //vector 4
    result.w.x = m1.w.x + m2.w.x;
    result.w.y = m1.w.y + m2.w.y;
    result.w.z = m1.w.z + m2.w.z;
    result.w.w = m1.w.w + m2.w.w;

    return result;
}

mat4 mm_subtraction(mat4 m1, mat4 m2){
    mat4 result;
    
    //vector 1
    result.x.x = m1.x.x - m2.x.x;
    result.x.y = m1.x.y - m2.x.y;
    result.x.z = m1.x.z - m2.x.z;
    result.x.w = m1.x.w - m2.x.w;

    //vector 2
    result.y.x = m1.y.x - m2.y.x;
    result.y.y = m1.y.y - m2.y.y;
    result.y.z = m1.y.z - m2.y.z;
    result.y.w = m1.y.w - m2.y.w;
    
    //vector 3
    result.z.x = m1.z.x - m2.z.x;
    result.z.y = m1.z.y - m2.z.y;
    result.z.z = m1.z.z - m2.z.z;
    result.z.w = m1.z.w - m2.z.w;

    //vector 4
    result.w.x = m1.w.x - m2.w.x;
    result.w.y = m1.w.y - m2.w.y;
    result.w.z = m1.w.z - m2.w.z;
    result.w.w = m1.w.w - m2.w.w;

    return result;
}

mat4 mm_multiplication(mat4 m1, mat4 m2){
    mat4 result;

    //vector 1
    result.x.x = (m1.x.x * m2.x.x) + (m1.y.x * m2.x.y) + (m1.z.x * m2.x.z) + (m1.w.x * m2.x.w);
    result.x.y = (m1.x.y * m2.x.x) + (m1.y.y * m2.x.y) + (m1.z.y * m2.x.z) + (m1.w.y * m2.x.w);
    result.x.z = (m1.x.z * m2.x.x) + (m1.y.z * m2.x.y) + (m1.z.z * m2.x.z) + (m1.w.z * m2.x.w);
    result.x.w = (m1.x.w * m2.x.x) + (m1.y.w * m2.x.y) + (m1.z.w * m2.x.z) + (m1.w.w * m2.x.w);

    //vector2
    result.y.x = (m1.x.x * m2.y.x) + (m1.y.x * m2.y.y) + (m1.z.x * m2.y.z) + (m1.w.x * m2.y.w);
    result.y.y = (m1.x.y * m2.y.x) + (m1.y.y * m2.y.y) + (m1.z.y * m2.y.z) + (m1.w.y * m2.y.w);
    result.y.z = (m1.x.z * m2.y.x) + (m1.y.z * m2.y.y) + (m1.z.z * m2.y.z) + (m1.w.z * m2.y.w);
    result.y.w = (m1.x.w * m2.y.x) + (m1.y.w * m2.y.y) + (m1.z.w * m2.y.z) + (m1.w.w * m2.y.w);

    //vector 3
    result.z.x = (m1.x.x * m2.z.x) + (m1.y.x * m2.z.y) + (m1.z.x * m2.z.z) + (m1.w.x * m2.z.w);
    result.z.y = (m1.x.y * m2.z.x) + (m1.y.y * m2.z.y) + (m1.z.y * m2.z.z) + (m1.w.y * m2.z.w);
    result.z.z = (m1.x.z * m2.z.x) + (m1.y.z * m2.z.y) + (m1.z.z * m2.z.z) + (m1.w.z * m2.z.w);
    result.z.w = (m1.x.w * m2.z.x) + (m1.y.w * m2.z.y) + (m1.z.w * m2.z.z) + (m1.w.w * m2.z.w);

    //vector 4
    result.w.x = (m1.x.x * m2.w.x) + (m1.y.x * m2.w.y) + (m1.z.x * m2.w.z) + (m1.w.x * m2.w.w);
    result.w.y = (m1.x.y * m2.w.x) + (m1.y.y * m2.w.y) + (m1.z.y * m2.w.z) + (m1.w.y * m2.w.w);
    result.w.z = (m1.x.z * m2.w.x) + (m1.y.z * m2.w.y) + (m1.z.z * m2.w.z) + (m1.w.z * m2.w.w);
    result.w.w = (m1.x.w * m2.w.x) + (m1.y.w * m2.w.y) + (m1.z.w * m2.w.z) + (m1.w.w * m2.w.w);

    return result;
}


mat4 inverse(mat4 m){
    mat4 minor = m_minor(m); 
    mat4 cofactor = m_cofactor(minor);
    mat4 transpose = m_transpose(cofactor);
    float determinant = m4_determinant(m, minor);  
    
    //check if determinent is 0, since we cannot divide by 0
    if(determinant == 0){
        printf("determinant is 0, therefore no inverse can be found");
    }

    mat4 m_inv = sm_multiplication(transpose, 1/determinant);
    return m_inv;
}

mat4 m_minor(mat4 m) {
    mat4 minor;

    //mat1
    mat3 m11; 
    m11.x.x = m.y.y; m11.x.y = m.y.z; m11.x.z = m.y.w;
    m11.y.x = m.z.y; m11.y.y = m.z.z; m11.y.z = m.z.w;
    m11.z.x = m.w.y; m11.z.y = m.w.z; m11.z.z = m.w.w;

    //mat2
    mat3 m21;
    m21.x.x = m.y.x; m21.x.y = m.y.z; m21.x.z = m.y.w;
    m21.y.x = m.z.x; m21.y.y = m.z.z; m21.y.z = m.z.w;
    m21.z.x = m.w.x; m21.z.y = m.w.z; m21.z.z = m.w.w;

    //mat3
    mat3 m31;
    m31.x.x = m.y.x; m31.x.y = m.y.y; m31.x.z = m.y.w; 
    m31.y.x = m.z.x; m31.y.y = m.z.y; m31.y.z = m.z.w;
    m31.z.x = m.w.x; m31.z.y = m.w.y; m31.z.z = m.w.w;

    //mat4
    mat3 m41;
    m41.x.x = m.y.x; m41.x.y = m.y.y; m41.x.z = m.y.z;
    m41.y.x = m.z.x; m41.y.y = m.z.y; m41.y.z = m.z.z;
    m41.z.x = m.w.x; m41.z.y = m.w.y; m41.z.z = m.w.z;

    //mat5
    mat3 m12;
    m12.x.x = m.x.y; m12.x.y = m.x.z; m12.x.z = m.x.w;
    m12.y.x = m.z.y; m12.y.y = m.z.z; m12.y.z = m.z.w;
    m12.z.x = m.w.y; m12.z.y = m.w.z; m12.z.z = m.w.w;

    //mat6
    mat3 m22;
    m22.x.x = m.x.x; m22.x.y = m.x.z; m22.x.z = m.x.w;
    m22.y.x = m.z.x; m22.y.y = m.z.z; m22.y.z = m.z.w;
    m22.z.x = m.w.x; m22.z.y = m.w.z; m22.z.z = m.w.w;

    //mat7
    mat3 m32;
    m32.x.x = m.x.x; m32.x.y = m.x.y; m32.x.z = m.x.w;
    m32.y.x = m.z.x; m32.y.y = m.z.y; m32.y.z = m.z.w; 
    m32.z.x = m.w.x; m32.z.y = m.w.y; m32.z.z = m.w.w;

    //mat8
    mat3 m42;
    m42.x.x = m.x.x; m42.x.y = m.x.y; m42.x.z = m.x.z;
    m42.y.x = m.z.x; m42.y.y = m.z.y; m42.y.z = m.z.z;
    m42.z.x = m.w.x; m42.z.y = m.w.y; m42.z.z = m.w.z;

    //mat9
    mat3 m13;
    m13.x.x = m.x.y; m13.x.y = m.x.z; m13.x.z = m.x.w;
    m13.y.x = m.y.y; m13.y.y = m.y.z; m13.y.z = m.y.w;
    m13.z.x = m.w.y; m13.z.y = m.w.z; m13.z.z = m.w.w;

    //mat10
    mat3 m23;
    m23.x.x = m.x.x; m23.x.y = m.x.z; m23.x.z = m.x.w;
    m23.y.x = m.y.x; m23.y.y = m.y.z; m23.y.z = m.y.w;
    m23.z.x = m.w.x; m23.z.y = m.w.z; m23.z.z = m.w.w;

    //mat11
    mat3 m33;
    m33.x.x = m.x.x; m33.x.y = m.x.y; m33.x.z = m.x.w;
    m33.y.x = m.y.x; m33.y.y = m.y.y; m33.y.z = m.y.w;
    m33.z.x = m.w.x; m33.z.y = m.w.y; m33.z.z = m.w.w;

    //mat12
    mat3 m43;
    m43.x.x = m.x.x; m43.x.y = m.x.y; m43.x.z = m.x.z;
    m43.y.x = m.y.x; m43.y.y = m.y.y; m43.y.z = m.y.z;
    m43.z.x = m.w.x; m43.z.y = m.w.y; m43.z.z = m.w.z;

    //mat13
    mat3 m14;
    m14.x.x = m.x.y; m14.x.y = m.x.z; m14.x.z = m.x.w;
    m14.y.x = m.y.y; m14.y.y = m.y.z; m14.y.z = m.y.w;
    m14.z.x = m.z.y; m14.z.y = m.z.z; m14.z.z = m.z.w;

    //mat14
    mat3 m24;
    m24.x.x = m.x.x; m24.x.y = m.x.z; m24.x.z = m.x.w;
    m24.y.x = m.y.x; m24.y.y = m.y.z; m24.y.z = m.y.w;
    m24.z.x = m.z.x; m24.z.y = m.z.z; m24.z.z = m.z.w;

    //mat15
    mat3 m34;
    m34.x.x = m.x.x; m34.x.y = m.x.y; m34.x.z = m.x.w;
    m34.y.x = m.y.x; m34.y.y = m.y.y; m34.y.z = m.y.w;
    m34.z.x = m.z.x; m34.z.y = m.z.y; m34.z.z = m.z.w;

    //mat16
    mat3 m44;
    m44.x.x = m.x.x; m44.x.y = m.x.y; m44.x.z = m.x.z;
    m44.y.x = m.y.x; m44.y.y = m.y.y; m44.y.z = m.y.z; 
    m44.z.x = m.z.x; m44.z.y = m.z.y; m44.z.z = m.z.z;

    //vec 1
    minor.x.x = determinant(m11);
    minor.x.y = determinant(m21);
    minor.x.z = determinant(m31);
    minor.x.w = determinant(m41);

    //vec2
    minor.y.x = determinant(m12);
    minor.y.y = determinant(m22);
    minor.y.z = determinant(m32);
    minor.y.w = determinant(m42);

    //vec3
    minor.z.x = determinant(m13);
    minor.z.y = determinant(m23);
    minor.z.z = determinant(m33);
    minor.z.w = determinant(m43);

    //vec4
    minor.w.x = determinant(m14);
    minor.w.y = determinant(m24);
    minor.w.z = determinant(m34);
    minor.w.w = determinant(m44);
    
    return minor;
}

float determinant(mat3 m) {
    return (m.x.x * m.y.y * m.z.z) + (m.y.x * m.z.y * m.x.z) + (m.z.x * m.x.y * m.y.z) - 
            (m.x.z * m.y.y * m.z.x) - (m.y.z * m.z.y * m.x.x) - (m.z.z * m.x.y * m.y.x);
}

mat4 m_cofactor(mat4 m) {
    mat4 cofactor;

    //vec1
    cofactor.x.x =  m.x.x;
    cofactor.x.y = -m.x.y;
    cofactor.x.z =  m.x.z;
    cofactor.x.w = -m.x.w;

    //vec2
    cofactor.y.x = -m.y.x;
    cofactor.y.y =  m.y.y;
    cofactor.y.z = -m.y.z;
    cofactor.y.w =  m.y.w;

    //vec3
    cofactor.z.x =  m.z.x;
    cofactor.z.y = -m.z.y;
    cofactor.z.z =  m.z.z;
    cofactor.z.w = -m.z.w;

    //vec4
    cofactor.w.x = -m.w.x;
    cofactor.w.y =  m.w.y;
    cofactor.w.z = -m.w.z;
    cofactor.w.w =  m.w.w;

    return cofactor;
}


mat4 m_transpose(mat4 m) {
    mat4 result;
    //bascically translating the columns into rows
    //vec1
    result.x.x = m.x.x;
    result.x.y = m.y.x;
    result.x.z = m.z.x;
    result.x.w = m.w.x;

    //vec2
    result.y.x = m.x.y;
    result.y.y = m.y.y;
    result.y.z = m.z.y;
    result.y.w = m.w.y;

    //vec3
    result.z.x = m.x.z;
    result.z.y = m.y.z;
    result.z.z = m.z.z;
    result.z.w = m.w.z;

    //vec4
    result.w.x = m.x.w;
    result.w.y = m.y.w;
    result.w.z = m.z.w;
    result.w.w = m.w.w;

    return result;
}

float m4_determinant(mat4 m1, mat4 minor){
    return (m1.x.x * minor.x.x) - (m1.x.y * minor.x.y) + (m1.x.z * minor.x.z) - (m1.x.w * minor.x.w);
}

vec4 mv_multiplication (mat4 m, vec4 v){
    vec4 result;

    result.x = (m.x.x * v.x) + (m.y.x * v.y) + (m.z.x * v.z) + (m.w.x * v.w);
    result.y = (m.x.y * v.x) + (m.y.y * v.y) + (m.z.y * v.z) + (m.w.y * v.w);
    result.z = (m.x.z * v.x) + (m.y.z * v.y) + (m.z.z * v.z) + (m.w.z * v.w);
    result.w = (m.x.w * v.x) + (m.y.w * v.y) + (m.z.w * v.z) + (m.w.w * v.w);

    return result;
}

mat4 identity_matrix() {
    mat4 identity;

    identity.x.x = 1.0f; identity.x.y = 0.0f; identity.x.z = 0.0f; identity.x.w = 0.0f;
    identity.y.x = 0.0f; identity.y.y = 1.0f; identity.y.z = 0.0f; identity.y.w = 0.0f;
    identity.z.x = 0.0f; identity.z.y = 0.0f; identity.z.z = 1.0f; identity.z.w = 0.0f;
    identity.w.x = 0.0f; identity.w.y = 0.0f; identity.w.z = 0.0f; identity.w.w = 1.0f;

    return identity;
}

vec4 transformation(mat4 m, vec4 v){
    mat4 affline;
    vec4 point;

    //set the point vector
    point.x = v.x;
    point.y = v.y;
    point.z = v.z;
    point.w = 1;

    //vec1
    affline.x.x = m.x.x;
    affline.x.y = m.x.y;
    affline.x.z = m.x.z;
    affline.x.w = 0;

    //vec2
    affline.y.x = m.y.x;
    affline.y.y = m.y.y;
    affline.y.z = m.y.z;
    affline.y.w = 0;

    //vec3
    affline.z.x = m.z.x;
    affline.z.y = m.z.y;
    affline.z.z = m.z.z;
    affline.z.w = 0;

    //vec4
    affline.w.x = m.w.x;
    affline.w.y = m.w.y;
    affline.w.z = m.w.z;
    affline.w.w = 1;

    vec4 result = mv_multiplication(affline, point);

    return result;
}

mat4 translate(float alpha_x, float alpha_y, float alpha_z){
    mat4 translation;

    //vec 1
    translation.x.x = 1.0f;
    translation.x.y = 0.0f;
    translation.x.z = 0.0f;
    translation.x.w = 0.0f;

    //vec2
    translation.y.x = 0.0f;
    translation.y.y = 1.0f;
    translation.y.z = 0.0f;
    translation.y.w = 0.0f;

    //vec 3
    translation.z.x = 0.0f;
    translation.z.y = 0.0f;
    translation.z.z = 1.0f;
    translation.z.w = 0.0f;

    //alpha vector
    translation.w.x = alpha_x;
    translation.w.y = alpha_y;
    translation.w.z = alpha_z;
    translation.w.w = 1;

    return translation;
}

mat4 scale(float beta_x, float beta_y, float beta_z){
    mat4 scale;

    //vec 1
    scale.x.x = beta_x;
    scale.x.y = 0.0f;
    scale.x.z = 0.0f;
    scale.x.w = 0.0f;

    //vec2
    scale.y.x = 0.0f;
    scale.y.y = beta_y;
    scale.y.z = 0.0f;
    scale.y.w = 0.0f;

    //vec 3
    scale.z.x = 0.0f;
    scale.z.y = 0.0f;
    scale.z.z = beta_z;
    scale.z.w = 0.0f;

    //alpha vector
    scale.w.x = 0.0f;
    scale.w.y = 0.0f;
    scale.w.z = 0.0f;
    scale.w.w = 1;

    return scale;
}

// Function to convert degrees to radians
float degrees_to_radians(float degrees) {
    return degrees * (M_PI / 180.0f);
}

// Rotate about the Z-axis
mat4 rotate_z(float theta) {
    mat4 rotation_matrix;
    float rad = degrees_to_radians(theta);

    rotation_matrix.x.x = cos(rad);
    rotation_matrix.x.y = sin(rad);
    rotation_matrix.x.z = 0.0f;
    rotation_matrix.x.w = 0.0f;

    rotation_matrix.y.x = -sin(rad);
    rotation_matrix.y.y = cos(rad);
    rotation_matrix.y.z = 0.0f;
    rotation_matrix.y.w = 0.0f;

    rotation_matrix.z.x = 0.0f;
    rotation_matrix.z.y = 0.0f;
    rotation_matrix.z.z = 1.0f;
    rotation_matrix.z.w = 0.0f;

    rotation_matrix.w.x = 0.0f;
    rotation_matrix.w.y = 0.0f;
    rotation_matrix.w.z = 0.0f;
    rotation_matrix.w.w = 1.0f;

    return rotation_matrix;
}

// Rotate about the X-axis
mat4 rotate_x(float theta) {
    mat4 rotation_matrix;
    float rad = degrees_to_radians(theta);

    rotation_matrix.x.x = 1.0f;
    rotation_matrix.x.y = 0.0f;
    rotation_matrix.x.z = 0.0f;
    rotation_matrix.x.w = 0.0f;

    rotation_matrix.y.x = 0.0f;
    rotation_matrix.y.y = cos(rad);
    rotation_matrix.y.z = sin(rad);
    rotation_matrix.y.w = 0.0f;

    rotation_matrix.z.x = 0.0f;
    rotation_matrix.z.y = -sin(rad);
    rotation_matrix.z.z = cos(rad);
    rotation_matrix.z.w = 0.0f;

    rotation_matrix.w.x = 0.0f;
    rotation_matrix.w.y = 0.0f;
    rotation_matrix.w.z = 0.0f;
    rotation_matrix.w.w = 1.0f;

    return rotation_matrix;
}

// Rotate about the Y-axis
mat4 rotate_y(float theta) {
    mat4 rotation_matrix;
    float rad = degrees_to_radians(theta);

    rotation_matrix.x.x = cos(rad);
    rotation_matrix.x.y = 0.0f;
    rotation_matrix.x.z = -sin(rad);
    rotation_matrix.x.w = 0.0f;

    rotation_matrix.y.x = 0.0f;
    rotation_matrix.y.y = 1.0f;
    rotation_matrix.y.z = 0.0f;
    rotation_matrix.y.w = 0.0f;

    rotation_matrix.z.x = sin(rad);
    rotation_matrix.z.y = 0.0f;
    rotation_matrix.z.z = cos(rad);
    rotation_matrix.z.w = 0.0f;

    rotation_matrix.w.x = 0.0f;
    rotation_matrix.w.y = 0.0f;
    rotation_matrix.w.z = 0.0f;
    rotation_matrix.w.w = 1.0f;

    return rotation_matrix;
}

mat4 look_at(vec4 eye, vec4 at, vec4 up){
    vec4 z_prime = normalize(vv_subtraction(eye, at));
    vec4 x_prime = normalize(cross_product(up, z_prime));
    vec4 y_prime = normalize(cross_product(z_prime, x_prime));
    mat4 r = {{x_prime.x, y_prime.x, z_prime.x, 0}, {x_prime.y, y_prime.y, z_prime.y, 0}, {x_prime.z, y_prime.z, z_prime.z, 0}, {0, 0, 0, 1}};
    mat4 t = translate(-eye.x, -eye.y, -eye.z);
    mat4 model_view_matrix = mm_multiplication(r, t);
    return model_view_matrix;
}

// Orthographic projection
mat4 orhto(float left, float right, float bottom, float top, float near, float far){
    mat4 ortho_projection_matrix = {{2 / (right - left), 0, 0, 0}, {0, 2 / (top - bottom), 0, 0}, {0, 0, 2 / (near - far), 0}, {-((right + left) / (right - left)), -((top + bottom) / (top - bottom)), -((near + far) / (near - far)), 1}};
    return ortho_projection_matrix;
}

// Perspective projection
mat4 frustum(float left, float right, float bottom, float top, float near, float far){
    mat4 perspective_projection_matrix = {{(-2 * near) / (right - left), 0, 0, 0}, 
    {0, (-2 * near) / (top - bottom), 0, 0}, 
    {(left + right) / (right - left), (bottom + top) / (top - bottom), (near + far) / (far - near), -1}, 
    {0, 0, - (2 * near * far) / (far - near), 0}};
    return perspective_projection_matrix;
}