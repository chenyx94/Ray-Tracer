#include <stdio.h>
#include <iostream>
#include <GL/glut.h>
#include <math.h>
#include "global.h"
#include "sphere.h"
#include <algorithm>

using namespace std;

//
// Global variables
//
extern int win_width;
extern int win_height;

extern GLfloat frame[WIN_HEIGHT][WIN_WIDTH][3];  

extern float image_width;
extern float image_height;

extern Point eye_pos;
extern float image_plane;
extern RGB_float background_clr;
extern RGB_float null_clr;

extern Spheres *scene;
extern board Board;

// light 1 position and color
extern Point light1;
extern float light1_ambient[3];
extern float light1_diffuse[3];
extern float light1_specular[3];

// global ambient term
extern float global_ambient[3];

// light decay parameters
extern float decay_a;
extern float decay_b;
extern float decay_c;

extern int shadow_on;
extern int reflection_on;
extern int refraction_on;
extern int chess_board_on;
extern int stochastic_on;
extern int step_max;
extern int supersampling_on;


/////////////////////////////////////////////////////////////////////

/*********************************************************************
 * Phong illumination - you need to implement this!
 *********************************************************************/
RGB_float phong(Point q, Vector ray, Vector surf_norm, Spheres *sph, int sphere_index, bool inside_sphere) {
  	RGB_float color;

    if (sph->index == 0) {
        int board_color = intersect_board(q);
        for (int i = 0; i < 3; i++) {
            sph->mat_ambient[i] = Board.mat_ambient[board_color][i];
            sph->mat_diffuse[i] = Board.mat_diffuse[board_color][i];
            sph->mat_specular[i] = Board.mat_specular[board_color][i];
        }
    }

    Vector o; // the origin vector
    o.x = o.y = o.z = 0.0;
    float illumination[3];
    Vector l = get_vec(q, light1); // vector from hit point to light source
    normalize(&l);
    Vector r = vec_minus(vec_scale(surf_norm, 2.0*vec_dot(surf_norm, l)), l); // vector for reflection
    normalize(&r);
    Vector v = vec_minus(o, ray); // vector from hit point to viewpoint
    normalize(&v);
    // calculate whether the point is in shadow
    bool in_shadow = false;
    Point hit_other;
    if (shadow_on) {
        Spheres *intersect_sph = intersect_scene(q, l, scene, &hit_other, sphere_index);
        in_shadow = intersect_sph != NULL || inside_sphere;
    }

    //calculate r, g, b value
    for (int i = 0; i < 3; i++) {
        //global ambient
        float clr_ga = global_ambient[i] * sph->mat_ambient[i];
        //local ambient
        float clr_la = light1_ambient[i] * sph->mat_ambient[i];
        //diffuse
        float clr_d;
        if (vec_dot(surf_norm, l) < 0 || in_shadow) {
            clr_d = 0;
        } else {
            clr_d = light1_diffuse[i] * sph->mat_diffuse[i] * vec_dot(surf_norm, l);
        }
        //specular
        float clr_s;
        if (vec_dot(surf_norm, l) < 0 || vec_dot(v, r) < 0 || in_shadow) {
            clr_s = 0;
        } else {
            clr_s = light1_specular[i] * sph->mat_specular[i] * pow(vec_dot(v, r), sph->mat_shineness);
        }
        //coefficient
        float dis = vec_len(get_vec(q, light1));
        float coefficient = min(1.0, (1.0 / (decay_a + decay_b*dis + decay_c*dis*dis)));
        illumination[i] = clr_ga + clr_la + coefficient * (clr_d + clr_s);
    }

    color.r = illumination[0];
    color.g = illumination[1];
    color.b = illumination[2];

  	return color;
}

