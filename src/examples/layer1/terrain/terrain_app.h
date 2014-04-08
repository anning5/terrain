////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Triangle example: The most basic OpenGL application
//
// Level: 0
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//
//

// namespace help us to group classes and avoid clashes
namespace octet {


inline void fill_vertices(float *vertices, float x, float y, float z, float u, float v)
{
	vertices[0] = x;
	vertices[1] = y;
	vertices[2] = z;
	vertices[3] = u;
	vertices[4] = v;
}

// Holds the vertices
static float vertices[1024];
// Holds the RGBA buffer
  // this is an app to draw a triangle, it takes some of its workings from the class "app"
  class terrain_app : public app {
    // Matrix to transform points on our triangle to the world space
    // This allows us to move and rotate our triangle
    mat4t modelToWorld;

    // Matrix to transform points in our camera space to the world.
    // This lets us move our camera
    camera_control cc;

    int mouse_x;
    int mouse_y;
    int mouse_wheel;
    GLuint texture;
    float view_distance_delta;
    bool is_left_button_down;

    // shader to draw a solid color
    terrain_shader shader_;

  public:

    // this is called when we construct the class
    terrain_app(int argc, char **argv) :
	    app(argc, argv),
	    mouse_x(0),
	    mouse_y(0),
	    mouse_wheel(0),
	    is_left_button_down(false),
	    view_distance_delta(.8f)
	  {
		  cc.set_view_distance(1.f);
		  cc.rotate_h(45);
		  cc.rotate_v(-45);
		  cc.set_view_position(vec3(.5f, .5f, .5f));
		  mouse_wheel = get_mouse_wheel();
	  }

    // this is called once OpenGL is initialized
    void app_init() 
    {
      texture = resources::get_texture_handle(GL_RGB, "assets/terrain.gif");
	    // initialize the shader
	    shader_.init();

	    // put the triangle at the center of the world
	    modelToWorld.loadIdentity();
	    // put the camera a short distance from the center, looking towards the triangle

	    //glEnable(GL_DEPTH_TEST);
	    glEnable(GL_CULL_FACE);
	    glCullFace(GL_BACK);
	    /*
	    glEnable(GL_BLEND);
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	    */
	    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // this is called to draw the world
    void draw_world(int x1, int y1, int w1, int h1) {
	static DWORD lastFrameCount = 0;
	static DWORD curFrameCount = 0;
	static DWORD aa = GetTickCount();
	curFrameCount++;
	if(GetTickCount() - aa >= 1000)
	{
		printf("%d\n", curFrameCount - lastFrameCount);
		lastFrameCount = curFrameCount;
		aa = GetTickCount();
	}
		/*
		static int ii = 0;
		printf("%d\n", ii++);
		//*/
      // set a viewport - includes whole window area
      glViewport(x1, y1, w1, h1);

      // clear the background to black
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      if(is_key_down(key_lmb) && !is_left_button_down)
      {
	      is_left_button_down = true;
	      get_mouse_pos(mouse_x, mouse_y);
	      SetCapture(get_hwnd());
      }
      else if(is_left_button_down && !is_key_down(key_lmb))
      {
	      is_left_button_down = false;
	      ReleaseCapture();
      }
      int mouse_wheel_delta = get_mouse_wheel() - mouse_wheel;
      if(mouse_wheel_delta != 0)
      {
	      static float factor = .1f;
	      cc.add_view_distance(mouse_wheel_delta / WHEEL_DELTA * factor);
	      mouse_wheel = get_mouse_wheel();
      }
			static float factor = .01f;
			if(is_key_down('W'))
			{
	      cc.add_view_distance(factor);
			}
			if(is_key_down('S'))
			{
	      cc.add_view_distance(-factor);
			}
      if(is_left_button_down)
      {
	      static float factor = .2f;
	      is_left_button_down = true;
	      int x, y;
	      get_mouse_pos(x, y);
	      short sx = x, sy = y;
	      int delta_x = mouse_x - sx, delta_y = mouse_y - sy;
	      if(delta_x != 0)
		      cc.rotate_h((float)delta_x * factor);
	      if(delta_y != 0)
		      cc.rotate_v((float)delta_y * factor);
	      mouse_x = sx;
	      mouse_y = sy;
      }

      // build a projection matrix: model -> world -> camera -> projection
      // the projection space is the cube -1 <= x/w, y/w, z/w <= 1
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cc.get_matrix());

      // spin the triangle by rotating about Z (the view direction)
      //modelToWorld.rotateZ(.01);

      // set up opengl to draw flat shaded triangles of a fixed color
      shader_.render(modelToProjection, 0);
      int index = 0;

      fill_vertices(vertices + index, 1, 0, 0, 1, 0); index += 5;
      fill_vertices(vertices + index, 0, 0, 0, 0, 0); index += 5;
      fill_vertices(vertices + index, 0, 0, 1, 0, 1); index += 5;
      fill_vertices(vertices + index, 1, 0, 1, 1, 1); index += 5;
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)vertices);
      glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(vertices + 2));
      glEnableVertexAttribArray(attribute_pos);
      glEnableVertexAttribArray(attribute_uv);
    
      glDrawArrays(GL_QUADS, 0, 4);
    }
  };
}
