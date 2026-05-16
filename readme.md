A simple project built with [raylib](https://www.raylib.com/). This program renders a 3D cube using basic points and lines (wireframe style) and rotates it continuously in space. 

## Features
* **3D Rendering:** Draws a 3D cube from ascii charaters, in the terminal.
* **Customisable:**All Parameters(variables) are customisble. although not intended you can also change the vertices matrix to a set of points, and update the faces matrix (cubeT) to hold the face vetices in anti clockwise order to render any shape. *NOTE: the size of both of these are hardcoded to be 8 and 12 (not only at their declaration but also all the loops (Find and Replace all should fix it))*
The animation by changing the logic can be changed in the UpdateDrawFrame() function.

## Prerequisites
Before running this project, ensure you have the following set up:
* A working C/C++ Compiler 

## Getting Started

### 1. Clone the repository
```bash
git clone [https://github.com/saharshkappa/ASCubeII.git]
cd [ASCubeII]
```
### 2. To Compile 
Run
```
gcc cube.c -lm
```

### 3. To Run 
Run
```
./a.out
```