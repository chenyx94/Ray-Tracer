#include "sphere.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>

using namespace std;

extern board Board;

/**********************************************************************
 * This function intersects a ray with a given sphere 'sph'. You should
 * use the parametric representation of a line and do the intersection.
 * The function should return the parameter value for the intersection, 
 * which will be compared with others to determine which intersection
 * is closest. The value -1.0 is returned if there is no intersection
 *
 * If there is an intersection, the point of intersection should be
 * stored in the "hit" variable
 **********************************************************************/
int intersect_board(Point hit) {
    Vector hit_vec = get_vec(Board.center, hit);
    int x = (int)floor(vec_dot(hit_vec, Board.x_axis) / Board.grid_size);
    int y = (int)floor(vec_dot(hit_vec, Board.y_axis) / Board.grid_size);
    if (x >= 4 || x <= -5 || y >= 4 || y <= -5){
        return BOARD_OUTSIDE;
    } else if ((x+y) % 2 == 0) {
        return BOARD_WHITE;
    } else {
        return BOARD_BLACK;
    }
}

float intersect_sphere(Point o, Vector u, Spheres *sph, Point *hit) {
    float dis_intersect;
    normalize(&u);

    if (sph->index == 0) {
        // intersect with board
        Vector normal = Board.normal;
        if (vec_dot(normal, u) >= 0) {
            return -1;
        } else {
            dis_intersect = (Board.center.y-o.y) / u.y;
            Point hit_tmp = get_point(o, vec_scale(u, dis_intersect));
            if (intersect_board(hit_tmp) == BOARD_OUTSIDE){
                return -1;
            } else {
                *hit = hit_tmp;
                return dis_intersect;
            }
        }
    } else {
        // intersect with sphere
        // calculate the distance between the vector and the center of the sphere
        Vector oc = get_vec(o, sph->center);
        float dis_oc = vec_len(oc); // distance between point o and the center of the sphere
        normalize(&u);
        Vector dis_vec = vec_cross(oc, u);
        float dis_cu = vec_len(dis_vec);

        if (dis_cu > sph->radius || vec_dot(oc, u) < 0) {
            return -1.0;
        } else if (fabs(dis_oc - sph->radius) < 1.0e-6) {
            dis_intersect = 2 * sqrt(sph->radius*sph->radius - dis_cu*dis_cu);
            *hit = get_point(o, vec_scale(u, dis_intersect));
            return dis_intersect;
        } else {
            dis_intersect = sqrt(dis_oc*dis_oc - dis_cu*dis_cu) 
                            - sqrt(sph->radius*sph->radius - dis_cu*dis_cu);
            *hit = get_point(o, vec_scale(u, dis_intersect));  
            return dis_intersect;
        }
    }
}

/*********************************************************************
 * This function returns a pointer to the sphere object that the
 * ray intersects first; NULL if no intersection. You should decide
 * which arguments to use for the function. For exmaple, note that you
 * should return the point of intersection to the calling function.
 **********************************************************************/
Spheres *intersect_scene(Point o, Vector u, Spheres *sph, Point *hit, int i) { //int i => index of sphere to skip
    float hit_dis = 1e10;
    Spheres *sphere = NULL;
    Point hit_temp;

    while (sph != NULL) {
        if (sph->index != i){
            float hit_dis_temp = intersect_sphere(o, u, sph, &hit_temp);
            if (hit_dis_temp > 0 && hit_dis_temp < hit_dis) {
                hit_dis = hit_dis_temp;
                sphere = sph;
                *hit = hit_temp;
            }
        }
        sph = sph->next;
    }

  	return sphere;
}

/*****************************************************
 * This function adds a sphere into the sphere list
 *
 * You need not change this.
 *****************************************************/
Spheres *add_sphere(Spheres *slist, Point ctr, float rad, float amb[],
		    float dif[], float spe[], float shine, 
		    float refl, int sindex) {
    Spheres *new_sphere;

    new_sphere = (Spheres *)malloc(sizeof(Spheres));
    new_sphere->index = sindex;
    new_sphere->center = ctr;
    new_sphere->radius = rad;
    (new_sphere->mat_ambient)[0] = amb[0];
    (new_sphere->mat_ambient)[1] = amb[1];
    (new_sphere->mat_ambient)[2] = amb[2];
    (new_sphere->mat_diffuse)[0] = dif[0];
    (new_sphere->mat_diffuse)[1] = dif[1];
    (new_sphere->mat_diffuse)[2] = dif[2];
    (new_sphere->mat_specular)[0] = spe[0];
    (new_sphere->mat_specular)[1] = spe[1];
    (new_sphere->mat_specular)[2] = spe[2];
    new_sphere->mat_shineness = shine;
    new_sphere->reflectance = refl;
    new_sphere->next = NULL;

    if (slist == NULL) { // first object
        slist = new_sphere;
    } else { // insert at the beginning
        new_sphere->next = slist;
        slist = new_sphere;
    }

    return slist;
}

/******************************************
 * computes a sphere normal - done for you
 ******************************************/
Vector sphere_normal(Point q, Spheres *sph) {
    Vector rc;

    rc = get_vec(sph->center, q);
    normalize(&rc);
    return rc;
}
