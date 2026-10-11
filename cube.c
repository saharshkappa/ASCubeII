#include <stdio.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <time.h> 

#ifdef _WIN32
    #include <windows.h> // Sleep() on Windows
#else
    #include <unistd.h>  // usleep() on Linux/Mac or Unix     
#endif

#define WIDTH 60
#define HEIGHT 30
#define PI 3.141592
char Screen[WIDTH][HEIGHT];
float Zindex[WIDTH][HEIGHT];


float theta=0.01;  //Rotation angle (smoothness)
const float f=100; //FocalLength
float a=2.0;        //Scale of the cube
float yscalefactor=0.5; // Account for the terminal stretching vertically
typedef struct{
 int indices[3];
}Triangle;

typedef struct{
    float x;
    float y;
}Vector2;

typedef struct{
    float x;
    float y;
    float z;
}Vector3;

Vector3 camera={0.0,0.0,25.0};  //camera position vector (only affects translation not rotation)
Vector3 Light={-1,-1,-1};       //Light direction vector 

Vector3 vertices[8]={
        { 1.0f, -1.0f,  1.0f}, { 1.0f,  1.0f,  1.0f}, 
        { 1.0f, -1.0f, -1.0f}, { 1.0f,  1.0f, -1.0f},        
        {-1.0f, -1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f}, 
        {-1.0f, -1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f},
    };
//Faces
Triangle cubeT[12]={(Triangle){{2,1,0}},(Triangle){{1,2,3}},
                    (Triangle){{0,4,2}},(Triangle){{2,4,6}},
                    (Triangle){{6,3,2}},(Triangle){{3,6,7}},
                    (Triangle){{3,7,1}},(Triangle){{7,5,1}},
                    (Triangle){{7,6,5}},(Triangle){{4,5,6}},
                    (Triangle){{1,5,4}},(Triangle){{0,1,4}}};



//top-Down Definations  
Vector2 project(Vector3 point,Vector3 camera);//projects the 3d points onto a 2d screen
static void UpdateDrawFrame(void); //contains all the animation logic
void raturize(Vector3 A,Vector3 B,Vector3 C); // renders a facefrom 3 points(or a plane)
void Rotation(float rotationalMatrix[3][3],Vector3 *point); 
void superRotator(float super[3][3],float thetaX,float thetaY,float thetaZ);
void MatMult(float Mat1[3][3],float Mat2[3][3], float Mat3[3][3]);
float DotProduct(Vector3 a,Vector3 b);
Vector3 CrossProduct(Vector3 a,Vector3 b);
Vector3 VectorSub(Vector3 a,Vector3 b);

    

void DrawScreen(){
printf("\x1b[H");
for(int y=0;y<HEIGHT;y++){
    for(int x=0;x<WIDTH;x++)
        printf("%c",Screen[x][y]);
    printf("\n");
    }
}


void ClearScreen(){
    for(int y=0;y<HEIGHT;y++){
        for(int x=0;x<WIDTH;x++){
            Screen[x][y]=' ';
            // z index maxxing 
            Zindex[x][y]=FLT_MAX;}
    }
}

char Color(float L){
    char ramp[]="$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
    //char ramp[]="@%#*+=-:. ";
    //L=fabsf(L);
    if (L < 0.0f) L = 0.0f;
    if (L > 1.0f) L = 1.0f;
    float ambient=(sizeof(ramp)/sizeof(ramp[0])<20) ? 0.15:0.04;
    float finalLight=ambient+(L*(1.0f-ambient));
    int i=(sizeof(ramp)/sizeof(ramp[0])-1)-(int)(finalLight*((sizeof(ramp)/sizeof(ramp[0])-1)-1))-1;
    return ramp[i];
}

void delay_ms(int milliseconds) {
    #ifdef _WIN32
        Sleep(milliseconds); 
    #else
        usleep(milliseconds * 1000); // unix takes microseconds 
    #endif
}

