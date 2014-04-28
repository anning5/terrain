namespace octet {
	class sky_box
	{
		cubemap_shader shader_;
		vec3 vertices[24];
		vec3 uvs[24];
		GLuint cube_texture;
		GLuint vao;
		GLuint vbo;

		sky_box()
		{
			const float HALF_WIDTH = 500.f, offset = 2.f;
			//const float HALF_WIDTH = 20.f, offset = 0;
			//top
			vertices[0] = vec3(-HALF_WIDTH, HALF_WIDTH - offset, -HALF_WIDTH);
			vertices[1] = vec3(HALF_WIDTH, HALF_WIDTH - offset, -HALF_WIDTH);
			vertices[2] = vec3(HALF_WIDTH, HALF_WIDTH - offset, HALF_WIDTH);
			vertices[3] = vec3(-HALF_WIDTH, HALF_WIDTH - offset, HALF_WIDTH);
			uvs[0] = vec3(-1, 1, -1);
			uvs[1] = vec3(1, 1, -1);
			uvs[2] = vec3(1, 1, 1);
			uvs[3] = vec3(-1, 1, 1);

			//back
			vertices[4] = vec3(-HALF_WIDTH, HALF_WIDTH, HALF_WIDTH - offset);
			vertices[5] = vec3(HALF_WIDTH, HALF_WIDTH, HALF_WIDTH - offset);
			vertices[6] = vec3(HALF_WIDTH, -HALF_WIDTH, HALF_WIDTH - offset);
			vertices[7] = vec3(-HALF_WIDTH, -HALF_WIDTH, HALF_WIDTH - offset);
			uvs[4] = vec3(-1, 1, 1);
			uvs[5] = vec3(1, 1, 1);
			uvs[6] = vec3(1, -1, 1);
			uvs[7] = vec3(-1, -1, 1);

			//bottom
			vertices[8] = vec3(-HALF_WIDTH, -HALF_WIDTH + offset, -HALF_WIDTH);
			vertices[9] = vec3(-HALF_WIDTH, -HALF_WIDTH + offset, HALF_WIDTH);
			vertices[10] = vec3(HALF_WIDTH, -HALF_WIDTH + offset, HALF_WIDTH);
			vertices[11] = vec3(HALF_WIDTH, -HALF_WIDTH + offset, -HALF_WIDTH);
			uvs[8] = vec3(-1, -1, -1);
			uvs[9] = vec3(-1, -1, 1);
			uvs[10] = vec3(1, -1, 1);
			uvs[11] = vec3(1, -1, -1);

			//left
			vertices[12] = vec3(-HALF_WIDTH + offset, HALF_WIDTH, HALF_WIDTH);
			vertices[13] = vec3(-HALF_WIDTH + offset, -HALF_WIDTH, HALF_WIDTH);
			vertices[14] = vec3(-HALF_WIDTH + offset, -HALF_WIDTH, -HALF_WIDTH);
			vertices[15] = vec3(-HALF_WIDTH + offset, HALF_WIDTH, -HALF_WIDTH);
			uvs[12] = vec3(-1, 1, 1);
			uvs[13] = vec3(-1, -1, 1);
			uvs[14] = vec3(-1, -1, -1);
			uvs[15] = vec3(-1, 1, -1);

			//right
			vertices[16] = vec3(HALF_WIDTH - offset, HALF_WIDTH, HALF_WIDTH);
			vertices[17] = vec3(HALF_WIDTH - offset, HALF_WIDTH, -HALF_WIDTH);
			vertices[18] = vec3(HALF_WIDTH - offset, -HALF_WIDTH, -HALF_WIDTH);
			vertices[19] = vec3(HALF_WIDTH - offset, -HALF_WIDTH, HALF_WIDTH);
			uvs[16] = vec3(1, 1, 1);
			uvs[17] = vec3(1, 1, -1);
			uvs[18] = vec3(1, -1, -1);
			uvs[19] = vec3(1, -1, 1);

			//front
			vertices[20] = vec3(HALF_WIDTH, HALF_WIDTH, -HALF_WIDTH + offset);
			vertices[21] = vec3(-HALF_WIDTH, HALF_WIDTH, -HALF_WIDTH + offset);
			vertices[22] = vec3(-HALF_WIDTH, -HALF_WIDTH, -HALF_WIDTH + offset);
			vertices[23] = vec3(HALF_WIDTH, -HALF_WIDTH, -HALF_WIDTH + offset);
			uvs[20] = vec3(1, 1, -1);
			uvs[21] = vec3(-1, 1, -1);
			uvs[22] = vec3(-1, -1, -1);
			uvs[23] = vec3(1, -1, -1);

		}

		public:

		static sky_box &get_sky_box()
		{
			static sky_box sb;
			return sb;
		}

		GLuint get_texture()
		{
			return cube_texture;
		}

		GLuint create_cube_map() {
			enum
			{
				F_TOP,
				F_BOTTOM,
				F_LEFT,
				F_RIGHT,
				F_FRONT,
				F_BACK
			};
			dynarray<uint8_t> buffer[6];
			dynarray<uint8_t> image[6];
			app_utils::get_url(buffer[F_TOP], "assets/top.jpg");
			app_utils::get_url(buffer[F_BOTTOM], "assets/bottom.jpg");
			app_utils::get_url(buffer[F_LEFT], "assets/left.jpg");
			app_utils::get_url(buffer[F_RIGHT], "assets/right.jpg");
			app_utils::get_url(buffer[F_FRONT], "assets/front.jpg");
			app_utils::get_url(buffer[F_BACK], "assets/back.jpg");
			uint16_t format[6] = {0};
			uint16_t width[6] = {0};
			uint16_t height[6] = {0};
			for(int i = 0; i < 6; i++)
			{
				const unsigned char *src = &buffer[i][0];
				const unsigned char *src_max = src + buffer[i].size();
				if (buffer[i].size() >= 6 && !memcmp(&buffer[i][0], "GIF89a", 6)) {
					gif_decoder dec;
					dec.get_image(image[i], format[i], width[i], height[i], src, src_max);
				} else if (buffer[i].size() >= 6 && buffer[i][0] == 0xff && buffer[i][1] == 0xd8) {
					jpeg_decoder dec;
					dec.get_image(image[i], format[i], width[i], height[i], src, src_max);
				} else if (buffer[i].size() >= 6 && buffer[i][0] == 0 && buffer[i][1] == 0 && buffer[i][2] == 2) {
					tga_decoder dec;
					dec.get_image(image[i], format[i], width[i], height[i], src, src_max);
				} else {
					printf("warning: unknown texture format\n");
					return 0;
				}

				if (width[i] == 0 || height[i] == 0 || format[i] == 0) {
					printf("failed to create cube map\n");
					return 0;
				}
			}


			glGenTextures(1, &cube_texture);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cube_texture);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format[F_RIGHT], width[F_RIGHT], height[F_RIGHT], 0, format[F_RIGHT], GL_UNSIGNED_BYTE, &image[F_RIGHT][0]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format[F_LEFT], width[F_LEFT], height[F_LEFT], 0, format[F_LEFT], GL_UNSIGNED_BYTE, &image[F_LEFT][0]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format[F_TOP], width[F_TOP], height[F_TOP], 0, format[F_TOP], GL_UNSIGNED_BYTE, &image[F_TOP][0]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format[F_BOTTOM], width[F_BOTTOM], height[F_BOTTOM], 0, format[F_BOTTOM], GL_UNSIGNED_BYTE, &image[F_BOTTOM][0]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format[F_BACK], width[F_BACK], height[F_BACK], 0, format[F_BACK], GL_UNSIGNED_BYTE, &image[F_BACK][0]);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format[F_FRONT], width[F_FRONT], height[F_FRONT], 0, format[F_FRONT], GL_UNSIGNED_BYTE, &image[F_FRONT][0]);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			return cube_texture;
		}

		void init()
		{
			create_cube_map();

			shader_.init();
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, 24 * (sizeof(vec3) + sizeof(vec3)), NULL, GL_STATIC_DRAW);
			int pos_array_size = 24 * sizeof(vec3);
			glBufferSubData(GL_ARRAY_BUFFER, 0, pos_array_size, &vertices[0]);
			glBufferSubData(GL_ARRAY_BUFFER, pos_array_size, 24 * sizeof(vec3), &uvs[0]);

			glEnableVertexAttribArray(attribute_pos);
			glEnableVertexAttribArray(attribute_uv);
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
			glVertexAttribPointer(attribute_uv, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)pos_array_size);
			glBindVertexArray(0);
		}

		void render(const mat4t &modelToProjection, int sampler)
		{
			shader_.render(modelToProjection, sampler);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cube_texture);
			glBindVertexArray(vao);
			glDrawArrays(GL_QUADS, 0, 24);
			glBindVertexArray(0);
		}
	};
}
