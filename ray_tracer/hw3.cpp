/* **************************
 * CSCI 420
 * Assignment 3 Raytracer
 * Name: <Your name here>
 * *************************
*/

#ifdef WIN32
  #include <windows.h>
#endif

#if defined(WIN32) || defined(linux)
  #include <GL/gl.h>
  #include <GL/glut.h>
#elif defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
  #define strcasecmp _stricmp
#endif

#include <imageIO.h>
#include <cmath>

#include "ray.h"

#define MAX_TRIANGLES 20000
#define MAX_SPHERES 100
#define MAX_LIGHTS 100

char * filename = NULL;

// The different display modes.
#define MODE_DISPLAY 1
#define MODE_JPEG 2

int mode = MODE_DISPLAY;

// While solving the homework, it is useful to make the below values smaller for debugging purposes.
// The still images that you need to submit with the homework should be at the below resolution (640x480).
// However, for your own purposes, after you have solved the homework, you can increase those values to obtain higher-resolution images.
#define WIDTH 640
#define HEIGHT 480

// The field of view of the camera, in degrees.
#define fov 60.0

vector3 cameraPos = vector3(0, 0, 0);
float focal_length = 1;
float viewport_height = 2 * sqrt(3)/3;
float viewport_width = 8 * sqrt(3) / 9;
vector3 pixel_delta_u = vector3(viewport_width / WIDTH,0,0);
vector3 pixel_delta_v = vector3(0, viewport_height / HEIGHT, 0);
vector3 viewport_bottom_left = cameraPos - vector3(0, viewport_height / 2, 0) - vector3(viewport_width / 2, 0, 0) - vector3(0,0, focal_length);
vector3 pixel00_pos = viewport_bottom_left + (pixel_delta_u + pixel_delta_v) * 0.5;

// Buffer to store the image when saving it to a JPEG.
unsigned char buffer[HEIGHT][WIDTH][3];

struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

struct Triangle
{
  Vertex v[3];
};