int main(void){
    
    float modLight=sqrt(Light.x*Light.x+Light.y*Light.y+Light.z*Light.z);
    if(modLight<0.0001f) return -1;
    Light.x=Light.x/modLight;
    Light.y=Light.y/modLight;
    Light.z=Light.z/modLight;
    
    const int TARGET_FPS = 30;
    const double MS_PER_FRAME = 1000.0 / TARGET_FPS; // 16.6667 (67 haha) milliseconds
    
    
    while (1){
        clock_t frameStart=clock();

        ClearScreen();  
        UpdateDrawFrame();
        DrawScreen();

        clock_t frameEnd=clock();
        double timeTakenMs=((double)(frameEnd-frameStart)/CLOCKS_PER_SEC)*1000.0;
        double timeLeft = MS_PER_FRAME - timeTakenMs;
        if(timeLeft>0){
            delay_ms((int)timeLeft);
        }
    }

    return 0;
}


// Update and draw game frame
static void UpdateDrawFrame(void){
    static int axis=0;
    static float angle=0.0;
    static float angleXYZ[3] = {0.0,0.0,0.0};
    static float rotation=0.0;
    float super[3][3]={{1.0,0.0,0.0},
                       {0.0,1.0,0.0},
                       {0.0,0.0,1.0}};
    //rotaion and projection
    Vector3 transformedVertices[8];

    for(int i=0;i<8;i++){
        transformedVertices[i].x=vertices[i].x*a;
        transformedVertices[i].y=vertices[i].y*a;
        transformedVertices[i].z=vertices[i].z*a;

    }
    superRotator(super,angleXYZ[0],angleXYZ[1],angleXYZ[2]);
    Vector2 projected[8];
    for(int i=0;i<8;i++){
        Rotation(super,&(transformedVertices[i]));
        
        projected[i] = project(transformedVertices[i],camera);
    }
    for(int i=0;i<12;i++){
         if( isnan(projected[(cubeT[i]).indices[0]].x) || isnan(projected[(cubeT[i]).indices[1]].x) || isnan(projected[(cubeT[i]).indices[2]].x)) continue;
         if( isnan(projected[(cubeT[i]).indices[0]].y) || isnan(projected[(cubeT[i]).indices[1]].y) || isnan(projected[(cubeT[i]).indices[2]].y)) continue;
         raturize(transformedVertices[(cubeT[i]).indices[0]], transformedVertices[(cubeT[i]).indices[1]], transformedVertices[(cubeT[i]).indices[2]]);
       }

    //axis change
    angleXYZ[axis]+=0.01;
    angle+=0.01;
    if(angle>=(PI/4)){
        angle=0;
        axis=(axis+1)%3;
    }
    if(angleXYZ[axis]>=(2*PI)){
        angleXYZ[axis]=0;
    }

}

