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

		int waveType;
		int numWaves;
		float speedMultipler;
		float waveLength;
		float amplitude;
		int steepness;

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
						float value = (ws[k] + waveLength) * dot(directions[k], vec2(x, z)) + (ps[k] * t) * speedMultipler;
						float cos_value = cos(value);
						sum_x += QAs[k] * directions[k][0] * cos_value;
						sum_z += QAs[k] * directions[k][1] * cos_value;
						sum_y += amplitude * Amplitudes[k] * pow(((sin(value) / 2)),steepness);
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
			waveType = 0;
			numWaves = 4;
			speedMultipler = 1;
			waveLength = 1;
			amplitude = 2;
			steepness = 1;

			directions.push_back(normalize(vec2(0, 1)));
			Qs.push_back(0.5f) ;
			Amplitudes.push_back(.05f);
			wave_lengths.push_back(5);
			wave_speeds.push_back(2);

			directions.push_back(normalize(vec2(1, 0)));
			Qs.push_back(0.5f);
			Amplitudes.push_back(.05f);
			wave_lengths.push_back(2);
			wave_speeds.push_back(2);

			directions.push_back(normalize(vec2(1, 1)));
			Qs.push_back(0.5f);
			Amplitudes.push_back(.05f);
			wave_lengths.push_back(2);
			wave_speeds.push_back(2);

			directions.push_back(normalize(vec2(-1, 1)));
			Qs.push_back(0.5f);
			Amplitudes.push_back(.05f);
			wave_lengths.push_back(2);
			wave_speeds.push_back(2);

			for(int i = 0; i < numWaves; i++)
			{
				ws.push_back(2 * 3.14159265f / wave_lengths[i] );
				ps.push_back(wave_speeds[i] * ws[i]);
				//Qs.push_back(steepness/(ws[i] * Amplitudes[i] * numWaves));
				QAs.push_back(Amplitudes[i] * Qs[i]);				
			}
		}

		float lerp(float time, float value_A, float value_B){
			return (1-time) * value_A + time * value_B;
		}

		void recalculate(){
			for(int i = 0; i < numWaves; i++)
			{
				ws[i] = (2 * 3.14159265f / wave_lengths[i] );
				ps[i] = (wave_speeds[i] * ws[i]);
				Qs[i] = (steepness/(ws[i] * Amplitudes[i] * numWaves));
				QAs[i] = (Amplitudes[i] * Qs[i]);				
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

		//Setters: Speed / Wave Length / Amplitude / Steepness
		void setWaveSpeed(int speed)
		{
			if(speed == 1){
				speedMultipler += 0.01f;
				printf("\n Current Speed: %f", speedMultipler);
			} else if (speed == 0){
				speedMultipler -= 0.01f;
				printf("\n Current Speed: %f", speedMultipler);
			}
		}

		void setWaveLength(int _waveLength)
		{
			if(_waveLength == 1){
				waveLength += 0.01f;
				printf("\n Current WaveLength: %f", waveLength);
			} else if (_waveLength == 0){
				waveLength -= 0.01f;
				printf("\n Current WaveLength: %f", waveLength);
			}
		}

		void setWaveAmplitude(int _amplitude)
		{
			if(_amplitude == 1){
				amplitude += 0.1f;
				printf("\n Current Amplitude: %f", amplitude);
			} else if (_amplitude == 0){
				amplitude -= 0.1f;
				printf("\n Current Amplitude: %f", amplitude);
			}
		}

		void setWaveSteepness(int _steepness)
		{
			if(_steepness == 1 && steepness < 4){
				steepness += 1;
				printf("\n Current Steepness: %d", steepness);
			} else if (_steepness == 0 && steepness > 0){
				steepness -= 1;
				printf("\n Current Steepness: %d", steepness);
			}
		}

		void waves(int type){
			if(type == 1){
				waveType += 1;
			} else if (type == 0){
				waveType -= 1;
			}

			if(waveType >= 4){ waveType = 4;}
			if(waveType < 0){ waveType = 1;}


			if(waveType == 4){
				printf("\n Wave Type: %d : Diagonal Beach waves", waveType);

#pragma region Diagonal Beach Waves
				//waveLength = 1;
				//amplitude = 2;
				//steepness = 1;
				//speedMultipler = 0.2f;
				//
				//directions[0] = (normalize(vec2(0, 1)));
				//Qs[0] = (0.5f) ;
				//Amplitudes[0] = (.05f);
				//wave_lengths[0] = (5);
				//wave_speeds[0] = (2);
				//
				//directions[1] = (normalize(vec2(-1, 1)));
				//Qs[1] = (0.5f);
				//Amplitudes[1] = (.05f);
				//wave_lengths[1] = (2);
				//wave_speeds[1] = (2);
				//
				//directions[2] = (normalize(vec2(1, 0)));
				//Qs[2] = (0.5f);
				//Amplitudes[2] =(.05f);
				//wave_lengths[2] = (2);
				//wave_speeds[2] = (2);
				//
				//directions[3] = (normalize(vec2(-1, 0)));
				//Qs[3] = (0.5f);
				//Amplitudes[3] = (.20f);
				//wave_lengths[3] = (1);
				//wave_speeds[3] = (2);

				float i = 0.0f;
				float transitionTime = 10.0f;
				float rate = 1.0f/transitionTime;

				while(i < 1.0f)
				{
					i+= t * rate;
				directions[0] = (normalize(vec2(0, 1)));
				Qs[0] = lerp(i, Qs[0],(0.5f)) ;
				Amplitudes[0] = lerp(i,Amplitudes[0], (.05f));
				wave_lengths[0] = lerp(i,wave_lengths[0],(5));
				wave_speeds[0] = lerp(i,wave_speeds[0], (2));

				directions[1] = (normalize(vec2(-1, 1)));
				Qs[1] = lerp(i, Qs[1],(0.5f)) ;
				Amplitudes[1] = lerp(i,Amplitudes[1], (.05f));
				wave_lengths[1] = lerp(i,wave_lengths[1],(2));
				wave_speeds[1] = lerp(i,wave_speeds[1], (2));

				directions[2] = (normalize(vec2(1, 0)));
				Qs[2] = lerp(i, Qs[2],(0.5f)) ;
				Amplitudes[2] = lerp(i,Amplitudes[2], (.05f));
				wave_lengths[2] = lerp(i,wave_lengths[2],(2));
				wave_speeds[2] = lerp(i,wave_speeds[2], (2));

				directions[3] = (normalize(vec2(0, 1)));
				Qs[3] = lerp(i, Qs[3],(0.5f)) ;
				Amplitudes[3] = lerp(i,Amplitudes[3], (.20f));
				wave_lengths[3] = lerp(i,wave_lengths[3],(1));
				wave_speeds[3] = lerp(i,wave_speeds[3], (2));


#pragma endregion
					recalculate();

				}
			}
			if(waveType == 3){
				printf("\n Wave Type: %d : Horizontal Rolling Waves", waveType);

#pragma region Horizontal Rolling Waves
				waveLength = 1;
				amplitude = 2;
				steepness = 1;
				speedMultipler = 0.2f;

				directions[0] = (normalize(vec2(0, 1)));
				Qs[0] = (0.5f) ;
				Amplitudes[0] = (.05f);
				wave_lengths[0] = (5);
				wave_speeds[0] = (2);

				directions[1] = (normalize(vec2(1, 0)));
				Qs[1] = (0.5f);
				Amplitudes[1] = (.05f);
				wave_lengths[1] = (2);
				wave_speeds[1] = (2);

				directions[2] = (normalize(vec2(1, 1)));
				Qs[2] = (0.5f);
				Amplitudes[2] =(.05f);
				wave_lengths[2] = (2);
				wave_speeds[2] = (2);

				directions[3] = (normalize(vec2(-1, 0)));
				Qs[3] = (0.5f);
				Amplitudes[3] = (.05f);
				wave_lengths[3] = (1);
				wave_speeds[3] = (2);
#pragma endregion
				recalculate();
			}
			if(waveType == 2){
				printf("\n Wave Type: %d : Ocean Wave", waveType);

#pragma region Ocean Wave
				speedMultipler = 0.4f;
				waveLength = -0.72f;
				amplitude = 2.69f;
				steepness = 3;

				directions[0] = (normalize(vec2(0, 1)));
				directions[1] = (normalize(vec2(1, 0)));
				directions[2] = (normalize(vec2(1, 1)));
				directions[3] = (normalize(vec2(-1, 1)));

				Qs[0] = 0.5f;
				Qs[1] = 0.5f;
				Qs[2] = 0.5f;
				Qs[3] = 0.5f;

				Amplitudes[0] = 0.5f;
				Amplitudes[1] = 0.5f;
				Amplitudes[2] = 0.5f;
				Amplitudes[3] = 0.5f;

				wave_lengths[0] = 2;
				wave_lengths[1] = 2;
				wave_lengths[2] = 2;
				wave_lengths[3] = 2;

				wave_speeds[0] = 2;
				wave_speeds[1] = 2;
				wave_speeds[2] = 2;
				wave_speeds[3] = 2;


#pragma endregion
				recalculate();
			}
			if(waveType == 1){
				printf("\n Wave Type: %d : Rain Drop", waveType);

#pragma region Rain Drop
				speedMultipler = -0.85f;
				waveLength = -7.62f;
				amplitude = -4.09f;
				steepness = 1;

				directions[0] = (normalize(vec2(0, 1)));
				Qs[0] = (0.5f) ;
				Amplitudes[0] = (.05f);
				wave_lengths[0] = (2);
				wave_speeds[0] = (2);

				directions[1] = (normalize(vec2(1, 0)));
				Qs[1] = (0.5f);
				Amplitudes[1] = (.05f);
				wave_lengths[1] = (2);
				wave_speeds[1] = (2);

				directions[2] = (normalize(vec2(1, 1)));
				Qs[2] = (0.5f);
				Amplitudes[2] = (.05f);
				wave_lengths[2] = (2);
				wave_speeds[2] = (2);

				directions[3] = (normalize(vec2(-1, 1)));
				Qs[3] = (0.5f);
				Amplitudes[3] = (.05f);
				wave_lengths[3] = (2);
				wave_speeds[3] = (2);

#pragma endregion
				recalculate();
			}
		}

		void debugParamter(){
			printf("\n WaveType: %d, WaveLenght: %f, Amplitude: %f, Steepness: %d, Speed: %f", waveType, waveLength, amplitude, steepness, speedMultipler);
		}
	};
}
