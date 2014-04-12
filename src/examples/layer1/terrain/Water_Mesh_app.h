//Reading from file: http://www.cplusplus.com/forum/beginner/80748/
/*

Notation:
WaveLength  (L) : Crest to crest distance between waves in world  space.
Frequency   (w) : How often to generate each wave.
Speed		(S) : The distance the crest moves forward per second.
Amplitude	(A) : The height / peak from the water plane.

*/
#define M_PI       3.14159265358979323846f
#define M_PI_2     1.57079632679489661923f
#define M_PI_4     0.785398163397448309616f

namespace octet{
	class Water_Square
	{
		float vertices[12];		//Each Square consist of 4 points. Each point has 3 floats to represent the position (X,Y,Z)

	public:
		Water_Square(){

		}
		
		void Render()
		{
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)vertices );
			glEnableVertexAttribArray(attribute_pos);

			glDrawArrays(GL_QUADS, 0, 4);
		}	  



		void setP0(vec3 &value){
			vertices[0] = value.x(); vertices[1] = value.y(); vertices[2] = value.z();
		}

		vec3 &getP0(){
			return (vec3 &)vertices[0];
		}

		void setP1(vec3 &value){
			vertices[3] = value.x(); vertices[4] = value.y(); vertices[5] = value.z();
		}

		vec3 &getP1(){
			return (vec3 &)vertices[3];
		}
		void setP2(vec3 &value){
			vertices[6] = value.x(); vertices[7] = value.y(); vertices[8] = value.z();
		}

		vec3 &getP2(){
			return (vec3 &)vertices[6];
		}

		void setP3(vec3 &value){
			vertices[9] = value.x(); vertices[10] = value.y(); vertices[11] = value.z();
		}

		vec3 &getP3(){
			return (vec3 &)vertices[9];
		}
	};


	//Main Class!
	class Water_Mesh_app : public octet::app {
		mat4t modelToWorld;
		mat4t cameraToWorld;

		// shader to draw a solid color
		color_shader color_shader_;
		Water_Square *squaresXZ[1000][1000];
		Water_Square *squares[12000];
		int waterSize;
		float time;
		//Wave Characteristic
		float waveLength;
		float amplitude;
		float steepness;
		float speed;
		int numWaves;
		vec4 waves[4];
		vec2 direction[4];
	public:
		//Keyboard 
		int cameraX, cameraY,cameraZ;
		vec4 color;
		float red,green,blue;
		float squareWidth;
		float lineWidth;
		//Constructor
		Water_Mesh_app(int argc, char **argv) : app(argc, argv) {

		}

		// this is called once OpenGL is initialized
		void app_init() {

			cameraX = 50;	  cameraY = 10;	  cameraZ = 150;
			// initialize the shader
			color_shader_.init();
			modelToWorld.loadIdentity();
			// put the camera a short distance from the center, looking towards the triangle
			cameraToWorld.loadIdentity();
			cameraToWorld.translate(cameraX, cameraY, cameraZ);
			modelToWorld.rotateX(45);
			//Colors
			red = 0.f;	  green = 1.0f;	  blue = 0.f;
			color = vec4(red, green, blue, 0.5);
			
			

			//Wave Characteristic
			waveLength =  0.02f;								//(L)
			amplitude = 2.0f;
			speed = 0.1f;
			steepness = 0.0f;
			numWaves = 1;
			waterSize = 50;
			squareWidth = 5;

			//Wave Parameters: WaveLength, Amplitude, Speed, Wave Steepness
			waves[0] =			vec4(0.1f,	0.5f,	0.1f, 1);
			waves[1] =			vec4(0.2f,	1.0f,	0.4f, 1.5f);
			waves[2] =			vec4(1.3f,	2.0f,	0.5f, 2);
			waves[3] =			vec4(1.4f,	3.0f,	1.1f, 2.5f);

			direction[0] = vec2(1,1).normalize();
			direction[1] = vec2(2,3).normalize();
			direction[2] = vec2(5,6).normalize();
			direction[3] = vec2(1,1).normalize();
		
			time = 0.0f;

			//Attempts for Gerstner Wave
			for(int z=0; z < waterSize; z++){
				for(int x=0; x < waterSize; x++){
					squaresXZ[x][z] = new Water_Square();
					squaresXZ[x][z]->setP0(vec3(x * squareWidth		 , 0 , 0));
					squaresXZ[x][z]->setP1(vec3((x + 1) * squareWidth	, 0 , 0));
					squaresXZ[x][z]->setP2(vec3((x + 1) * squareWidth	, 0 , (z - 1) * squareWidth));
					squaresXZ[x][z]->setP3(vec3(x * squareWidth		 , 0 , (z - 1) * squareWidth));
				}
			}

			for(int i=0; i<numWaves; i++){
				printf("\n\n Wave: %d \n WaveLength: %f \n Amplitude: %f \n Speed: %f \n Steepness: %f \n", i, waves[i].x(),waves[i].y(),waves[i].z(),waves[i].w());
			}
		}



		// this is called to draw the world
		void draw_world(int x, int y, int w, int h) {
			// set a viewport - includes whole window area
			glViewport(x, y, w, h);

			// clear the background to black
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// allow Z buffer depth testing (closer objects are always drawn in front of far ones)
			glEnable(GL_DEPTH_TEST);

			//Shader Section
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);
			// set up opengl to draw flat shaded triangles of a fixed color
			vec4 color(0, 0, 1, 1);
			color_shader_.render(modelToProjection, color);
			//Camera Control Section
			simulate();	
			setCameraToWorld(cameraX, cameraY, cameraZ);
			
			time += 0.1f;
			
			for(int z=0; z < waterSize; z++){
				for(int x=0; x < waterSize; x++){
				
					float y = 0;

					y = GerstnerWaveFunction(x , y, z).y();
					squaresXZ[x][z]->setP0(vec3(x * squareWidth, y , z));
					
					y = GerstnerWaveFunction(x , y, z).y();
					squaresXZ[x][z]->setP1(vec3((x + 1) * squareWidth, y , z));

					y = GerstnerWaveFunction(x , y, z).y();
					squaresXZ[x][z]->setP2(vec3((x + 1) * squareWidth, y , (z - squareWidth) * squareWidth));
					
					y = GerstnerWaveFunction(x , y, z).y();
					squaresXZ[x][z]->setP3(vec3(x * squareWidth , y , (z - squareWidth) * squareWidth));
				}
			}
			
			for(int z=0; z < waterSize; z++){
				for(int x=0; x < waterSize; x++){
					squaresXZ[x][z]->Render();
				}
			}

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		//Helper Function
		vec3 GerstnerWaveFunction(float x, float y, float z){
			vec3 result = vec3(0,0,0);
			//X
			for(int i=0; i<numWaves; i++){
				float temp = 0;
				//			Qi				Ai				Di.x				wi					Di			(x,y)			Speed * Time
				temp = waves[i].w() * waves[i].y() * direction[i].x() * cos( waves[i].x() * direction[i].dot(vec2(x,z)) + (waves[i].z() * time) );
				result.x() += temp;
			}
			result.x() += x;

			//Z
			for(int i=0; i<numWaves; i++){
				float temp = 0;
				//			Qi				Ai				Di.x				wi					Di			(x,y)			Speed * Time
				temp = waves[i].w() * waves[i].y() * direction[i].y() * cos( waves[i].x() * direction[i].dot(vec2(x,z)) + (waves[i].z() * time) );
				result.z() += temp;
			}
			result.z() += z;

			//Y = Height
			result.y() = sumSinWaves(x,z);
		

			return result;
		}

		//Sum of sine waves
		float sumSinWaves(float x, float y){
			float result = 0;
			for(int i=0; i<numWaves; i++){
				float temp = 0;
				//		2 *		Ai						* (		sin(		wi					   *		Direction . (x,y)	    +		speed			 * time) /2) ^ K	
				temp = (2 * (waves[i].y() + amplitude)) * power((sin(( waves[i].x() + waveLength *  direction[i].dot(vec2(x,y)) ) + ((waves[i].z() + speed) * time))/2),steepness);
				result += temp;
			}
			return result;
		}

		//Single Sine Wave
		float sinWaveFunction(float x, float y, int waveNum, float _w, float _A, float _speed){
			float result = _A * power(sin(direction[waveNum].dot(vec2(x,y)) * _w  + time * _speed), steepness);
			return result;
		}

		float sumOfSinWaves(float x, float y){
			float result = 0;
			for(int i=0; i<numWaves; i++){
				float temp = 0;
				temp = (waves[i].y() + amplitude) * (sin(( waves[i].x() + waveLength *  direction[i].dot(vec2(x,y)) ) + (waves[i].z() + speed) * time));
				result += temp;
			}
			return result;
		}

		vec2 scalarMultipleVec2(float scalar, vec2 position){
			vec2 result;

			result.x() /= scalar;
			result.y() /= scalar;

			return result;
		}
		
		float power(float value, int exponent){
			float result = value;

			for(int i=1; i<exponent; i++){
				result *= value;
			}

			return result;
		}

		float Steepness(float Q, int waveNumber){
			//Qi = Q/(wi Ai x numWaves)
			float result = Q / (waves[waveNumber].x() * waves[waveNumber].y() * numWaves);
			return result;
		}

		float Speed(float _S){
			float result = _S * (2* M_PI) / waveLength;
			return result;
		}

		float WaveLength(float _waveLength){
			float result = (2*M_PI) / _waveLength;
			return result;
		}

		void simulate() {
			//if (is_key_down('1')) {
			//	cameraY+=10;		
			//	cameraZ+=10;
			//} else if (is_key_down('2')) {
			//	cameraY-=10;		
			//	cameraZ-=10;
			//} else if (is_key_down('W')) {
			//	cameraY+=10;	
			//} else if (is_key_down('S')) {
			//	cameraY-=10;
			//}else if (is_key_down('A')) {
			//	cameraX-=10;
			//} else if (is_key_down('D')) {
			//	cameraX+=10;
			//} else if(is_key_down('Q')) {
			//	modelToWorld.rotateX(2);
			//} else if(is_key_down('E')) {
			//	modelToWorld.rotateX(-2);
			//} else if(is_key_down('Z')) {
			//	printf("\n CameraX: %d, CameraX: %d, CameraX: %d, ",cameraX,cameraY,cameraZ);
			//} 
			
			
			if (is_key_down('U')) {
				steepness+=0.5;
				printf("Steepness: %f \n", steepness);
			} else if (is_key_down('J')) {
				steepness-=0.5;
				printf("Steepness: %f \n", steepness);
			} else if (is_key_down('I')) {
				waveLength -= 0.01f;
				printf("WaveLength: %f \n", waveLength);
			} else if (is_key_down('K')) {
				waveLength += 0.01f;
				printf("WaveLength: %f \n", waveLength);
			} else if (is_key_down('O')) {
				amplitude++;
				printf("Amplitude: %f \n", amplitude);
			} else if (is_key_down('L')) {
				amplitude--;
				printf("Amplitude: %f \n", amplitude);
			} else if (is_key_down('M')) {
				speed+= 0.1f;
				printf("Speed: %f \n", speed);
			}

			//Different Sine Wave
			if (is_key_down('0')) {
				waveLength = 0.07f;
				amplitude = 100.0f;
				speed = 0.24f;
				steepness = 4.4f;
				printf(" WaveLength: %f \n Amplitude: %f \n Speed: %f \n", waveLength,amplitude,speed);
			} else if (is_key_down('9')) {
				waveLength = 0.06f;
				amplitude = 20.0f;
				speed = 2.0f;
				steepness = 1.4f;
				printf(" WaveLength: %f \n Amplitude: %f \n Speed: %f \n", waveLength,amplitude,speed);
			} else if (is_key_down('8')) {
				waveLength = 0.01f;
				amplitude = 20.0f;
				speed = 0.24f;
				steepness = 1.4f;
				printf(" WaveLength: %f \n Amplitude: %f \n Speed: %f \n", waveLength,amplitude,speed);
			}else if (is_key_down('6')) {
				for(int i=0; i<numWaves; i++){
					printf("\n\n Wave: %d \n WaveLength: %f \n Amplitude: %f \n Speed: %f \n Steepness: %f \n", i, waves[i].x(),waves[i].y(),waves[i].z(),waves[i].w());
				}
			} else if (is_key_down('5')) {
				float currentWaveLength = 0;
				float currentSpeed = 0;
				float currentSteepness = 0;
				
				for(int i=0; i<numWaves; i++){
					currentWaveLength += waves[i].x() + waveLength;
					currentSpeed += waves[i].z() + speed;
					currentSteepness += waves[i].w() + steepness;
				}

				printf("\n\n Total Wave: \n WaveLength: %f \n Amplitude: %f \n Speed: %f \n Steepness: %f \n", currentWaveLength,amplitude,currentSpeed,currentSteepness);
			}
		}

		void setCameraToWorld(float X, float Y, float Z){
		
			//put the camera a short distance from the center, looking towards the L System Tree
			cameraToWorld.loadIdentity();
			cameraToWorld.translate(X, Y, Z);
		}



	};	//End of Class App
}