/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(Point pos, Vector ray, int step, int sphere_index) {
    RGB_float color;
    Point hit;
    // get the closest intersect sphere
    Spheres* intersect_sph = intersect_scene(pos, ray, scene, &hit, sphere_index);

    if (intersect_sph != NULL) {
        // calculate surface normal
        Vector surf_norm;
        if (intersect_sph->index == 0) {
            //intersect with board
            surf_norm = Board.normal;
        } else{
            surf_norm = sphere_normal(hit, intersect_sph);
        }
        
        // Phong illumination calculation
        Vector e = get_vec(hit, pos); // vector from hit point to eye position
        normalize(&e);      
        bool inside_sphere = vec_dot(surf_norm, e) < 0; // whether the ray is inside a sphere
        Vector n = inside_sphere ? vec_scale(surf_norm, -1.0) : surf_norm;

        color = phong(hit, ray, n, intersect_sph, intersect_sph->index, inside_sphere);

        if (step == 0) {
            return color;
        } else {
            // calculate illumination from reflection
            if (reflection_on) {
                int skip_index = (inside_sphere) ? -1 : intersect_sph->index;
                Vector reflection_ray = vec_minus(vec_scale(n, 2.0*vec_dot(n, e)), e);
                normalize(&reflection_ray);
                RGB_float color_reflection = recursive_ray_trace(hit, reflection_ray, step-1, skip_index);
                color = clr_add (color, clr_scale(color_reflection, intersect_sph->reflectance));
            }

            // calculate illumination from refraction
            if (refraction_on && intersect_sph->index != 0) {
                int skip_index = (inside_sphere) ? intersect_sph->index : -1;
                float refr_co = (inside_sphere) 
                              ? 1.0 / intersect_sph->mat_refraction 
                              : intersect_sph->mat_refraction;
                
                normalize(&ray);
                float sini = vec_len(vec_cross(e, n));
                float norm_co = - sqrt(1-pow(refr_co * sini, 2)) - vec_dot(ray, n) * refr_co;
                Vector refraction_ray = vec_plus(vec_scale(ray, refr_co), vec_scale(n, norm_co));

                normalize(&refraction_ray);
                RGB_float color_refraction = recursive_ray_trace(hit, refraction_ray, step-1, skip_index);
                color = clr_add (color, clr_scale(color_refraction, intersect_sph->refraction));
            }

            // calculate illumination from diffuse reflections using stochastic ray
            if (stochastic_on) {
                for (int i = 0; i < STOCHA_RAY_NUM; i++) {
                    Vector ran_ray = vec_scale(surf_norm, -1.0);
                    while (vec_dot(ran_ray, surf_norm) <=0 ) {
                        ran_ray = {(rand() % 10) * pow(-1, rand() % 2),
                                   (rand() % 10) * pow(-1, rand() % 2), 
                                   (rand() % 10) * pow(-1, rand() % 2)};
                        normalize(&ran_ray);
                    }
                    int skip_index = (inside_sphere) ? -1 : intersect_sph->index;
                    RGB_float color_stoch = recursive_ray_trace(hit, ran_ray, step-1, skip_index);
                    color = clr_add (color, clr_scale(color_stoch, intersect_sph->reflectance/STOCHA_RAY_NUM));
                }
            }
            return color;
        }
    } else {
        return background_clr;
    }
}

/*********************************************************************
 * This function traverses all the pixels and cast rays. It calls the
 * recursive ray tracer and assign return color to frame
 *
 * You should not need to change it except for the call to the recursive
 * ray tracer. Feel free to change other parts of the function however,
 * if you must.
 *********************************************************************/
void ray_trace() {
    int i, j;
    float x_grid_size = image_width / float(win_width);
    float y_grid_size = image_height / float(win_height);
    float x_start = -0.5 * image_width;
    float y_start = -0.5 * image_height;
    RGB_float ret_color;
    Point cur_pixel_pos;
    Vector ray;

    // ray is cast through center of pixel
    cur_pixel_pos.x = x_start + 0.5 * x_grid_size;
    cur_pixel_pos.y = y_start + 0.5 * y_grid_size;
    cur_pixel_pos.z = image_plane;

    for (i=0; i<win_height; i++) {
        for (j=0; j<win_width; j++) {
            ray = get_vec(eye_pos, cur_pixel_pos);

            ret_color = recursive_ray_trace(eye_pos, ray, step_max, -1);
            
            // calculate illumination from supersampling
            if (supersampling_on) {
                Point sample_points[4] = {
                    {cur_pixel_pos.x + 0.25*x_grid_size, cur_pixel_pos.y + 0.25*y_grid_size, image_plane},
                    {cur_pixel_pos.x + 0.25*x_grid_size, cur_pixel_pos.y - 0.25*y_grid_size, image_plane},
                    {cur_pixel_pos.x - 0.25*x_grid_size, cur_pixel_pos.y + 0.25*y_grid_size, image_plane},
                    {cur_pixel_pos.x - 0.25*x_grid_size, cur_pixel_pos.y - 0.25*y_grid_size, image_plane}
                };

                for (int i = 0; i < 4; i++) {
                    ray = get_vec(eye_pos, sample_points[i]);
                    ret_color = clr_add(ret_color, recursive_ray_trace(eye_pos, ray, step_max, -1));
                }
            }

            frame[i][j][0] = GLfloat(ret_color.r);
            frame[i][j][1] = GLfloat(ret_color.g);
            frame[i][j][2] = GLfloat(ret_color.b);

            cur_pixel_pos.x += x_grid_size;
        }
        cur_pixel_pos.y += y_grid_size;
        cur_pixel_pos.x = x_start;
    }
}
