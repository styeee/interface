#include <stdio.h>
//#include <conio.h>
using namespace std;
#include <functional>
#include <bitset>
#include <thread>
#include <windows.h>
#include "GL/glut.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define CAPACITY 1024//max number of panels

//mouse
int X=0;
int Y=0;
int W=800;
int H=800;

#define IS_CLK 0        //it is inverted
#define IS_NODRAW 1
#define IS_NOHOVER 2
#define IS_RELEASE 3
#define IS_HOVER0 4      //it is puls when hovered
#define IS_HOVER1 5      //it mem previous
#define IS_HOVER(s) s[IS_HOVER0]^s[IS_HOVER1]

    size_t ps=0;   //count of panels
    size_t pc=(~0);//current hover panel
class panel
{
    //start point of square
    float x;
    float y;
    //end point of square
    float xx;
    float yy;
    //panel info

    //some animations
    void simple(float r,float g,float b)
    {
         glColor4f(r,g,b,.5);
        glBegin(GL_QUADS);
            glVertex2f(x,y);
            glVertex2f(x,yy);
            glVertex2f(xx,yy);
            glVertex2f(xx,y);
        glEnd();
    }
public:
    float color=1;
    function<void(size_t,bitset<32>&)>f=0;//hover heandler

    //general info about panel
    size_t id=0;
    size_t parent=0;
    bitset<32>state=1;
    const char*texture=0;

    //constructors
    panel(float _x,float _y,float _w,float _h)
    {x=_x;y=_y;xx=_x+_w;yy=_y+_h;id=ps;ps++;}

    void draw()
    {
            if(texture)
        {glBindTexture(GL_TEXTURE_2D,id+1);glColor4f(1,1,1,1);}
            else
         glColor4f(color,color,color,1);

        if(IS_HOVER(state))
        {
            if(state[IS_CLK])
                glColor3f(0,0,color);
            else
                glColor3f(0,color,0);
        }

        glBegin(GL_QUADS);
            glTexCoord2f(0,0); 
                              glVertex2f(x,y);
            glTexCoord2f(1,0);
                              glVertex2f(xx,y);
            glTexCoord2f(1,1);
                              glVertex2f(xx,yy);
            glTexCoord2f(0,1);
                              glVertex2f(x,yy);
        glEnd();
        
        if(texture)glBindTexture(GL_TEXTURE_2D,0);
    }

    //call it when need check hover
    bool test(float a,float b)
    {
        state[IS_HOVER1]=state[IS_HOVER0];

        if(a<x)return false;
        if(a>xx)return false;
        if(b<y)return false;
        if(b>yy)return false;

        return true;
    }

    //call it when hover is on
    void hover()
    {
        state[IS_HOVER0]=!state[IS_HOVER0];
        
        if(f)f(id,state);
        /*
        if(state[IS_CLK])
            simple(0,0,color);
        else
            simple(0,color,0);
        */
    }

    //call it when clk was been active
    void click(int b,int s)
    {
        //thread th([](bool b){if(b)Beep(1000,5);else Beep(5000,5);},s);

        state[IS_CLK]=s;
    }
};
panel*panels[CAPACITY];
//std::fill(panels,panels+,0);

void text(const size_t a,const size_t b,const size_t s,const char c)
{
    const size_t aa=a+s;
    const size_t bb=b+s;

    const float x=(c%16)*.0625;
    const float y=(c/16)*.0625;

        glBindTexture(GL_TEXTURE_2D,CAPACITY+1);
    glBegin(GL_QUADS);
        glTexCoord2f(x,y);
            glVertex2f(a,b);
        glTexCoord2f(x+.0625,y);
            glVertex2f(aa,b);
        glTexCoord2f(x+.0625,y+.0625);
            glVertex2f(aa,bb);
        glTexCoord2f(x,y+.0625);
            glVertex2f(a,bb);
    glEnd();
}
void text(const size_t a,const size_t b,const size_t s,const char*t)
{
    size_t i=0;
    while(*t){text(a+(s*i>>1),b,s,*t);t++;i++;}
}
void text(const size_t a,const size_t b,const size_t s,int n)
{
    char t[12]="";
    sprintf_s<12>(t,"%d",n);
    text(a,b,s,t);
}
void text(const size_t a,const size_t b,const size_t s,float n)
{
    char t[12]="";
    sprintf_s<12>(t,"%f",n);
    text(a,b,s,t);
}

int num;

void display()
{
    //if(panels[pc]);

    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1,1,0);
    text(100,200,32,num);

    for(size_t i=0;i<ps;i++)
    {
        if(panels[i]->state[IS_NODRAW])continue;
        panels[i]->draw();
    }

    for(size_t i=0;i<ps;i++)
    {
        if(panels[i]->state[IS_NOHOVER])continue;
        if(panels[i]->test(X,Y))
        {
            pc=panels[i]->id;
            panels[i]->hover();
            goto pass;
        }
    }
    pc=(~0);
    pass:

    if(panels[0]->state[0])

    glutSwapBuffers();
}