//for(int z=0; z < waterSize; z++){
//	for(int x=0; x < waterSize; x++){
//		vec3 position ;
//
//		position.x() = squaresXZ[x][z]->getP0().x();
//		position.z() = squaresXZ[x][z]->getP0().z();
//		position.y() = sumSinWaves(position.x(),position.z());
//		squaresXZ[x][z]->setP0(vec3(x * squareWidth, position.y() , position.z()));
//
//		position.x() = squaresXZ[x][z]->getP0().x();
//		position.z() = squaresXZ[x][z]->getP0().z();
//		position.y() = sumSinWaves(position.x(),position.z());
//		squaresXZ[x][z]->setP1(vec3((x+1) * squareWidth, position.y() , position.z()));
//
//		position.x() = squaresXZ[x][z]->getP0().x();
//		position.z() = squaresXZ[x][z]->getP0().z();
//		position.y() = sumSinWaves(position.x(),position.z());
//		squaresXZ[x][z]->setP2(vec3((x+1) * squareWidth, position.y() , (position.z() - squareWidth) * z));
//
//		position.x() = squaresXZ[x][z]->getP0().x();
//		position.z() = squaresXZ[x][z]->getP0().z();
//		position.y() = sumSinWaves(position.x(),position.z());
//		squaresXZ[x][z]->setP3(vec3(x * squareWidth, position.y() , (position.z() - squareWidth) * z));
//	}
//}

