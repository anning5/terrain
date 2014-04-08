////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// angry_box2d example: simple 2d physics game with sprites and sounds
//
// Level: 1
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Simple game mechanics
//   Texture loaded from GIF file
//   Audio
//   Simple 2D physics
//

namespace octet {
  class sprite_2d {
    // rigid body information for this sprite.
    b2Body *body; 
    // what texture is on our sprite?
    int texture;

