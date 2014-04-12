namespace octet {
	class sky_box
	{
		texture_shader shader_;
		mat4t modelToWorld;
		vec3 vertices[24];
		vec2 uvs[24];
		GLuint texture;
		GLuint vao;
		GLuint vbo;
	public:

		sky_box()
		{
			const float HALF_WIDTH = 500.f;
			//top
			float u = 1.f / 4, du = 1.f / 4, v = 1.f, dv = 1.f / 3, offset = 2.f;
			vertices[0] = vec3(-HALF_WIDTH, HALF_WIDTH - offset, -HALF_WIDTH);
			vertices[1] = vec3(HALF_WIDTH, HALF_WIDTH - offset, -HALF_WIDTH);
			vertices[2] = vec3(HALF_WIDTH, HALF_WIDTH - offset, HALF_WIDTH);
			vertices[3] = vec3(-HALF_WIDTH, HALF_WIDTH - offset, HALF_WIDTH);
			uvs[0] = vec2(u, v);
			uvs[1] = vec2(u + du, v);
			uvs[2] = vec2(u + du, v - dv);
			uvs[3] = vec2(u, v - dv);

			//back
			v -= dv;
			vertices[4] = vec3(-HALF_WIDTH, HALF_WIDTH, HALF_WIDTH - offset);
			vertices[5] = vec3(HALF_WIDTH, HALF_WIDTH, HALF_WIDTH - offset);
			vertices[6] = vec3(HALF_WIDTH, -HALF_WIDTH, HALF_WIDTH - offset);
			vertices[7] = vec3(-HALF_WIDTH, -HALF_WIDTH, HALF_WIDTH - offset);
			uvs[4] = vec2(u, v);
			uvs[5] = vec2(u + du, v);
			uvs[6] = vec2(u + du, v - dv);
			uvs[7] = vec2(u, v - dv);

			//bottom
			v -= dv;
			vertices[8] = vec3(-HALF_WIDTH, -HALF_WIDTH + offset, -HALF_WIDTH);
			vertices[9] = vec3(-HALF_WIDTH, -HALF_WIDTH + offset, HALF_WIDTH);
			vertices[10] = vec3(HALF_WIDTH, -HALF_WIDTH + offset, HALF_WIDTH);
			vertices[11] = vec3(HALF_WIDTH, -HALF_WIDTH + offset, -HALF_WIDTH);
			uvs[8] = vec2(u, v - dv);
			uvs[9] = vec2(u, v);
			uvs[10] = vec2(u + du, v);
			uvs[11] = vec2(u + du, v - dv);

			//left
			u -= du;
			v += dv;
			vertices[12] = vec3(-HALF_WIDTH + offset, HALF_WIDTH, HALF_WIDTH);
			vertices[13] = vec3(-HALF_WIDTH + offset, -HALF_WIDTH, HALF_WIDTH);
			vertices[14] = vec3(-HALF_WIDTH + offset, -HALF_WIDTH, -HALF_WIDTH);
			vertices[15] = vec3(-HALF_WIDTH + offset, HALF_WIDTH, -HALF_WIDTH);
			uvs[12] = vec2(u + du, v);
			uvs[13] = vec2(u + du, v - dv);
			uvs[14] = vec2(u, v - dv);
			uvs[15] = vec2(u, v);

			//right
			u += 2 * du;
			vertices[16] = vec3(HALF_WIDTH - offset, HALF_WIDTH, HALF_WIDTH);
			vertices[17] = vec3(HALF_WIDTH - offset, HALF_WIDTH, -HALF_WIDTH);
			vertices[18] = vec3(HALF_WIDTH - offset, -HALF_WIDTH, -HALF_WIDTH);
			vertices[19] = vec3(HALF_WIDTH - offset, -HALF_WIDTH, HALF_WIDTH);
			uvs[16] = vec2(u, v);
			uvs[17] = vec2(u + du, v);
			uvs[18] = vec2(u + du, v - dv);
			uvs[19] = vec2(u, v - dv);

			//front
			u += du;
			vertices[20] = vec3(HALF_WIDTH, HALF_WIDTH, -HALF_WIDTH + offset);
			vertices[21] = vec3(-HALF_WIDTH, HALF_WIDTH, -HALF_WIDTH + offset);
			vertices[22] = vec3(-HALF_WIDTH, -HALF_WIDTH, -HALF_WIDTH + offset);
			vertices[23] = vec3(HALF_WIDTH, -HALF_WIDTH, -HALF_WIDTH + offset);
			uvs[20] = vec2(u, v);
			uvs[21] = vec2(u + du, v);
			uvs[22] = vec2(u + du, v - dv);
			uvs[23] = vec2(u, v - dv);

			modelToWorld.loadIdentity();

		}

		void set_position(const vec3 &pos)
		{
			modelToWorld.loadIdentity();
			modelToWorld.translate(pos[0], pos[1], pos[2]);
		}

		void init(char *path)
		{
			shader_.init();
      texture = resources::get_texture_handle(GL_RGB, path);
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, 24 * 5 * sizeof(float), NULL, GL_STATIC_DRAW);
			int pos_array_size = 24 * 3 * sizeof(float);
			glBufferSubData(GL_ARRAY_BUFFER, 0, pos_array_size, &vertices[0]);
			glBufferSubData(GL_ARRAY_BUFFER, pos_array_size, 24 * 2 * sizeof(float), &uvs[0]);

			glEnableVertexAttribArray(attribute_pos);
			glEnableVertexAttribArray(attribute_uv);
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)pos_array_size);
			glBindVertexArray(0);
		}

		void render(const mat4t &modelToProjection, int sampler)
		{
			shader_.render(modelToWorld * modelToProjection, sampler);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(vao);
			glDrawArrays(GL_QUADS, 0, 24);
			glBindVertexArray(0);
		}
	};
}