////Initialise 
//for(int i=0; i < waterSize; i++){
//	squares[i] = new Water_Square();
//	squares[i]->setP0(vec3(i * squareWidth		 , 0 , 0));
//	squares[i]->setP1(vec3((i + 1) * squareWidth	, 0 , 0));
//	squares[i]->setP2(vec3((i + 1) * squareWidth	, 0 , 0 - 1));
//	squares[i]->setP3(vec3(i * squareWidth		 , 0 , 0 - 1));
//}

//for(int i=0; i < waterSize; i++){
//	float x , y = 0 , z;
//
//	x = squares[i]->getP0().x();
//	z = squares[i]->getP0().z();
//	y = sumSinWaves(x,z);
//	squares[i]->setP0(vec3(x , y , z));
//
//	x = squares[i]->getP1().x();
//	z = squares[i]->getP1().z();
//	y = sumSinWaves(x,z);
//	squares[i]->setP1(vec3(x	, y , z));
//
//	x = squares[i]->getP2().x();
//	z = squares[i]->getP2().z();
//	y = sumSinWaves(x,z);
//	squares[i]->setP2(vec3(x	, y , z - 1));
//
//	x = squares[i]->getP3().x();
//	z = squares[i]->getP3().z();
//	y = sumSinWaves(x,z);
//	squares[i]->setP3(vec3(x , y , z - 1));
//}
//
//for(int i=0; i < waterSize; i++){
//	squares[i]->Render();
//}