void raturize(Vector3 A,Vector3 B,Vector3 C){
        Vector2 a=project(A,camera);
    Vector2 b=project(B,camera);
    Vector2 c=project(C,camera);
    if(isnan(a.x)||isnan(b.x)||isnan(c.x))  return;
    if(isnan(a.y)||isnan(b.y)||isnan(c.y))  return;

      //Normallity
    Vector3 N = CrossProduct(VectorSub(B,A),VectorSub(A,C));
    float modN=sqrt(N.x*N.x+N.y*N.y+N.z*N.z);
    if(modN < 0.0001f)return;
    N.x=N.x/modN;
    N.y=N.y/modN;
    N.z=N.z/modN;
    float L=DotProduct(N,Light);

    float relAz=camera.z-A.z;
    float relBz=camera.z-B.z;
    float relCz=camera.z-C.z;

    int minX=(int)fminf(a.x, fminf(b.x, c.x));
    int minY=(int)fminf(a.y, fminf(b.y, c.y));
    int maxX=(int)fmaxf(a.x, fmaxf(b.x, c.x));
    int maxY=(int)fmaxf(a.y, fmaxf(b.y, c.y));
    if(minX<0) minX=0;
    if(minY<0) minY=0;
    if(maxX>=WIDTH) maxX=WIDTH-1;
    if(maxY>=HEIGHT) maxY=HEIGHT-1;

    for(int y=minY;y<=maxY;y++){
        for(int x=minX;x<=maxX;x++){

            float w0 = (b.x-a.x)*(y-a.y)-(b.y-a.y)*(x-a.x);
            float w1 = (c.x-b.x)*(y-b.y)-(c.y-b.y)*(x-b.x);
            float w2 = (a.x-c.x)*(y-c.y)-(a.y-c.y)*(x-c.x);
            //if(w0>=0&&w1>=0&&w2>=0 ||w0<=0&&w1<=0&&w2<=0){
            if(w0>=0&&w1>=0&&w2>=0){
                float totalArea=(b.x-a.x)*(c.y -a.y)-(b.y-a.y)*(c.x-a.x);

                if (fabsf(totalArea)<0.0001f)   return;

                float lambda1=w1/totalArea;
                float lambda2=w2/totalArea;
                float lambda0=1.0f-lambda1-lambda2;
                float pixelDepthToCam=(lambda1*relAz)+(lambda2*relBz)+(lambda0*relCz);
                if(pixelDepthToCam<Zindex[x][y]){
                    Zindex[x][y] = pixelDepthToCam;
                    Screen[x][y]=Color(L);
                }
            }
        }
    }
  
}

float DotProduct(Vector3 a,Vector3 b){
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
Vector3 CrossProduct(Vector3 a,Vector3 b){
    Vector3 ans;
    ans.x=a.y*b.z-a.z*b.y;
    ans.y=a.z*b.x-a.x*b.z;
    ans.z=a.x*b.y-a.y*b.x;
    return ans;
}

Vector3 VectorSub(Vector3 a,Vector3 b){
    Vector3 ans;
    ans.x=a.x-b.x;
    ans.y=a.y-b.y;
    ans.z=a.z-b.z;
    return ans;
}

void Rotation(float rotationalMatrix[3][3],Vector3 *point){
    float oldPoint[3]={point->x, point->y, point->z};
    float newPoint[3]={0.0,0.0,0.0};
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            newPoint[i]+=rotationalMatrix[i][j]*oldPoint[j];
        }
    }
    point->x=newPoint[0]; 
    point->y=newPoint[1]; 
    point->z=newPoint[2]; 
}

void MatMult(float Mat1[3][3],float Mat2[3][3], float Mat3[3][3]){
    float Ans[3][3];
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            Ans[i][j]=0.0;
            for(int k=0;k<3;k++)
                Ans[i][j]+=Mat1[i][k]*Mat2[k][j];
        }
    }
    memcpy(Mat3, Ans, sizeof(Ans));
}
void superRotator(float super[3][3],float thetaX,float thetaY,float thetaZ){
    float rotations[3][3][3]={{{    1,        0,              0    },
                              {     0,    cos(thetaX), -sin(thetaX)},
                              {     0,    sin(thetaX),  cos(thetaX)}},

                             {{cos(thetaY),      0,      -sin(thetaY)},
                              {     0,           1,          0       },
                              {sin(thetaY),      0,       cos(thetaY)}},

                             {{cos(thetaZ), -sin(thetaZ),   0},
                              {sin(thetaZ),  cos(thetaZ),   0},
                              {0,               0,          1}}};
    MatMult(rotations[0],super,super);
    MatMult(rotations[1],super,super);    
    MatMult(rotations[2],super,super);    

}

Vector2 project(Vector3 point,Vector3 camera){
    if ((camera.z-point.z)<0.001f) {
        return (Vector2){ NAN, NAN };
    }
    Vector2 proj=(Vector2 ){(WIDTH/2.0)+((point.x-camera.x)*f/(camera.z-point.z)), (HEIGHT/2.0)+yscalefactor*((point.y-camera.y)*f/(camera.z-point.z))};
    return proj;
}
