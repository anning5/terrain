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
    vertex_color_shader vertex_color_shader_;
		nurbs_surface terrain;
		sky_box sb;

    int mouse_x;
    int mouse_y;
    int mouse_wheel;
		int key_cool_down;
		unsigned int current_selected_ctrl_point;
    GLuint texture;

		dynarray<vec3> ctrl_point_colors;
		dynarray<vec3> vertices;
		dynarray<vec2> uvs;
    bool is_right_button_down;
    bool is_left_button_down;
    bool toggle_wireframe;
    bool toggle_ctrl_points;
		static const int TERRAIN_WIDTH = 3;

  public:

    // this is called when we construct the class
    terrain_app(int argc, char **argv) :
	    app(argc, argv),
	    mouse_x(0),
	    mouse_y(0),
	    mouse_wheel(0),
	    is_right_button_down(false),
	    is_left_button_down(false),
			key_cool_down(0),
			toggle_wireframe(false),
			toggle_ctrl_points(true),
			current_selected_ctrl_point(-1)
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
			glEnableVertexAttribArray(attribute_pos);
			glEnableVertexAttribArray(attribute_uv);
			glEnableVertexAttribArray(attribute_color);
			sb.init("assets/sky_box.jpg");
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
	    vertex_color_shader_.init();

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
	    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

		void create_ctrl_points()
		{
			terrain.add_weight_u(1);
			terrain.add_weight_u(1);
			terrain.add_weight_u(1);
			terrain.add_weight_u(1);
			terrain.add_weight_v(1);
			terrain.add_weight_v(1);
			terrain.add_weight_v(1);
			terrain.add_weight_v(1);

			terrain.add_ctrl_points(vec3(0, 0, 0));
			terrain.add_ctrl_points(vec3(1 * TERRAIN_WIDTH / 3.f, 0, 0));
			terrain.add_ctrl_points(vec3(2 * TERRAIN_WIDTH / 3.f, 0, 0));
			terrain.add_ctrl_points(vec3(3 * TERRAIN_WIDTH / 3.f, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			int index = 0;

			static float offset = .5f;
			//offset = 0;
			terrain.add_ctrl_points(vec3(0, 0, 1));
			terrain.add_ctrl_points(vec3(1 * TERRAIN_WIDTH / 3.f, 0, 1));
			terrain.add_ctrl_points(vec3(2 * TERRAIN_WIDTH / 3.f, 0, 1));
			terrain.add_ctrl_points(vec3(3 * TERRAIN_WIDTH / 3.f, 0, 1));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));

			static float offset1 = .5f;
			//offset1 = 0;
			terrain.add_ctrl_points(vec3(0, 0, 2));
			terrain.add_ctrl_points(vec3(1 * TERRAIN_WIDTH / 3.f, 0, 2));
			terrain.add_ctrl_points(vec3(2 * TERRAIN_WIDTH / 3.f, 0, 2));
			terrain.add_ctrl_points(vec3(3 * TERRAIN_WIDTH / 3.f, 0, 2));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));

			static float offset2 = 1.f;
			//offset2 = 0;
			terrain.add_ctrl_points(vec3(0, 0, 3));
			terrain.add_ctrl_points(vec3(1 * TERRAIN_WIDTH / 3.f, 0, 3));
			terrain.add_ctrl_points(vec3(2 * TERRAIN_WIDTH / 3.f, 0, 3));
			terrain.add_ctrl_points(vec3(3 * TERRAIN_WIDTH / 3.f, 0, 3));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
			ctrl_point_colors.push_back(vec3(1, 0, 0));
		}

		vec3 view_to_world(const vec3 &v)
		{
			return v * cc.get_matrix();
		}

		void select_ctrl_point()
		{
			int x, y, w, h;
			get_mouse_pos(x, y);
			get_viewport_size(w, h);
			vec3 target(x - w * .5f, h * .5f - y, -w * .5f);
			vec3 camera((vec3&)(cc.get_matrix()[3]));
			vec3 ray((view_to_world(target) - camera).normalize());

			static float radius = .05f;

			const dynarray<vec3> &points = terrain.get_ctrl_points();
			float x0 = camera[0], y0 = camera[1], z0 = camera[2], x1 = ray[0], y1 = ray[1], z1 = ray[2];
			for(unsigned int i = 0; i < points.size(); i++)
			{
				float x02 = x0 - points[i][0];
				float y02 = y0 - points[i][1];
				float z02 = z0 - points[i][2];

				float a = x1 * x1 + y1 * y1 + z1 * z1;
				float b = 2 * (-x1 * x02 - y1 * y02 - z1 * z02);
				float c = x02 * x02+ y02 * y02 + z02 * z02 - radius * radius;

				if(b * b - 4 * a * c >= 0)
				{
					ctrl_point_colors[current_selected_ctrl_point] = vec3(1, 0, 0);
					current_selected_ctrl_point = i;
					ctrl_point_colors[current_selected_ctrl_point] = vec3(1, 1, 0);
				}
			}
		}

		void draw_ctrl_points()
		{
			vertex_color_shader_.render(modelToProjection);
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), &terrain.get_ctrl_points()[0]);
			glVertexAttribPointer(attribute_color, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), &ctrl_point_colors[0]);
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

		void move_ctrl_point(int x, int y)
		{
				if(mouse_x != x || mouse_y != y)
				{
					const dynarray<vec3> &ctrl_points = terrain.get_ctrl_points();
					if(current_selected_ctrl_point < ctrl_points.size())
					{
						int w, h;
						get_viewport_size(w, h);
						float half_w = w * .5f;
						float half_h = h * .5f;
						vec3 ray(x - half_w, half_h - y, -half_w);
						mat4t worldToCamera;
						cc.get_matrix().invertQuick(worldToCamera);
						const vec3 &ctrl_point = ctrl_points[current_selected_ctrl_point];
						vec3 v =  ctrl_point * worldToCamera;
						float k = v[2] / ray[2];
						v[0] = ray[0] * k;
						v[1] = ray[1] * k;
						v[2] = ray[2] * k;
						terrain.set_ctrl_points(current_selected_ctrl_point, view_to_world(v));
					}

					mouse_x = x;
					mouse_y = y;
				}
		}

		// this is called to draw the world
		void draw_world(int x1, int y1, int w1, int h1) {
			static DWORD lastFrameCount = 0;
			static DWORD curFrameCount = 0;
			DWORD tick_count = GetTickCount();
			static DWORD aa = tick_count;
			curFrameCount++;
			int count = tick_count - aa;
			if(count >= 1000)
			{
				//printf("%d\n", (int)((curFrameCount - lastFrameCount) * 1000.f / count));
				char buf[256];
				sprintf(buf, "%d", (int)((curFrameCount - lastFrameCount) * 1000.f / count));
				SetWindowTextA(get_hwnd(), buf);
				lastFrameCount = curFrameCount;
				aa += count;
			}
			//terrain.add_weight_v(1, .001f);
			// set a viewport - includes whole window area
			glViewport(x1, y1, w1, h1);

			// clear the background to black
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//left mouse button
			if(is_key_down(key_lmb) && !is_left_button_down)
			{
				is_left_button_down = true;
				select_ctrl_point();
				get_mouse_pos(mouse_x, mouse_y);
				SetCapture(get_hwnd());
			}
			else if(is_left_button_down && !is_key_down(key_lmb))
			{
				is_left_button_down = false;
				ReleaseCapture();
			}
			if(is_left_button_down)
			{
				static float factor = .002f;
				int x, y;
				get_mouse_pos(x, y);
				short sx = x, sy = y;
				int delta_x = sx - mouse_x, delta_y = mouse_y - sy;
				move_ctrl_point(sx, sy);
			}

			//right mouse button
			if(is_key_down(key_rmb) && !is_right_button_down)
			{
				is_right_button_down = true;
				get_mouse_pos(mouse_x, mouse_y);
				SetCapture(get_hwnd());
			}
			else if(is_right_button_down && !is_key_down(key_rmb))
			{
				is_right_button_down = false;
				ReleaseCapture();
			}
			if(is_right_button_down)
			{
				static float factor = .2f;
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

			static float factor3 = .01f;
			if(is_key_down(key_up))
			{
				terrain.increase_weight_value_u(current_selected_ctrl_point, factor3);
			}
			if(is_key_down(key_down))
			{
				terrain.increase_weight_value_u(current_selected_ctrl_point, -factor3);
			}
			if(is_key_down(key_left))
			{
				terrain.increase_weight_value_v(current_selected_ctrl_point, -factor3);
			}
			if(is_key_down(key_right))
			{
				terrain.increase_weight_value_v(current_selected_ctrl_point, factor3);
			}

			if(tick_count - key_cool_down > 200)
			{
				if(is_key_down('P'))
				{
					key_cool_down = tick_count;
					toggle_wireframe = !toggle_wireframe;
					if(toggle_wireframe)
					{
						glPolygonMode(GL_FRONT, GL_LINE);
					}
					else
					{
						glPolygonMode(GL_FRONT, GL_FILL);
					}
				}
				if(is_key_down('C'))
				{
					key_cool_down = tick_count;
					toggle_ctrl_points = !toggle_ctrl_points;
				}
			}

			// build a projection matrix: model -> world -> camera -> projection
			// the projection space is the cube -1 <= x/w, y/w, z/w <= 1
			modelToProjection = mat4t::build_projection_matrix(modelToWorld, cc.get_matrix());

			// spin the triangle by rotating about Z (the view direction)
			//modelToWorld.rotateZ(.01);

			// set up opengl to draw flat shaded triangles of a fixed color
			sb.render(modelToProjection, 0);
			shader_.render(modelToProjection, 0);

			static const int COUNT = 10;
			generate_terrain_mesh(COUNT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), &vertices[0]);
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), &uvs[0]);

			glDrawArrays(GL_QUADS, 0, 4 * COUNT * COUNT);

			if(toggle_ctrl_points)
			{
				draw_ctrl_points();
			}
		}
	};
}
