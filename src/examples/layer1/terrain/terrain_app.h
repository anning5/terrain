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
		nurbs_surface terrain;

    int mouse_x;
    int mouse_y;
    int mouse_wheel;
    GLuint texture;

		dynarray<vec3> vertices;
		dynarray<vec2> uvs;
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
			terrain.set_degree_u(3);
			terrain.set_degree_v(3);
			terrain.add_knot_u(0);
			terrain.add_knot_u(0);
			terrain.add_knot_u(0);
			terrain.add_knot_u(0);
			terrain.add_knot_u(1);
			terrain.add_knot_u(1);
			terrain.add_knot_u(1);
			terrain.add_knot_u(1);

			terrain.add_knot_v(0);
			terrain.add_knot_v(0);
			terrain.add_knot_v(0);
			terrain.add_knot_v(0);
			terrain.add_knot_v(1);
			terrain.add_knot_v(1);
			terrain.add_knot_v(1);
			terrain.add_knot_v(1);

			create_ctrl_points();
			
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

		void create_ctrl_points()
		{
			terrain.add_ctrl_points(vec3(0, 0, 0));
			terrain.add_ctrl_points(vec3(1, 1, 0));
			terrain.add_ctrl_points(vec3(2, 1, 0));
			terrain.add_ctrl_points(vec3(3, 0, 0));
			int index = 0;

			static float offset = .5f;
			offset = 0;
			terrain.add_ctrl_points(vec3(0, 0 + offset, 1));
			terrain.add_ctrl_points(vec3(1, 1 + offset, 1));
			terrain.add_ctrl_points(vec3(2, 1 + offset, 1));
			terrain.add_ctrl_points(vec3(3, 0 + offset, 1));

			static float offset1 = .5f;
			offset1 = 0;
			terrain.add_ctrl_points(vec3(0, 0 + offset1, 2));
			terrain.add_ctrl_points(vec3(1, 1 + offset1, 2));
			terrain.add_ctrl_points(vec3(2, 1 + offset1, 2));
			terrain.add_ctrl_points(vec3(3, 0 + offset1, 2));

			static float offset2 = 1.f;
			offset2 = 0;
			terrain.add_ctrl_points(vec3(0, 0 + offset2, 3));
			terrain.add_ctrl_points(vec3(1, 1 + offset2, 3));
			terrain.add_ctrl_points(vec3(2, 1 + offset2, 3));
			terrain.add_ctrl_points(vec3(3, 0 + offset2, 3));
		}

		void draw_ctrl_points()
		{
      color_shader.render(modelToProjection, vec4(1, 0, 0, 1));
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), &terrain.get_ctrl_points()[0]);
      glDrawArrays(GL_POINTS, 0, terrain.get_ctrl_points().size());
		}

		void generate_terrain_mesh(int resolution)
		{
			vertices.resize(resolution * resolution * 4);
			uvs.resize(resolution * resolution * 4);
			int index = 0;
			float du = 1.f / resolution, dv = 1.f / resolution, u = 0, v = 0;
			for(int i = 0; i < resolution; i++)
			{
				u = 0;
				for(int j = 0; j < resolution; j++)
				{
					terrain.get_surface_vertex(vertices[index], u, v);
					uvs[index][0] = vertices[index][0];
					uvs[index][1] = vertices[index][2];
					index++;

					terrain.get_surface_vertex(vertices[index], u, v + dv);
					uvs[index][0] = vertices[index][0];
					uvs[index][1] = vertices[index][2];
					index++;

					terrain.get_surface_vertex(vertices[index], u + du, v + dv);
					uvs[index][0] = vertices[index][0];
					uvs[index][1] = vertices[index][2];
					index++;

					terrain.get_surface_vertex(vertices[index], u + du, v);
					uvs[index][0] = vertices[index][0];
					uvs[index][1] = vertices[index][2];
					index++;
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

			static const int COUNT = 10;
			generate_terrain_mesh(COUNT);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), &vertices[0][0]);
      glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), &uvs[0][0]);
      glEnableVertexAttribArray(attribute_pos);
      glEnableVertexAttribArray(attribute_uv);

      glDrawArrays(GL_QUADS, 0, 4 * COUNT * COUNT);
    
			draw_ctrl_points();
		}
	};
}