struct Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
};
struct TriangleLightingInfo
{
    int triangleIndex;
    float alpha, beta, gamma;
};
struct hitGeo
{
    int sphereIndex;
    TriangleLightingInfo t;
};
struct LightingParameters
{
    int LightIndex;
    point3 p;
    vector3 normal;
    bool sphere;
    hitGeo hitRecord;
};
struct Light
{
  double position[3];
  double color[3];
};

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void clampColor(point3& color)
{
    color.x = min(1.0, color.x);
    color.y = min(1.0, color.y);
    color.z = min(1.0, color.z);
}
point3 Illumination(LightingParameters parameters)
{
    float alpha = parameters.hitRecord.t.alpha;
    float beta = parameters.hitRecord.t.beta;
    float gamma = parameters.hitRecord.t.gamma;
    point3 I_total = point3();
    point3 I_ambient = point3(ambient_light);
    point3 I_diffuse, I_specular;
    vector3 L = point3(lights[parameters.LightIndex].position) - parameters.p;
    L.normalize();
    if(parameters.sphere)
        I_diffuse = (point3(spheres[parameters.hitRecord.sphereIndex].color_diffuse) * (max(0, L.dot(parameters.normal))));
    else
    {
        point3 averageDiffuse = point3(triangles[parameters.hitRecord.t.triangleIndex].v[0].color_diffuse) /** alpha 
            + point3(triangles[parameters.hitRecord.t.triangleIndex].v[1].color_diffuse) * beta + 
            point3(triangles[parameters.hitRecord.t.triangleIndex].v[2].color_diffuse) * gamma*/;
        I_diffuse = (point3(triangles[parameters.hitRecord.t.triangleIndex].v[0].color_diffuse) * (max(0, L.dot(parameters.normal)))) * alpha
            + (point3(triangles[parameters.hitRecord.t.triangleIndex].v[1].color_diffuse) * (max(0, L.dot(parameters.normal)))) * beta
            + (point3(triangles[parameters.hitRecord.t.triangleIndex].v[2].color_diffuse) * (max(0, L.dot(parameters.normal)))) * gamma;
    }
    vector3 v = cameraPos - parameters.p;
    v.normalize();
    vector3 I = point3(lights[parameters.LightIndex].position) - parameters.p;
    I.normalize();
    vector3 r = parameters.normal * I.dot(parameters.normal) * 2 - I;
    r.normalize();
    float cos_Phi = max(v.dot(r),0);
    if (parameters.sphere)
        I_specular = point3(spheres[parameters.hitRecord.sphereIndex].color_specular) * pow(cos_Phi, spheres[parameters.hitRecord.sphereIndex].shininess);
    else
    {
        I_specular = (point3(triangles[parameters.hitRecord.t.triangleIndex].v[0].color_specular)
            * pow(cos_Phi, triangles[parameters.hitRecord.t.triangleIndex].v[0].shininess)) * alpha
            + (point3(triangles[parameters.hitRecord.t.triangleIndex].v[1].color_specular)
                * pow(cos_Phi, triangles[parameters.hitRecord.t.triangleIndex].v[1].shininess)) * beta
            + (point3(triangles[parameters.hitRecord.t.triangleIndex].v[2].color_specular)
                * pow(cos_Phi, triangles[parameters.hitRecord.t.triangleIndex].v[2].shininess)) * gamma;
    }
    I_total = (I_diffuse + I_specular ).multiplication(lights[parameters.LightIndex].color);
    clampColor(I_total);
    return I_total;
}
bool hitSpheres(point3 origin, vector3 direction, float& t_min, bool isShadowRay, LightingParameters& parameters)
{
    bool res = false;
    for (int i = 0; i < num_spheres; i++)
    {
        if ((isShadowRay && i != parameters.hitRecord.sphereIndex) || !isShadowRay)
        {
            vector3 oc = vector3(spheres[i].position) - origin;
            float catheti = oc.dot(direction);
            float d_2 = oc.len_2() - catheti * catheti;
            float t = catheti - sqrt(spheres[i].radius * spheres[i].radius - d_2);
            point3 p = origin + direction * t;
            if (d_2 <= spheres[i].radius * spheres[i].radius && t > 0)
            {
                if (t < t_min && t > 0)
                {
                    if (!isShadowRay)
                    {
                        parameters.normal = p - point3(spheres[i].position);
                        parameters.normal.normalize();
                        //color = Illumination(parameters.normal, lights[parameters.LightIndex], p, spheres[i], triangles[0], true);
                        t_min = t;
                        parameters.p = p;
                        parameters.hitRecord.sphereIndex = i;
                        parameters.hitRecord.t.triangleIndex = -1;
                        parameters.sphere = true;
                        res = true;
                    }
                    else
                        return true;
                }
            }
        }
    }
    return res;
}
bool hitTriangles(point3 origin, vector3 direction, float& t_min, bool isShadowRay,LightingParameters& parameters)
{
    bool res = false;
    for (int i = 0; i < num_triangles; i++)
    {
        if ((isShadowRay && i != parameters.hitRecord.t.triangleIndex) || !isShadowRay)
        {
            vector3 v0_v1 = point3(triangles[i].v[1].position) - point3(triangles[i].v[0].position);
            vector3 v0_v2 = point3(triangles[i].v[2].position) - point3(triangles[i].v[0].position);
            vector3 n = v0_v1.cross(v0_v2);
            float d = -n.dot(point3(triangles[i].v[0].position));
            float t = -(d + n.dot(origin)) / (n.dot(direction));
            point3 p = origin + direction * t;
          
            float s = n.len_2();
            vector3 p_v0 = point3(triangles[i].v[0].position) - p;
            vector3 p_v1 = point3(triangles[i].v[1].position) - p;
            vector3 p_v2 = point3(triangles[i].v[2].position) - p;

            float s1 = (p_v1.cross(p_v2)).dot(n);
            float s2 = (p_v2.cross(p_v0)).dot(n);
            float s3 = (p_v0.cross(p_v1)).dot(n);
            float a = s1/s, b = s2/s, c = 1 - a - b;
            if (a >= 0 && b >= 0 && c >= 0)
            {
                if (t < t_min && t>0)
                {
                    if (!isShadowRay)
                    {
                        parameters.normal = point3(triangles[i].v[0].normal)*a + point3(triangles[i].v[1].normal)*b + point3(triangles[i].v[2].normal)*c;
                        parameters.normal.normalize();
                        parameters.p = p;
                        parameters.hitRecord.t.alpha = a;
                        parameters.hitRecord.t.beta = b;
                        parameters.hitRecord.t.gamma = c;
                        //color = Illumination(parameters.normal, lights[parameters.LightIndex], p, spheres[0], triangles[i], false, a, b, c);
                        t_min = t;
                        parameters.hitRecord.t.triangleIndex = i;
                        parameters.hitRecord.sphereIndex = -1;
                        parameters.sphere = false;
                        res = true;
                    }
                    else
                        return true;
                        
                }
            }
        }
    }
    return res;
}
void draw_scene()
{
  for(unsigned int x=0; x<WIDTH; x++)
  {
    glPointSize(2.0);  
    // Do not worry about this usage of OpenGL. This is here just so that we can draw the pixels to the screen,
    // after their R,G,B colors were determined by the ray tracer.
    glBegin(GL_POINTS);
    for(unsigned int y=0; y<HEIGHT; y++)
    {
      // A simple R,G,B output for testing purposes.
      // Modify these R,G,B colors to the values computed by your ray tracer.
        vector3 dir = pixel00_pos + pixel_delta_u * x + pixel_delta_v * y - cameraPos;
        dir.normalize();
        ray r = ray(cameraPos, dir);
        LightingParameters parameters;
        point3 color = point3(ambient_light);
        float t_min = INT_MAX;
        //check for sphere
        hitSpheres(cameraPos, r.direction, t_min, false, parameters);
        //check for triangles
        hitTriangles(cameraPos, r.direction, t_min, false, parameters);
        //check if ray is in shadow
        if (t_min != INT_MAX)
        {
            point3 p = cameraPos + r.direction * t_min;
            point3 color_Per_light;
            for (int i = 0; i < num_lights; i++)
            {
                vector3 shadowRayDir = point3(lights[i].position) - p;
                shadowRayDir.normalize();
                parameters.LightIndex = i;
                ray shadowCheckRay = ray(p, shadowRayDir);
                //do Phong illumination
                color_Per_light = Illumination(parameters);
                //if ray is block by sphere or triangles then it becomes black
                if (hitSpheres(p, shadowRayDir, t_min, true, parameters) || hitTriangles(p, shadowRayDir, t_min, true, parameters))
                {
                    color_Per_light = color_Per_light * 0.1 * 0.1 + point3(0.1,0.1,0.1)*0.9;
                }
                color = color + color_Per_light;
            }
            //color = color * (1.0 / num_lights);
        }
        else
            color = point3(1, 1, 1);
        clampColor(color);
        color = (color) * 255.0f;
      plot_pixel(x, y, color.x, color.y, color.z);
    }
    glEnd();
    glFlush();
  }
  printf("Ray tracing completed.\n"); 
  fflush(stdout);
}

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  glColor3f(((float)r) / 255.0f, ((float)g) / 255.0f, ((float)b) / 255.0f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  buffer[y][x][0] = r;
  buffer[y][x][1] = g;
  buffer[y][x][2] = b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
    plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  printf("Saving JPEG file: %s\n", filename);

  ImageIO img(WIDTH, HEIGHT, 3, &buffer[0][0][0]);
  if (img.save(filename, ImageIO::FORMAT_JPEG) != ImageIO::OK)
    printf("Error in saving\n");
  else 
    printf("File saved successfully\n");
}

