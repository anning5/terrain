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
  class terrain_shader : public shader {
    // indices to use with glUniform*()

    // index for model space to projection space matrix
    GLuint modelToProjectionIndex_;
    GLuint samplerIndex_;
    GLuint light_dir_;

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
				varying vec3 normal_;
				varying vec3 pos_;
				void main() {
					gl_Position = modelToProjection * pos;
					uv_ = uv;
					normal_ = normal;
					pos_ = pos.xyz;
				}
      );

      // this is the fragment shader. It is called once for every pixel
      // in the rasterized triangles.
      const char fragment_shader[] = SHADER_STR(
	      varying vec2 uv_;
	      varying vec3 pos_;
	      varying vec3 normal_;
	      uniform sampler2D sampler;
	      uniform vec3 light_dir;
	      void main() {
			  gl_FragColor = texture2D(sampler, uv_);
				gl_FragColor.xyz = gl_FragColor.xyz * clamp(dot(normalize(light_dir), normalize(normal_)), 0.f, 1.f);
				//gl_FragColor.w = gl_FragColor.w * clamp(dot(normalize(light_dir), normalize(normal_)), 0.f, 1.f);
	      }
      );
    
      // compile and link the shaders
      shader::init(vertex_shader, fragment_shader);

      // set up handles to access the uniforms.
      modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
      samplerIndex_ = glGetUniformLocation(program(), "sampler");
      light_dir_ = glGetUniformLocation(program(), "light_dir");
    }

    // start drawing with this shader
    void render(const mat4t &modelToProjection, GLuint sampler) {
      // start using the program
      shader::render();

      // set the uniforms.
      glUniform1i(samplerIndex_, sampler);
      glUniform3f(light_dir_, 0, 1, 0);
      glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());

      // now we are ready to define the attributes and draw the triangles.
    }
  };
}
