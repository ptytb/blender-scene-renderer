# OpenGL render engine, for scenes created in Blender

## Scene sources are supported in Blender
- OBJ model format support;
- MTL material format support;
- BVH skeletal animation format support (BioVision Hierarchy);
- TGA targa images for textures;
- All math hardcore explanation included. Russian only, but formulas are
  international 😉;

## Requirements
- OS: tested on Windows, Linux;
- GCC or Intel C++ compiler;
- GLEW library;
- [Blender](http://www.blender.org/) for creating scenes;

## Features
- Simple skeletal animation. No weight arrays for vertices.
- VBO (Vertex Buffer Objects);
- Quaternions are used for model orientation;
- Optimized for speed only. Peculiarity is that model's data vector accumulates
  error during animation playback. So model's geometry will degrade with time
  according to animation frames per second (aps). Read doc for explanation.

=====
by Pronin Ilya, updated Feb, 2014