void parse_check(const char *expected, char *found)
{
  if(strcasecmp(expected,found))
  {
    printf("Expected '%s ' found '%s '\n", expected, found);
    printf("Parsing error; abnormal program abortion.\n");
    exit(0);
  }
}

void parse_doubles(FILE* file, const char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE *file, double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE *file, double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE * file = fopen(argv,"r");
  if (!file)
  {
    printf("Unable to open input file %s. Program exiting.\n", argv);
    exit(0);
  }

  int number_of_objects;
  char type[50];
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i", &number_of_objects);

  printf("number of objects: %i\n",number_of_objects);

  parse_doubles(file,"amb:",ambient_light);

  for(int i=0; i<number_of_objects; i++)
  {
    fscanf(file,"%s\n",type);
    printf("%s\n",type);
    if(strcasecmp(type,"triangle")==0)
    {
      printf("found triangle\n");
      for(int j=0;j < 3;j++)
      {
        parse_doubles(file,"pos:",t.v[j].position);
        parse_doubles(file,"nor:",t.v[j].normal);
        parse_doubles(file,"dif:",t.v[j].color_diffuse);
        parse_doubles(file,"spe:",t.v[j].color_specular);
        parse_shi(file,&t.v[j].shininess);
      }

      if(num_triangles == MAX_TRIANGLES)
      {
        printf("too many triangles, you should increase MAX_TRIANGLES!\n");
        exit(0);
      }
      triangles[num_triangles++] = t;
    }
    else if(strcasecmp(type,"sphere")==0)
    {
      printf("found sphere\n");

      parse_doubles(file,"pos:",s.position);
      parse_rad(file,&s.radius);
      parse_doubles(file,"dif:",s.color_diffuse);
      parse_doubles(file,"spe:",s.color_specular);
      parse_shi(file,&s.shininess);

      if(num_spheres == MAX_SPHERES)
      {
        printf("too many spheres, you should increase MAX_SPHERES!\n");
        exit(0);
      }
      spheres[num_spheres++] = s;
    }
    else if(strcasecmp(type,"light")==0)
    {
      printf("found light\n");
      parse_doubles(file,"pos:",l.position);
      parse_doubles(file,"col:",l.color);

      if(num_lights == MAX_LIGHTS)
      {
        printf("too many lights, you should increase MAX_LIGHTS!\n");
        exit(0);
      }
      lights[num_lights++] = l;
    }
    else
    {
      printf("unknown type in scene description:\n%s\n",type);
      exit(0);
    }
  }
  return 0;
}

void display()
{
}

void init()
{
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
  // Hack to make it only draw once.
  static int once=0;
  if(!once)
  {
    draw_scene();
    if(mode == MODE_JPEG)
      save_jpg();
  }
  once=1;
}

int main(int argc, char ** argv)
{
  if ((argc < 2) || (argc > 3))
  {  
    printf ("Usage: %s <input scenefile> [output jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
  {
    mode = MODE_JPEG;
    filename = argv[2];
  }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  #ifdef __APPLE__
    // This is needed on recent Mac OS X versions to correctly display the window.
    glutReshapeWindow(WIDTH - 1, HEIGHT - 1);
  #endif
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}

