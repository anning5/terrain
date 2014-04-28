namespace octet {
	vec3 get_normal(const vec3 &p0, const vec3 &p1, const vec3 &p2)
	{
		vec3 a = p0 - p1;
		vec3 b = p2 - p1;
		return normalize(cross(a, b));
	}

	class water_surface
	{
		water_shader water_shader_;
		vertex_color_shader vertex_color_shader_;
		mat4t modelToWorld;
		vec3 position;
		dynarray<vec3> vertices;
		dynarray<vec2> uvs;
		dynarray<vec3> normals;
		dynarray<vec3> normal_vertices;
		dynarray<vec3> normal_colors;
		dynarray<unsigned int> indices;

		dynarray<vec2> directions;
		dynarray<float> Qs;
		dynarray<float> Amplitudes;
		dynarray<float> QAs; // = Q * Amplitude
		dynarray<float> wave_lengths;// L
		dynarray<float> ws; // w = 2 * PI / L
		dynarray<float> wave_speeds; // S
		dynarray<float> ps; // phase-constant = S * 2 * PI / L

		sky_box &sb;
		GLuint texture;
		GLuint vao;
		GLuint vbo;
		GLuint vao_normal;
		GLuint vbo_normal;
		GLuint frame_buffer;
		GLuint scene_texture;
		GLuint depth_render_buffer;
		GLuint ibo;
		float width;
		float t;
		int resolution;
		float last_update_time;
		bool draw_normal;
	public:

		water_surface(sky_box &s) :
			t(0),
			resolution(100),
			last_update_time(0),
			draw_normal(false),
			sb(s)
		{
		}

		void set_position(const vec3 &pos)
		{
			position = pos;
		}

		void toggle_draw_normal()
		{
			draw_normal = !draw_normal;
		}

		void generate_water_surface()
		{
			int count = resolution * resolution;

			vertices.resize(count);
			uvs.resize(count);
			normals.resize(count);
			indices.resize((resolution - 1) * (resolution - 1) * 4);
			normal_vertices.resize(count * 2);
			normal_colors.resize(count * 2);
			for(unsigned int i = 0; i < normal_colors.size(); i++)
			{
				normal_colors[i] = vec3(1, 0, 0);
			}

			float du = 1.f / (resolution - 1), v = 0, dv = du;

			int index = 0;
			for(int i = 0; i < resolution; i++)
			{
				float u = 0;
				for(int j = 0; j < resolution; j++)
				{
					uvs[index++] = vec2(u, v);
					u += du;
				}
				v += dv;
			}
			index = 0;
			int index1 = 0;
			for(int i = 0; i < resolution - 1; i++)
			{
				for(int j = 0; j < resolution - 1; j++)
				{
					indices[index++] = index1 + j;
					indices[index++] = index1 + resolution + j;
					indices[index++] = index1 + resolution + j + 1;
					indices[index++] = index1 + j + 1;
				}
				index1 += resolution;
			}
		}

		void update_wave(float t)
		{
			int index = 0;
			float half_width = width * .5f;
			float z = -half_width;
			float span = width / resolution;
			for(int i = 0; i < resolution; i++)
			{
				float x = -half_width;
				for(int j = 0; j < resolution; j++)
				{
					//vertices[index][1] = sin(t * 1.f + j * .1f);
					vec3 &v = vertices[index++];
					float sum_x = position[0], sum_y = position[1], sum_z = position[2];
					for(unsigned int k = 0; k < directions.size(); k++)
					{
						float value = ws[k] * dot(directions[k], vec2(x, z)) + ps[k] * t;
						float cos_value = cos(value);
						sum_x += QAs[k] * directions[k][0] * cos_value;
						sum_z += QAs[k] * directions[k][1] * cos_value;
						sum_y += Amplitudes[k] * sin(value);
					}
					//*
					v[0] = x + sum_x;
					v[2] = z + sum_z;
					v[1] = sum_y;
					//*/
					/*
					v[0] = x;
					v[2] = z + 5.f;
					v[1] = position[1];
					//*/
					x += span;
				}
				z += span;
			}
			index = 0;
			vec3 sum;
			for(int i = 0; i < resolution; i++)
			{
				for(int j = 0; j < resolution; j++)
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
							sum += get_normal(vertices[index - resolution], vertices[index], vertices[index - 1]);
						}
						if(j < resolution - 1)
						{
							k++;
							sum += get_normal(vertices[index + 1], vertices[index], vertices[index - resolution]);
						}
					}
					if(i < resolution - 1)
					{
						if(j > 0)
						{
							k++;
							sum += get_normal(vertices[index - 1], vertices[index], vertices[index + resolution]);
						}
						if(j < resolution - 1)
						{
							k++;
							sum += get_normal(vertices[index + resolution], vertices[index], vertices[index + 1]);
						}
					}
					normals[index] = normalize(sum / (float)k);
					normal_vertices[index * 2] = vertices[index];
					normal_vertices[index * 2 + 1] = vertices[index] + normals[index];
					index++;
				}
			}

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vec3), &vertices[0]);
			glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * (sizeof(vec3) + sizeof(vec2)), normals.size() * sizeof(vec3), &normals[0]);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
			glBufferSubData(GL_ARRAY_BUFFER, 0, normal_vertices.size() * sizeof(vec3), &normal_vertices[0]);
		}

		void update(float frame_time)
		{
			t += frame_time;
			const float TIME_STEP = .01f;
			if(t - last_update_time > TIME_STEP)
			{
				last_update_time = t;
				update_wave(t);
			}
		}

		void wave_parameters_init()
		{
			directions.push_back(normalize(vec2(0, 1.f)));
			Qs.push_back(.8f);
			Amplitudes.push_back(.02f);
			wave_lengths.push_back(2);
			wave_speeds.push_back(2);

			directions.push_back(normalize(vec2(1.f, 0)));
			Qs.push_back(.8f);
			Amplitudes.push_back(.02f);
			wave_lengths.push_back(2);
			wave_speeds.push_back(2);

			directions.push_back(normalize(vec2(1.f, 1.f)));
			Qs.push_back(.8f);
			Amplitudes.push_back(.02f);
			wave_lengths.push_back(2);
			wave_speeds.push_back(2);

			directions.push_back(normalize(vec2(-1.f, 1.f)));
			Qs.push_back(.8f);
			Amplitudes.push_back(.02f);
			wave_lengths.push_back(2);
			wave_speeds.push_back(2);

			for(unsigned int i = 0; i < Amplitudes.size(); i++)
			{
				QAs.push_back(Amplitudes[i] * Qs[i]);
				ws.push_back(2 * 3.14159265f / wave_lengths[i]);
				ps.push_back(wave_speeds[i] * ws[i]);
			}
		}

		void init(const vec3 &v, float width, GLuint scene_texture)
		{
			this->width = width;
			this->scene_texture = scene_texture;
			set_position(v);
			wave_parameters_init();
			t = 0;
			last_update_time = 0;
			water_shader_.init();
			vertex_color_shader_.init();
			generate_water_surface();

      texture = resources::get_texture_handle(GL_RGBA, "#7fffff4f");
			glGenVertexArrays(1, &vao_normal);
			glBindVertexArray(vao_normal);
			glGenBuffers(1, &vbo_normal);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
			glBufferData(GL_ARRAY_BUFFER, normal_vertices.size() * sizeof(vec3) + normal_colors.size() * sizeof(vec3), NULL, GL_STATIC_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER, normal_vertices.size() * sizeof(vec3), normal_colors.size() * sizeof(vec3), &normal_colors[0]);
			glEnableVertexAttribArray(attribute_pos);
			glEnableVertexAttribArray(attribute_color);
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
			glVertexAttribPointer(attribute_color, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(normal_vertices.size() * sizeof(vec3)));
			glBindVertexArray(0);


			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			int count = resolution * resolution;
			int pos_array_size = count * sizeof(vec3);
			int uv_array_size = count * sizeof(vec2);
			int normal_array_size = count * sizeof(vec3);
			glBufferData(GL_ARRAY_BUFFER, pos_array_size + uv_array_size + normal_array_size, NULL, GL_STATIC_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER, 0, pos_array_size, &vertices[0]);
			glBufferSubData(GL_ARRAY_BUFFER, pos_array_size, uv_array_size, &uvs[0]);
			glBufferSubData(GL_ARRAY_BUFFER, pos_array_size + uv_array_size, normal_array_size, &normals[0]);

			glGenBuffers(1, &ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

			glEnableVertexAttribArray(attribute_pos);
			glEnableVertexAttribArray(attribute_uv);
			glEnableVertexAttribArray(attribute_normal);
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)pos_array_size);
			glVertexAttribPointer(attribute_normal, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(pos_array_size + uv_array_size));
			glBindVertexArray(0);
		}

		void render(const mat4t &worldToProjection, const vec3 &camera)
		{
			mat4t m = worldToProjection;
			water_shader_.render(m, camera);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, sb.get_texture());
			glBindVertexArray(vao);
			glDrawElements(GL_QUADS, indices.size(), GL_UNSIGNED_INT, 0);
			if(draw_normal)
			{
				glBindVertexArray(vao_normal);
				vertex_color_shader_.render(m);
				glDrawArrays(GL_LINES, 0, normal_vertices.size());
			}
			glBindVertexArray(0);
		}
	};
}
