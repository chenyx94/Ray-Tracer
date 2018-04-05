/**********************************************************************
 * Some stuff to handle spheres
 **********************************************************************/
#include "vector.h"

typedef struct sphere {
    int index;               // identifies a sphere; must be greater than 0

    Point center;
    float radius;

    float mat_ambient[3];    // material property used in Phong model
    float mat_diffuse[3];
    float mat_specular[3];
    float mat_shineness;

    float reflectance;       // this number [0,1] determines how much 
                             // reflected light contributes to the color
                             // of a pixel

    float mat_refraction; // the ratio of the indices of refraction between sphere and air
    float refraction; // this number [0,1] determines how much 
                            // refraction light contributes to the color
                            // of a pixel
    struct sphere *next;
} Spheres;   // a list of spheres

// intersect ray with sphere
Spheres *intersect_scene(Point, Vector, Spheres *, Point *, int);
// return the unit normal at a point on sphere
Vector sphere_normal(Point, Spheres *);
// add a sphere to the sphere list
Spheres *add_sphere(Spheres *, Point, float, float [], float [], float [], float, float, int);

#define BOARD_BLACK 0
#define BOARD_WHITE 1
#define BOARD_OUTSIDE 2

struct board {
    Point center;
    Vector normal;
    float grid_size;
    Vector x_axis;
    Vector y_axis;

    float mat_ambient[2][3];    // material property used in Phong model
    float mat_diffuse[2][3];
    float mat_specular[2][3];
    float mat_shineness;

    float reflectance;       // this number [0,1] determines how much 
                             // reflected light contributes to the color
                             // of a pixel    
};

int intersect_board(Point hit);