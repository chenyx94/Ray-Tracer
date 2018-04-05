//
// this provide functions to set up the scene
//
#include "sphere.h"
#include <stdio.h>
#include <iostream>

extern Point light1;
extern float light1_ambient[3];
extern float light1_diffuse[3];
extern float light1_specular[3];

extern float global_ambient[3];
extern Spheres *scene;
extern board Board;
extern int chess_board_on;
extern int refraction_on;

extern RGB_float background_clr;
extern float decay_a;
extern float decay_b;
extern float decay_c;

//////////////////////////////////////////////////////////////////////////

/*******************************************
 * set up the default scene - DO NOT CHANGE
 *******************************************/
void set_up_default_scene() {
    // set background color
    background_clr.r = 0.5;
    background_clr.g = 0.05;
    background_clr.b = 0.8;

    // setup global ambient term
    global_ambient[0] = global_ambient[1] = global_ambient[2] = 0.2;

    // setup light 1
    light1.x = -2.0;
    light1.y = 5.0;
    light1.z = 1.0;
    light1_ambient[0] = light1_ambient[1] = light1_ambient[2] = 0.1;
    light1_diffuse[0] = light1_diffuse[1] = light1_diffuse[2] = 1.0;
    light1_specular[0] = light1_specular[1] = light1_specular[2] = 1.0;

    // set up decay parameters
    decay_a = 0.5;
    decay_b = 0.3;
    decay_c = 0.0;

    // sphere 1 blue
    Point sphere1_ctr = {1.5, -0.2, -3.2};
    // Point sphere1_ctr = {-0.5, -0.2, -6};
    float sphere1_rad = 1.23;
    float sphere1_ambient[] = {0.7, 0.7, 0.7};
    float sphere1_diffuse[] = {0.1, 0.5, 0.8};
    float sphere1_specular[] = {1.0, 1.0, 1.0};
    float sphere1_shineness = 10;
    float sphere1_reflectance = 0.4;
    scene = add_sphere(scene, sphere1_ctr, sphere1_rad, sphere1_ambient,
            sphere1_diffuse, sphere1_specular, sphere1_shineness,
  		    sphere1_reflectance, 1);

    // sphere 2 red
    Point sphere2_ctr = {-1.5, 0.0, -3.5};
    float sphere2_rad = 1.5;
    float sphere2_ambient[] = {0.6, 0.6, 0.6};
    float sphere2_diffuse[] = {1.0, 0.0, 0.25};
    float sphere2_specular[] = {1.0, 1.0, 1.0};
    float sphere2_shineness = 6;
    float sphere2_reflectance = 0.3;
    scene = add_sphere(scene, sphere2_ctr, sphere2_rad, sphere2_ambient,
            sphere2_diffuse, sphere2_specular, sphere2_shineness,
  		    sphere2_reflectance, 2);

    // sphere 3 green
    Point sphere3_ctr = {-0.35, 1.75, -2.25};
    float sphere3_rad = 0.5;
    float sphere3_ambient[] = {0.2, 0.2, 0.2};
    float sphere3_diffuse[] = {0.0, 1.0, 0.25};
    float sphere3_specular[] = {0.0, 1.0, 0.0};
    float sphere3_shineness = 30;
    float sphere3_reflectance = 0.3;
    scene = add_sphere(scene, sphere3_ctr, sphere3_rad, sphere3_ambient,
            sphere3_diffuse, sphere3_specular, sphere3_shineness,
  		    sphere3_reflectance, 3);
}

/***************************************
 * You can create your own scene here
 ***************************************/
void add_chess_board() {
    Point board_center = {0, -2.5, -5};
    Board.center = board_center;
    Vector board_normal = {0, 1, 0};
    Board.normal = board_normal;
    Board.grid_size = 2;
    Board.x_axis = {1, 0, 0};
    Board.y_axis = {0, 0, -1};

    float board_ambient[][3] = {{1.0, 1.0, 1.0}, {0.0, 0.0, 0.0}};
    float board_diffuse[][3] = {{1.0, 1.0, 1.0}, {0.0, 0.0, 0.0}};
    float board_specular[][3] = {{1.0, 1.0, 1.0}, {0.0, 0.0, 0.0}};

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j< 3; j++) {
            Board.mat_ambient[i][j] = board_ambient[i][j];
            Board.mat_diffuse[i][j] = board_diffuse[i][j];
            Board.mat_specular[i][j] = board_specular[i][j];
        }
    }

    Board.mat_shineness = 6;
    Board.reflectance = 0.3;

    // add the board to the list of spheres as a fake sphere with the index of 0 
    // to make it be traversed during intersection
    float board_radius = 0.5;
    scene = add_sphere(scene, board_center, board_radius, board_ambient[0], 
            board_diffuse[0], board_specular[0], Board.mat_shineness,
            Board.reflectance, 0);
}

void set_up_user_scene() {
    set_up_default_scene();
    if (refraction_on) {
        Spheres* sph = scene;
        while (sph != NULL) {
            sph->mat_refraction = 0.667;
            sph->refraction = 0.4;
            sph = sph->next;
        }
    }
}
