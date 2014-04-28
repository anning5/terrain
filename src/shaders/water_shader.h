////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012, 2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Solid color shader
//
// shader which renders with a solid color

namespace octet {
  class water_shader : public shader {
    // indices to use with glUniform*()

    // index for model space to projection space matrix
    GLuint modelToProjectionIndex_;
    GLuint sampler_;
    GLuint cube_sampler_;
    GLuint light_dir_;
    GLuint camera_;

    // index for flat shader emissive color
  public:
    void init() {
      // this is the vertex shader. main() will be called for every vertex
      // of every triangle. The output is gl_Position which is used to generate
      // 2D triangles in the rasterizer.
      const char vertex_shader[] = SHADER_STR(
        attribute vec4 pos;
        attribute vec2 uv;
        attribute vec3 normal;
        uniform mat4 modelToProjection;
				varying vec2 uv_;
				varying vec3 srt_;
				varying vec3 normal_;
				varying vec3 pos_;
				void main() {
					gl_Position = modelToProjection * pos;
					normal_ = normal;
					pos_ = pos.xyz;
					uv_ = uv;
				}
      );

      // this is the fragment shader. It is called once for every pixel
      // in the rasterized triangles.
      const char fragment_shader[] = SHADER_STR(
	      varying vec2 uv_;
	      varying vec3 srt_;
	      varying vec3 pos_;
	      varying vec3 normal_;
	      uniform sampler2D sampler;
	      uniform samplerCube cube_sampler;
	      uniform vec3 light_dir;
				uniform vec3 camera;
	      void main() {
			  gl_FragColor = texture2D(sampler, uv_);
				vec3 v = reflect(normalize(pos_ - camera), normalize(normal_));
				float half_width = 20.f;
				/*
				float k = 0;
				if(abs(v.x) > abs(v.y))
				{
					if(abs(v.x) > abs(v.z))
					{
							k = ((v.x > 0 ? 1 : -1) * half_width - pos_.x) / v.x;
					}
					else
					{
							k = ((v.z > 0 ? 1 : -1) * half_width - pos_.z) / v.z;
					}
				}
				else
				{
					if(abs(v.y) > abs(v.z))
					{
							k = ((v.y > 0 ? 1 : -1) * half_width - pos_.y) / v.y;
					}
					else
					{
							k = ((v.z > 0 ? 1 : -1) * half_width - pos_.z) / v.z;
					}
				}
				v = pos_ + k * v;
				//*/
			  gl_FragColor.xyz = textureCube(cube_sampler, v).xyz * clamp(dot(normalize(light_dir), normalize(normal_)), 0.f, 1.f);
				//gl_FragColor.xyz = texture2D(cube_sampler, vec2(.5f * (uv_reflection.x / uv_reflection.w + 1), .5f - .5f * (uv_reflection.y / uv_reflection.w))).xyz * ;
				//gl_FragColor.w = gl_FragColor.w * clamp(dot(normalize(light_dir), normalize(normal_)), 0.f, 1.f);
	      }
      );
    
      // compile and link the shaders
      shader::init(vertex_shader, fragment_shader);

      // set up handles to access the uniforms.
      modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
			camera_ = glGetUniformLocation(program(), "camera");
      sampler_ = glGetUniformLocation(program(), "sampler");
      cube_sampler_ = glGetUniformLocation(program(), "cube_sampler");
      light_dir_ = glGetUniformLocation(program(), "light_dir");
    }

    // start drawing with this shader
    void render(const mat4t &modelToProjection, const vec3 &camera) {
      // start using the program
      shader::render();

      // set the uniforms.
      glUniform1i(sampler_, 0);
      glUniform1i(cube_sampler_, 1);
      glUniform3f(light_dir_, 0, 1, 0);
      glUniform3fv(camera_, 1, &camera[0]);
      glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());

      // now we are ready to define the attributes and draw the triangles.
    }
  };
}
