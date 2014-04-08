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

    // index for flat shader emissive color
  public:
    void init() {
      // this is the vertex shader. main() will be called for every vertex
      // of every triangle. The output is gl_Position which is used to generate
      // 2D triangles in the rasterizer.
      const char vertex_shader[] = SHADER_STR(
        attribute vec4 pos;
        attribute vec2 uv;
        uniform mat4 modelToProjection;
				varying vec2 uv_;
				varying vec3 pos_;
				void main() {
					gl_Position = modelToProjection * pos;
					uv_ = uv;
					pos_ = pos.xyz;
				}
      );

      // this is the fragment shader. It is called once for every pixel
      // in the rasterized triangles.
      const char fragment_shader[] = SHADER_STR(
	      varying vec2 uv_;
	      varying vec3 pos_;
	      uniform sampler2D sampler;
	      void main() {
			  gl_FragColor = texture2D(sampler, uv_);
	      }
      );
    
      // compile and link the shaders
      shader::init(vertex_shader, fragment_shader);

      // set up handles to access the uniforms.
      modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
      samplerIndex_ = glGetUniformLocation(program(), "sampler");
    }

    // start drawing with this shader
    void render(const mat4t &modelToProjection, GLuint sampler) {
      // start using the program
      shader::render();

      // set the uniforms.
      glUniform1i(samplerIndex_, sampler);
      glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());

      // now we are ready to define the attributes and draw the triangles.
    }
  };
}
