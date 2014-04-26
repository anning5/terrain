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
		mat4t worldToProjection;
		mat4t reflective_worldToProjection;

    // Matrix to transform points in our camera space to the world.
    // This lets us move our camera
    camera_control cc;
    camera_control cc_reflection;
    terrain_shader terrain_shader_;
    vertex_color_shader vertex_color_terrain_shader_;
		nurbs_surface terrain;
		water_surface water;
		sky_box sb;

    int mouse_x;
    int mouse_y;
    int mouse_wheel;
		int key_cool_down;
		int resolution;
		int ctrl_point_count;
		int degree;
		float water_height;
		unsigned int current_selected_ctrl_point;
    GLuint texture;
    GLuint vbo_terrain;
    GLuint vbo_ctrl_points;
    GLuint vao_terrain;
    GLuint vao_ctrl_points;
		GLuint vao_normal;
		GLuint vbo_normal;
		GLuint ibo;
		GLuint frame_buffer;
		GLuint frame_texture;
		GLuint depth_render_buffer;

		dynarray<vec3> ctrl_point_colors;
		dynarray<vec3> vertices;
		dynarray<vec3> normals;
		dynarray<vec2> uvs;
		dynarray<vec3> normal_vertices;
		dynarray<vec3> normal_colors;
		dynarray<unsigned int> indices;
    bool is_right_button_down;
    bool is_left_button_down;
    bool toggle_wireframe;
    bool toggle_ctrl_points;
    bool reallocate_vertices;
    bool reallocate_ctrl_points;
    bool draw_normal;
		static const int TERRAIN_WIDTH = 20;

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
			toggle_ctrl_points(false),
			current_selected_ctrl_point(-1),
			resolution(50),
			ctrl_point_count(10),
			degree(3),
			reallocate_vertices(true),
			reallocate_ctrl_points(true),
			draw_normal(false),
			water_height(.2f)
	  {
			const float DIS = 3.f, ANGLE_V = 25.f;
		  cc.set_view_distance(DIS);
		  cc_reflection.set_view_distance(DIS);
		  cc.rotate_v(-ANGLE_V);
		  cc_reflection.rotate_v(ANGLE_V);
			float span = (float)TERRAIN_WIDTH / ctrl_point_count;
			float offset = (TERRAIN_WIDTH - span) * .5f;
			vec3 pos(offset, water_height, offset);
		  cc.set_view_position(pos);
		  cc_reflection.set_view_position(pos);
		  mouse_wheel = get_mouse_wheel();
	  }

		void buffer_terrain_vertices()
		{
			glBindVertexArray(vao_terrain);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_terrain);
			int pos_array_size = vertices.size() * sizeof(vec3);
			int uv_array_size = uvs.size() * sizeof(vec2);
			int normal_array_size = normals.size() * sizeof(vec3);
			if(reallocate_vertices)
			{
				glBufferData(GL_ARRAY_BUFFER, pos_array_size + uv_array_size + normal_array_size, NULL, GL_STATIC_DRAW);
				glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
				glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)(vertices.size() * sizeof(vec3)));
				glVertexAttribPointer(attribute_normal, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(vertices.size() * sizeof(vec3) + uvs.size() * sizeof(vec2)));
			}
			glBufferSubData(GL_ARRAY_BUFFER, 0, pos_array_size, &vertices[0]);
			glBufferSubData(GL_ARRAY_BUFFER, pos_array_size, uv_array_size, &uvs[0]);
			glBufferSubData(GL_ARRAY_BUFFER, pos_array_size + uv_array_size, normal_array_size, &normals[0]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			if(reallocate_vertices)
			{
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
			}
			glBindVertexArray(0);

			glBindVertexArray(vao_normal);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
			glVertexAttribPointer(attribute_color, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(normal_vertices.size() * sizeof(vec3)));
			if(reallocate_vertices)
			{
				glBufferData(GL_ARRAY_BUFFER, normal_vertices.size() * sizeof(vec3) + normal_colors.size() * sizeof(vec3), NULL, GL_STATIC_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER, normal_vertices.size() * sizeof(vec3), normal_colors.size() * sizeof(vec3), &normal_colors[0]);
			}
			glBufferSubData(GL_ARRAY_BUFFER, 0, normal_vertices.size() * sizeof(vec3), &normal_vertices[0]);
			glBindVertexArray(0);
			reallocate_vertices = false;
		}

		void buffer_ctrl_points_vertices()
		{
			glBindVertexArray(vao_ctrl_points);

			glBindBuffer(GL_ARRAY_BUFFER, vbo_ctrl_points);
			int pos_array_size = terrain.get_ctrl_points().size() * sizeof(vec3);
			if(reallocate_ctrl_points)
			{
				glBufferData(GL_ARRAY_BUFFER, pos_array_size + ctrl_point_colors.size() * sizeof(vec3), NULL, GL_STATIC_DRAW);
			}
			glBufferSubData(GL_ARRAY_BUFFER, 0, pos_array_size, &terrain.get_ctrl_points()[0]);
			glBufferSubData(GL_ARRAY_BUFFER, pos_array_size, ctrl_point_colors.size() * sizeof(vec3), &ctrl_point_colors[0]);

			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
			glVertexAttribPointer(attribute_color, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)pos_array_size);
			glBindVertexArray(0);
			reallocate_ctrl_points = false;
		}

		void reset_terrain()
		{
			terrain.reset();
			terrain.set_degree_u(degree);
			terrain.set_degree_v(degree);
			for(int i = 0; i < ctrl_point_count + degree + 1; i++)
			{
				terrain.add_knot_u((float)i);
				terrain.add_knot_v((float)i);
			}

			reset_ctrl_points();
			generate_terrain_mesh();
		}

		void object_init()
		{
			glGenVertexArrays(1, &vao_terrain);
			glBindVertexArray(vao_terrain);
			glEnableVertexAttribArray(attribute_pos);
			glEnableVertexAttribArray(attribute_uv);
			glEnableVertexAttribArray(attribute_normal);
			glGenBuffers(1, &vbo_terrain);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_terrain);
			glGenBuffers(1, &ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

			glGenVertexArrays(1, &vao_normal);
			glBindVertexArray(vao_normal);
			glGenBuffers(1, &vbo_normal);
			glEnableVertexAttribArray(attribute_pos);
			glEnableVertexAttribArray(attribute_color);

			glGenVertexArrays(1, &vao_ctrl_points);
			glBindVertexArray(vao_ctrl_points);
			glEnableVertexAttribArray(attribute_pos);
			glEnableVertexAttribArray(attribute_color);
			glGenBuffers(1, &vbo_ctrl_points);
			glBindVertexArray(0);
		}

    // this is called once OpenGL is initialized
    void app_init() 
    {
			object_init();
			sb.init("assets/sky_box.jpg");
			generate_frame_buffer();
			water.init(vec3(10.f, water_height, 10.f), frame_texture);
			glPointSize(5.f);
			reset_terrain();
			
      texture = resources::get_texture_handle(GL_RGB, "assets/terrain.jpg");
	    // initialize the shader
	    terrain_shader_.init();
	    vertex_color_terrain_shader_.init();

	    // put the triangle at the center of the world
	    modelToWorld.loadIdentity();
	    // put the camera a short distance from the center, looking towards the triangle

	    glEnable(GL_DEPTH_TEST);
	    glEnable(GL_CULL_FACE);
	    //glDisable(GL_CULL_FACE);
	    glCullFace(GL_BACK);
	    ///*
	    glEnable(GL_BLEND);
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	    //*/
    }

		void reset_ctrl_points()
		{
			ctrl_point_colors.reset();
			int index = 0;
			float offset = TERRAIN_WIDTH / (float)ctrl_point_count;
			for(int i = 0; i < ctrl_point_count; i++)
			{
				for(int j = 0; j < ctrl_point_count; j++)
				{
					terrain.add_ctrl_points(vec3(j * offset, 0, offset * i));
					terrain.add_weight(1);
					ctrl_point_colors.push_back(vec3(1, 0, 0));
				}
			}

			buffer_ctrl_points_vertices();
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

			static float radius = .1f;

			const dynarray<vec3> &points = terrain.get_ctrl_points();
			float x0 = camera[0], y0 = camera[1], z0 = camera[2], x1 = ray[0], y1 = ray[1], z1 = ray[2];
			unsigned int i = 0;
			for(; i < points.size(); i++)
			{
				float x02 = x0 - points[i][0];
				float y02 = y0 - points[i][1];
				float z02 = z0 - points[i][2];

				float a = x1 * x1 + y1 * y1 + z1 * z1;
				float b = 2 * (-x1 * x02 - y1 * y02 - z1 * z02);
				float c = x02 * x02+ y02 * y02 + z02 * z02 - radius * radius;

				if(b * b - 4 * a * c >= 0)
				{
					if(current_selected_ctrl_point < ctrl_point_colors.size())
					{
						ctrl_point_colors[current_selected_ctrl_point] = vec3(1, 0, 0);
					}
					current_selected_ctrl_point = i;
					ctrl_point_colors[current_selected_ctrl_point] = vec3(1, 1, 0);
					break;
				}
			}
			if(i == points.size() && current_selected_ctrl_point < ctrl_point_colors.size())
			{
					ctrl_point_colors[current_selected_ctrl_point] = vec3(1, 0, 0);
					current_selected_ctrl_point = -1;
			}
			buffer_ctrl_points_vertices();
		}

		void draw_ctrl_points()
		{
			vertex_color_terrain_shader_.render(worldToProjection);
			glBindVertexArray(vao_ctrl_points);
			glDrawArrays(GL_POINTS, 0, terrain.get_ctrl_points().size());
			glBindVertexArray(0);
		}

		void generate_frame_buffer()
		{
			int w, h;
			get_viewport_size(w, h);
			glGenFramebuffers(1, &frame_buffer);
			glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
			
			glGenTextures(1, &frame_texture);
			glBindTexture(GL_TEXTURE_2D, frame_texture);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_BYTE, 0);
			
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_texture, 0); 
			
			glGenRenderbuffers(1, &depth_render_buffer);
			glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_render_buffer);
			GLenum res = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			assert(res == GL_FRAMEBUFFER_COMPLETE);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void generate_terrain_mesh()
		{
			float start = terrain.get_knot_u(degree), end = terrain.get_knot_u(terrain.get_knot_count_u() - 1 - degree);
			if(start == INVALID_KNOT_VALUE || end == INVALID_KNOT_VALUE)
			{
				return;
			}
			int width = resolution + 1;
			int count = width * width;
			vertices.resize(count);
			uvs.resize(count);
			normals.resize(count);
			indices.resize(resolution * resolution * 4);
			normal_vertices.resize(count * 2);
			normal_colors.resize(count * 2);
			for(unsigned int i = 0; i < normal_colors.size(); i++)
			{
				normal_colors[i] = vec3(1, 0, 0);
			}
			int index = 0;
			float length = end - start;
			float du = length / resolution, dv = length / resolution, u = start, v = start;
			for(int i = 0; i < width; i++)
			{
				u = start;
				for(int j = 0; j < width; j++)
				{
					terrain.get_surface_vertex(vertices[index], u, v);
					uvs[index][0] = vertices[index][0];
					uvs[index][1] = vertices[index][2];
					u += du;
					index++;
				}
				v += dv;
			}
			index = 0;
			vec3 sum;
			for(int i = 0; i < width; i++)
			{
				for(int j = 0; j < width; j++)
				{
					sum[0] = 0;
					sum[1] = 0;
					sum[2] = 0;
					int k = 0;
					if(i > 0)
					{
						if(j > 0)
						{
							k++;
							sum += get_normal(vertices[index - width], vertices[index], vertices[index - 1]);
						}
						if(j < resolution)
						{
							k++;
							sum += get_normal(vertices[index + 1], vertices[index], vertices[index - width]);
						}
					}
					if(i < resolution)
					{
						if(j > 0)
						{
							k++;
							sum += get_normal(vertices[index - 1], vertices[index], vertices[index + width]);
						}
						if(j < resolution)
						{
							k++;
							sum += get_normal(vertices[index + width], vertices[index], vertices[index + 1]);
						}
					}
					normals[index] = normalize(sum / (float)k);
					normals[index] = vec3(0, 1, 0);
					normal_vertices[index * 2] = vertices[index];
					normal_vertices[index * 2 + 1] = vertices[index] + normals[index];
					index++;
				}
			}
			index = 0;
			int index1 = 0;
			for(int i = 0; i < resolution; i++)
			{
				for(int j = 0; j < resolution; j++)
				{
					indices[index++] = index1 + j;
					indices[index++] = index1 + width + j;
					indices[index++] = index1 + width + j + 1;
					indices[index++] = index1 + j + 1;
				}
				index1 += width;
			}
			buffer_terrain_vertices();
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
					generate_terrain_mesh();
					buffer_ctrl_points_vertices();
				}
				mouse_x = x;
				mouse_y = y;
			}
		}

		void handle_messages()
		{
			DWORD tick_count = GetTickCount();
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
				{
					float f = delta_x * factor;
					cc.rotate_h(f);
					cc_reflection.rotate_h(f);
				}
				if(delta_y != 0)
				{
					float f = delta_y * factor;
					cc.rotate_v(f);
					cc_reflection.rotate_v(-f);
				}
				mouse_x = sx;
				mouse_y = sy;
			}

			int mouse_wheel_delta = get_mouse_wheel() - mouse_wheel;
			if(mouse_wheel_delta != 0)
			{
				static float factor1 = .1f;
				float f = mouse_wheel_delta / WHEEL_DELTA * factor1;
				cc.add_view_distance(f);
				cc_reflection.add_view_distance(f);
				mouse_wheel = get_mouse_wheel();
			}
			static float factor2 = .005f;
			if(is_key_down('S'))
			{
				cc.add_view_distance(factor2);
				cc_reflection.add_view_distance(factor2);
			}
			if(is_key_down('W'))
			{
				cc.add_view_distance(-factor2);
				cc_reflection.add_view_distance(-factor2);
			}

			static float factor3 = .01f;
			if(is_key_down(key_up))
			{
				terrain.increase_weight_value(current_selected_ctrl_point, factor3);
				generate_terrain_mesh();
			}
			if(is_key_down(key_down))
			{
				terrain.increase_weight_value(current_selected_ctrl_point, -factor3);
				generate_terrain_mesh();
			}

			if(tick_count - key_cool_down > 200)
			{
				if(is_key_down('Z'))
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
				if(is_key_down('V'))
				{
					key_cool_down = tick_count;
					water.toggle_draw_normal();
				}
				if(is_key_down('B'))
				{
					key_cool_down = tick_count;
					draw_normal = !draw_normal;
				}
				if(is_key_down('R'))
				{
					key_cool_down = tick_count;
					if(is_key_down(key_shift))
					{
						if(resolution != 0)
						{
							resolution--;
						}
					}
					else
					{
						resolution++;
					}
					reallocate_vertices = true;
					generate_terrain_mesh();
				}
				if(is_key_down('F'))
				{
					key_cool_down = tick_count;
					if(is_key_down(key_shift))
					{
						if(ctrl_point_count != 0)
						{
							ctrl_point_count--;
						}
					}
					else
					{
						ctrl_point_count++;
					}
					reallocate_ctrl_points = true;
					reset_terrain();
				}
			}
		}

		void draw_terrain(const mat4t &m)
		{
			terrain_shader_.render(m, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(vao_terrain);
			glDrawElements(GL_QUADS, indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		// this is called to draw the world
		void draw_world(int x, int y, int w, int h) {
			static DWORD lastFrameCount = 0;
			static DWORD curFrameCount = 0;
			static DWORD lastTickCount = GetTickCount();
			DWORD tick_count = GetTickCount();
			float frame_time = (tick_count - lastTickCount) / 1000.f;
			lastTickCount = tick_count;
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
			handle_messages();
			// set a viewport - includes whole window area
			glViewport(x, y, w, h);

			// clear the background to black
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			// build a projection matrix: model -> world -> camera -> projection
			// the projection space is the cube -1 <= x/w, y/w, z/w <= 1
			worldToProjection = mat4t::build_projection_matrix(modelToWorld, cc.get_matrix());

			// spin the triangle by rotating about Z (the view direction)
			//modelToWorld.rotateZ(.01);

			// set up opengl to draw flat shaded triangles of a fixed color
			sb.render(worldToProjection, 0);

			draw_terrain(worldToProjection);
			if(draw_normal)
			{
				glBindVertexArray(vao_normal);
				vertex_color_terrain_shader_.render(worldToProjection);
				glDrawArrays(GL_LINES, 0, normal_vertices.size());
				glBindVertexArray(0);
			}

			//*
			water.update(frame_time);
			//render water reflection
			glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
			reflective_worldToProjection = mat4t::build_projection_matrix(modelToWorld, cc_reflection.get_matrix());
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			draw_terrain(reflective_worldToProjection);
			sb.render(reflective_worldToProjection, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//
			water.render(worldToProjection, 0);
			//*/

			if(toggle_ctrl_points)
			{
				glDisable(GL_DEPTH_TEST);
				draw_ctrl_points();
				glEnable(GL_DEPTH_TEST);
			}
		}
	};
}
