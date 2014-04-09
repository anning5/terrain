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
// Holds the RGBA buffer
  // this is an app to draw a triangle, it takes some of its workings from the class "app"
  class terrain_app : public app {
    // Matrix to transform points on our triangle to the world space
    // This allows us to move and rotate our triangle
    mat4t modelToWorld;
		mat4t modelToProjection;

    // Matrix to transform points in our camera space to the world.
    // This lets us move our camera
    camera_control cc;
    terrain_shader shader_;
    color_shader color_shader;

    int mouse_x;
    int mouse_y;
    int mouse_wheel;
    GLuint texture;

		float vertices[50000];
		float ctrl_points[10 * 10 * 3];
    bool is_left_button_down;
		static const int TERRAIN_WIDTH = 3;

  public:

    // this is called when we construct the class
    terrain_app(int argc, char **argv) :
	    app(argc, argv),
	    mouse_x(0),
	    mouse_y(0),
	    mouse_wheel(0),
	    is_left_button_down(false)
	  {
		  cc.set_view_distance(3.f);
		  //cc.rotate_h(45);
		  cc.rotate_v(-45);
		  cc.set_view_position(vec3(TERRAIN_WIDTH / 2.f, TERRAIN_WIDTH / 2.f, TERRAIN_WIDTH / 2.f));
		  mouse_wheel = get_mouse_wheel();
	  }

    // this is called once OpenGL is initialized
    void app_init() 
    {
			glPointSize(5.f);
			set_ctrl_points();
			generate_terrain_mesh();
			
      texture = resources::get_texture_handle(GL_RGB, "assets/terrain.gif");
	    // initialize the shader
	    shader_.init();
	    color_shader.init();

	    // put the triangle at the center of the world
	    modelToWorld.loadIdentity();
	    // put the camera a short distance from the center, looking towards the triangle

	    glEnable(GL_DEPTH_TEST);
	    glEnable(GL_CULL_FACE);
	    glCullFace(GL_BACK);
	    /*
	    glEnable(GL_BLEND);
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	    */
	    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

		void set_point(int index, float x, float y, float z)
		{
			ctrl_points[index] = x;
			ctrl_points[index + 1] = y;
			ctrl_points[index + 2] = z;
		}

		void set_ctrl_points()
		{
			int index = 0;
			set_point(index, 0, 0, 0); index += 3;
			set_point(index, 1, 1, 0); index += 3;
			set_point(index, 2, 1, 0); index += 3;
			set_point(index, 3, 0, 0); index += 3;

			set_point(index, 0, 0, 1); index += 3;
			set_point(index, 1, 1, 1); index += 3;
			set_point(index, 2, 1, 1); index += 3;
			set_point(index, 3, 0, 1); index += 3;

			set_point(index, 0, 0, 2); index += 3;
			set_point(index, 1, 1, 2); index += 3;
			set_point(index, 2, 1, 2); index += 3;
			set_point(index, 3, 0, 2); index += 3;

			set_point(index, 0, 0, 3); index += 3;
			set_point(index, 1, 1, 3); index += 3;
			set_point(index, 2, 1, 3); index += 3;
			set_point(index, 3, 0, 3); index += 3;
		}

		void draw_ctrl_points(int count)
		{
      color_shader.render(modelToProjection, vec4(1, 0, 0, 1));
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)ctrl_points);
      glDrawArrays(GL_POINTS, 0, count);
		}

		void get_surface_vertex_by_uv(float *vertex, float u, float v)
		{
			int degree = 3;
			int ctrl_point_count = 4;
			float knots[] = {0, 0, 0 ,0, 1, 1, 1, 1};
			//float knots[] = {0, 0, 0 ,0, .1f, .3f, .6f, 1};
			vec3 v1(0, 0, 0), v2(1, 1, 0), v3(2, 1, 0), v4(3, 0, 0);
			float basis_u[4], basis_v[4];
			get_basis_functions(degree, u, ctrl_point_count, knots, basis_u);
			get_basis_functions(degree, v, ctrl_point_count, knots, basis_v);
			get_surface_vertex(vertex, ctrl_points, basis_u, 4, basis_v, 4);
		}

		void generate_terrain_mesh()
		{
			int index = 0;
			const int COUNT = 10;
			float du = 1.f / COUNT, dv = 1.f / COUNT, u = 0, v = 0;
			for(int i = 0; i < COUNT; i++)
			{
				u = 0;
				for(int j = 0; j < COUNT; j++)
				{
					if(u > .9f)
					{
						u = .9f;
					}
					if(v > .9f)
					{
						v = .9f;
					}
					get_surface_vertex_by_uv(vertices + index, u, v);
					vertices[index + 3] = vertices[index];
					vertices[index + 4] = vertices[index + 2];
					index += 5;
					get_surface_vertex_by_uv(vertices + index, u, v + dv);
					vertices[index + 3] = vertices[index];
					vertices[index + 4] = vertices[index + 2];
					index += 5;
					get_surface_vertex_by_uv(vertices + index, u + du, v + dv);
					vertices[index + 3] = vertices[index];
					vertices[index + 4] = vertices[index + 2];
					index += 5;
					get_surface_vertex_by_uv(vertices + index, u + du, v);
					vertices[index + 3] = vertices[index];
					vertices[index + 4] = vertices[index + 2];
					index += 5;
					u += du;
				}
				v += dv;
			}
		}

    // this is called to draw the world
    void draw_world(int x1, int y1, int w1, int h1) {
	static DWORD lastFrameCount = 0;
	static DWORD curFrameCount = 0;
	static DWORD aa = GetTickCount();
	curFrameCount++;
	int count = GetTickCount() - aa;
	if(count >= 1000)
	{
		//printf("%d\n", (int)((curFrameCount - lastFrameCount) * 1000.f / count));
		char buf[256];
		sprintf(buf, "%d", (int)((curFrameCount - lastFrameCount) * 1000.f / count));
		SetWindowTextA(get_hwnd(), buf);
		lastFrameCount = curFrameCount;
		aa += count;
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
	      static float factor1 = .1f;
	      cc.add_view_distance(mouse_wheel_delta / WHEEL_DELTA * factor1);
	      mouse_wheel = get_mouse_wheel();
      }
			static float factor2 = .005f;
			if(is_key_down('S'))
			{
	      cc.add_view_distance(factor2);
			}
			if(is_key_down('W'))
			{
	      cc.add_view_distance(-factor2);
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
      modelToProjection = mat4t::build_projection_matrix(modelToWorld, cc.get_matrix());

      // spin the triangle by rotating about Z (the view direction)
      //modelToWorld.rotateZ(.01);

      // set up opengl to draw flat shaded triangles of a fixed color
      shader_.render(modelToProjection, 0);
      int index = 0;

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)vertices);
      glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(vertices + 3));
      glEnableVertexAttribArray(attribute_pos);
      glEnableVertexAttribArray(attribute_uv);
			/*
      fill_vertices(vertices + index, TERRAIN_WIDTH, 0, 0, 1, 0); index += 5;
      fill_vertices(vertices + index, 0, 0, 0, 0, 0); index += 5;
      fill_vertices(vertices + index, 0, 0, TERRAIN_WIDTH, 0, 1); index += 5;
      fill_vertices(vertices + index, TERRAIN_WIDTH, 0, TERRAIN_WIDTH, 1, 1); index += 5;
      glDrawArrays(GL_QUADS, 0, 4);
			//*/
      glDrawArrays(GL_QUADS, 0, 4 * 10 * 10);
    
			draw_ctrl_points(16);
		}
	};
}
