namespace octet {
	class water_surface
	{
		terrain_shader shader_;
		vertex_color_shader vertex_color_shader_;
		mat4t modelToWorld;
		dynarray<vec3> vertices;
		dynarray<vec2> uvs;
		dynarray<vec3> normals;
		dynarray<vec3> normal_vertices;
		dynarray<vec3> normal_colors;
		dynarray<unsigned int> indices;
		GLuint texture;
		GLuint vao;
		GLuint vbo;
		GLuint vao_normal;
		GLuint vbo_normal;
		GLuint ibo;
		int width;
		float t;
		float span;
		float last_update_time;
		bool draw_normal;
	public:

		water_surface() :
			t(0),
			span(.1f),
			last_update_time(0),
			draw_normal(false)
		{}

		void set_position(const vec3 &pos)
		{
			modelToWorld.loadIdentity();
			modelToWorld.translate(pos[0], pos[1], pos[2]);
		}

		void toggle_draw_normal()
		{
			draw_normal = !draw_normal;
		}

		void generate_water_surface()
		{
			width = 100;
			int count = width * width;

			vertices.resize(count);
			uvs.resize(count);
			normals.resize(count);
			indices.resize((width - 1) * (width - 1) * 4);
			normal_vertices.resize(count * 2);
			normal_colors.resize(count * 2);
			for(unsigned int i = 0; i < normal_colors.size(); i++)
			{
				normal_colors[i] = vec3(1, 0, 0);
			}


			int index = 0;
			for(int i = 0; i < width; i++)
			{
				float span_z = i * span;
				for(int j = 0; j < width; j++)
				{
					vertices[index] = vec3(j * span, 0, span_z);
					uvs[index] = vec2(1, 1);
					normals[index++] = vec3(0, 1, 0);
				}
			}
			index = 0;
			int index1 = 0;
			for(int i = 0; i < width - 1; i++)
			{
				for(int j = 0; j < width - 1; j++)
				{
					indices[index++] = index1 + j;
					indices[index++] = index1 + width + j;
					indices[index++] = index1 + width + j + 1;
					indices[index++] = index1 + j + 1;
				}
				index1 += width;
			}
		}

		vec3 get_normal(const vec3 &p0, const vec3 &p1, const vec3 &p2)
		{
			vec3 a = p0 - p1;
			vec3 b = p2 - p1;
			return normalize(cross(a, b));
		}

		void update_wave(float t)
		{
			int index = 0;
			for(int i = 0; i < width; i++)
			{
				for(int j = 0; j < width; j++)
				{
					vertices[index][1] = sin(t * 1.f + j * .1f);
					int k = 0;
					if(i > 0)
					{
						if(j > 0)
						{
							k++;
							normals[index] += get_normal(vertices[index - width], vertices[index], vertices[index - 1]);
						}
						if(j < width - 1)
						{
							k++;
							normals[index] += get_normal(vertices[index + 1], vertices[index], vertices[index - width]);
						}
					}
					if(i < width - 1)
					{
						if(j > 0)
						{
							k++;
							normals[index] += get_normal(vertices[index - 1], vertices[index], vertices[index + width]);
						}
						if(j < width - 1)
						{
							k++;
							normals[index] += get_normal(vertices[index + width], vertices[index], vertices[index + 1]);
						}
					}
					normals[index] = normalize(normals[index] / (float)k);
					if(draw_normal)
					{
						normal_vertices[index * 2] = vertices[index];
						normal_vertices[index * 2 + 1] = vertices[index] + normals[index];
					}
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

		void init()
		{
			t = 0;
			last_update_time = 0;
			shader_.init();
			vertex_color_shader_.init();
			generate_water_surface();

      texture = resources::get_texture_handle(GL_RGBA, "#00ffff7f");
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
			int count = width * width;
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

		void render(const mat4t &modelToProjection, int sampler)
		{
			shader_.render(modelToProjection, sampler);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(vao);
			glDrawElements(GL_QUADS, indices.size(), GL_UNSIGNED_INT, 0);
			if(draw_normal)
			{
				glBindVertexArray(vao_normal);
				vertex_color_shader_.render(modelToProjection);
				glDrawArrays(GL_LINES, 0, normal_vertices.size());
			}
			glBindVertexArray(0);
		}
	};
}
