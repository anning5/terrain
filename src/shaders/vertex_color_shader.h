
namespace octet {
  class vertex_color_shader : public shader {
    // indices to use with glUniform*()

    // index for model space to projection space matrix
    GLuint modelToProjectionIndex_;

  public:
    void init() {
      // this is the vertex shader. main() will be called for every vertex
      // of every triangle. The output is gl_Position which is used to generate
      // 2D triangles in the rasterizer.
      const char vertex_shader[] = SHADER_STR(
        attribute vec4 pos;
        attribute vec4 color;
        uniform mat4 modelToProjection;
	varying vec4 color_;
        void main() { gl_Position = modelToProjection * pos; color_ = color;}
      );

      // this is the fragment shader. It is called once for every pixel
      // in the rasterized triangles.
      const char fragment_shader[] = SHADER_STR(
	varying vec4 color_;
        void main() {gl_FragColor = color_;}
      );
    
      // compile and link the shaders
      shader::init(vertex_shader, fragment_shader);

      // set up handles to access the uniforms.
      modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
    }

    // start drawing with this shader
    void render(const mat4t &modelToProjection) {
      // start using the program
      shader::render();

      // set the uniforms.
      glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());

      // now we are ready to define the attributes and draw the triangles.
    }
  };
}