void reshape(int width,int height)
{
    glViewport(0,0,(GLsizei)width,(GLsizei)height); // Set our viewport to the size of our window  
    glMatrixMode(GL_PROJECTION); // Switch to the projection matrix so that we can manipulate how our scene is viewed  
    glLoadIdentity(); // Reset the projection matrix to the identity matrix so that we don't get any artifacts (cleaning up)  
    gluPerspective(60,(GLfloat)width/(GLfloat)height,1.0,100.0); // Set the Field of view angle (in degrees), the aspect ratio of our window, and the new and far planes  
    glMatrixMode(GL_MODELVIEW); // Switch back to the model view matrix, so that we can start drawing shapes correctly  
}

#define change(fun) if(s[IS_RELEASE]&&(!s[IS_CLK])){fun};s[IS_RELEASE]=s[IS_CLK];
//#define stop(fun)
#define panel_create(a,b,c,d) panels[ps-1]=new panel(a,b,c,d);
#define panel_clk(fun) panels[ps-1]->f=[](size_t id,bitset<32>&s){fun};
#define panel_tex(a) panels[ps-1]->texture=a;

/*
class numpad
{
    numpad(size_t x,size_t y,size_t w,size_t h)
    {
        panel*const p=new panel(x,y,w,h);
        panels[ps-1]=p;
    }
};
*/

class grid
{
    size_t id;
    size_t count;

    const size_t s=5;
public:
    const size_t get_id(){return id;}
    const size_t get_count(){return count;}

    grid(size_t x,const size_t y,const size_t w,const size_t h,const size_t n)
    {
        const size_t d=w/n;

        id=ps;
        count=n;

        for(char i=0;i<n;i++)
        {
            panel_create(x+i*d+s,y,d-s-s,h)
                //panel_tex("button.png")

                panel_clk
                (
                    change(float c=id/8.;glClearColor(c,c,c,0);)
                )
        }
    }

    panel*const operator[](size_t i)
    {
        if(i>=count)return 0;
        return panels[id+i];
    }

    void operator()(void(*f)(panel*,size_t i))
    {
        for(size_t i=id;i<=count;i++)f(panels[i],i);
    }
};

int main(int argc,char** argv)
{
    //main chunk <s:
    /*
    for(char i=0;i<9;i++)
    {
        panel_create(20+i*40,20,20,20)
        panel_tex("button.png")
        panel_clk
        (
            change(float c=id/8.;glClearColor(c,c,c,0);)
        )
    }
    */
    //panels[0]->state=7;

    panel_create(100,100,100,50);
    panel_tex("button.png")
    panel_clk(
        change(exit(1);)
        printf("%s\n",s.to_string().c_str());
    )

    grid g(20,20,400,20,14);
    g(
        [](panel*p,size_t da)
        {
            const float c=da/16.;
            p->color=c;
            
            p->f=[c](size_t i,bitset<32>&s){
                change(
                    num=i;
                    glClearColor(c,c,c,1);
                )
            };
        }
    );

    //for(size_t i=g.get_count();i--;)g[i].;

    //main chunk >e;

    /////////////////////

    //about window
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(W,H);
    glutInitWindowPosition(0,0);
    glutCreateWindow("Window");

    //gl init
    glClearColor(0,0,0,0);
    glColor4f(1,1,1,1);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.,double(W),double(H),0.,-1.,1.);
        //glOrtho(0.,1.,1.,0.,-1.,1.);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    //abaout textures
    size_t textures[CAPACITY+1];
    glGenTextures(CAPACITY+1,textures);
    for(size_t i=1;i<=ps;i++)
    {
        const char*name=panels[i-1]->texture;
        if(!name)continue;
        glBindTexture(GL_TEXTURE_2D,i);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        int w;
        int h;
        unsigned char*pixels=stbi_load(name,&w,&h,0,4);
        glBindTexture(GL_TEXTURE_2D,i);
        glTexImage2D(GL_TEXTURE_2D,0,4,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
        stbi_image_free(pixels);
    }

    glBindTexture(GL_TEXTURE_2D,CAPACITY+1);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    int w;
    int h;
    unsigned char*pixels=stbi_load("font.png",&w,&h,0,4);
    glBindTexture(GL_TEXTURE_2D,CAPACITY+1);
    glTexImage2D(GL_TEXTURE_2D,0,4,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    stbi_image_free(pixels);

    glBindTexture(GL_TEXTURE_2D,0);

    //about events
    glutIdleFunc(display);
    glutDisplayFunc(display);
    glutPassiveMotionFunc([](int x,int y){X=x;Y=y;});
    glutReshapeFunc([](int w,int h){W=w;H=h;});
    glutMouseFunc([](int b,int s,int x,int y)
    {
        if(pc!=(~0))panels[pc]->click(b,s);
    });

    glutMainLoop();

    return 0;
}