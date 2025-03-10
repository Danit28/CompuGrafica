//=============================================================================
// Sample Application: Lighting (Per Fragment Phong)
//=============================================================================

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "glApplication.h"
#include "glutWindow.h"
#include <iostream>
#include "glsl.h"
#include <time.h>
#include "glm.h"
#include <FreeImage.h> //*** Para Textura: Incluir librer�a
#define NM 10

//-----------------------------------------------------------------------------


//Clase creada para inicializar los modelos 3D
/* Contiene los atributos para el puntero del modelo3d, la identificacion de la textura y un booleano indicando si tiene textura o no 
* Se crean los metodos para abrirMalla, DibujarMalla y inicializarTextura
* Estos clase se utiliza cada vez que se quiere crear un objeto 3d en la escena 
*/

//Esta clase la realizo Daniela Tellez Cobo
class Model {
	protected:

		GLMmodel* model;  // Puntero al modelo 3D cargado
		GLuint textureID; // ID de la textura OpenGL
		bool textured; // Indica si el modelo tiene textura

	public:

		Model() : model(nullptr), textureID(0), textured(false) {}

		// Método para cargar un archivo de modelo 3D
		virtual void AbrirMalla(char* nombre)
		{
			model = NULL;
			if (!model)
			{
				model = glmReadOBJ(nombre); // Carga del modelo OBJ
				if (!model)
					exit(0);

				// Normalización y cálculo de normales para una mejor iluminación
				glmUnitize(model);
				glmFacetNormals(model);
				glmVertexNormals(model, 90.0);

			}

		}

		// Método para dibujar el modelo en la escena
		virtual void DibujarMalla( float x, float y, float z, float scale, int rotatey, cwc::glShader* shader) {


			if (shader) shader->begin();// Activa el shader

			glPushMatrix(); // Guarda la transformación actual
			glTranslatef(x, y, z); // Posiciona el modelo en la escena
			glScalef(scale, scale, scale); // Escala el modelo

			if (!textured) {
				glmDraw(model, GLM_SMOOTH | GLM_MATERIAL); // Dibuja el modelo sin textura

			}
			else {
				glRotatef(rotatey, 0, 1, 0); // Rotación opcional
				glBindTexture(GL_TEXTURE_2D, textureID); // Vincula la textura
				glmDraw(model, GLM_SMOOTH | GLM_MATERIAL | GLM_TEXTURE); // Dibuja el modelo con textura

			}

			glPopMatrix(); // Restaura la transformación anterior


			if (shader) shader->end(); // Desactiva el shader

			
		}

		// Método para inicializar una textura desde un archivo
		virtual void initialize_textures(const char* filename)
		{

			glGenTextures(1, &textureID); // Genera un ID de textura
			glBindTexture(GL_TEXTURE_2D, textureID); // Enlaza la textura a OpenGL
			glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			FIBITMAP* bitmap;

			bitmap = FreeImage_Load(FreeImage_GetFileType(filename, 0),filename); // Carga la imagen
			//*** Para Textura: esta es la ruta en donde se encuentra la textura
			if (!bitmap) return;

			FIBITMAP* pImage = FreeImage_ConvertTo32Bits(bitmap);
			int nWidth = FreeImage_GetWidth(pImage);
			int nHeight = FreeImage_GetHeight(pImage);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight,
				0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(pImage));

			FreeImage_Unload(pImage); // Libera la imagen
			glEnable(GL_TEXTURE_2D); // Habilita el uso de texturas en OpenGL
			textured = true; // Marca el modelo como texturizado
		}

};




class myWindow : 
	public cwc::glutWindow
{
protected:
	cwc::glShaderManager SM;
	cwc::glShader* shader; // Shader principal
	cwc::glShader* shader1; //Para Textura: variable para abrir los shader de textura
	std::vector<Model> modelos; //Se crea un vector que contiene objetos de clase Model, este vector contiene todos los modelos de la escena 
	GLuint ProgramObject;
	clock_t time0, time1;
	float timer010;  // timer counting 0->1->0
	bool bUp;        // flag if counting up or down.


public:
	myWindow() {}

	// Método para renderizar la escena //La realizo David Alonso Bayona 
	virtual void OnRender(void)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glPushMatrix();
		//ARGS: x. y. z, scale, rotateY, shader

		glTranslatef(0, 0, -0.5); // Hacerle zoom it and out a la imagen

		//Dibujar los modelos 
		//Sin textura
		modelos[0].DibujarMalla(0.1, 0, 0, 0.1, 0, shader); // Uvas
		modelos[1].DibujarMalla(0, 0, 0, 0.2, 0, shader); // Chill guy
		modelos[2].DibujarMalla(0.1, -0.45, 0.7, 0.15, 0, shader); // Cadenas
		modelos[3].DibujarMalla(-0.2, -0.45, 0.7, 0.15, 0, shader); // Pitufo

		//Con textura
		modelos[4].DibujarMalla(-0.6, -0.2, 0.1, 0.1, 180, shader1); // Chicken
		modelos[5].DibujarMalla(0.0, 0, 0, 1.5, 0, shader1); // Casa
		modelos[6].DibujarMalla(-1.5, 1, 0, 0.5, 0, shader1); //Moon


		glutSwapBuffers();
		glPopMatrix();

		UpdateTimer();

		Repaint(); // Vuelve a dibujar la ventana
	}

	virtual void OnIdle() {}

	// Método para inicializar OpenGL y shaders //La realizo Santiago Salazar 
	virtual void OnInit()
	{
		glClearColor(2.0f, 1.0f, 3.0f, 1.2f);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_DEPTH_TEST);

		shader = SM.loadfromFile("vertexshader.txt", "fragmentshader.txt"); // load (and compile, link) from file
		if (shader == 0)
			std::cout << "Error Loading, compiling or linking shader\n";
		else
		{
			ProgramObject = shader->GetProgramObject();
		}

		//*** Para Textura: abre los shaders para texturas
		shader1 = SM.loadfromFile("vertexshaderT.txt", "fragmentshaderT.txt"); // load (and compile, link) from file
		if (shader1 == 0)
			std::cout << "Error Loading, compiling or linking shader\n";
		else
		{
			ProgramObject = shader1->GetProgramObject();
		}

		time0 = clock();
		timer010 = 0.0f;
		bUp = true;

		modelos.resize(NM);

		// Name
		//Objetos sin textura 
		modelos[0].AbrirMalla("./Mallas/uvas.obj"); //Uvas
		modelos[1].AbrirMalla("./Mallas/chill_guy.obj"); //Chill Guy
		modelos[2].AbrirMalla("./Mallas/cadenitas.obj"); //Cadenitas
		modelos[3].AbrirMalla("./Mallas/Pitufo.obj");  //Gato Pitufo

		//Textura
		modelos[4].AbrirMalla("./Mallas/chicken.obj"); // gallina
		modelos[5].AbrirMalla("./Mallas/granja.obj"); // granja
		modelos[6].AbrirMalla("./Mallas/moon.obj");  //luna
		


		//*** Para Textura: abrir archivo de textura
		modelos[4].initialize_textures("./Mallas/chickenF_color_1.jpeg"); //Textura de gallina
		modelos[5].initialize_textures("./Mallas/textura_madera.jpeg"); //textura de granja
		modelos[6].initialize_textures("./Mallas/moon_00_0.jpeg");  //textura de luna
		DemoLight(); 

	}

	virtual void OnResize(int w, int h)
	{
		if (h == 0) h = 1;
		float ratio = 1.0f * (float)w / (float)h;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glViewport(0, 0, w, h);

		gluPerspective(45, ratio, 1, 100);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0.0f, 0.0f, 4.0f,
			0.0, 0.0, -1.0,
			0.0f, 1.0f, 0.0f);
	}
	virtual void OnClose(void) {}
	virtual void OnMouseDown(int button, int x, int y) {}
	virtual void OnMouseUp(int button, int x, int y) {}
	virtual void OnMouseWheel(int nWheelNumber, int nDirection, int x, int y) {}

	virtual void OnKeyDown(int nKey, char cAscii)
	{
		if (cAscii == 27) // 0x1b = ESC
		{
			this->Close(); // Close Window!
		}
	};

	virtual void OnKeyUp(int nKey, char cAscii)
	{
		if (cAscii == 's')      // s: Shader
			shader->enable();
		else if (cAscii == 'f') // f: Fixed Function
			shader->disable();
	}

	void UpdateTimer()
	{
		time1 = clock();
		float delta = static_cast<float>(static_cast<double>(time1 - time0) / static_cast<double>(CLOCKS_PER_SEC));
		delta = delta / 4;
		if (delta > 0.00005f)
		{
			time0 = clock();
			if (bUp)
			{
				timer010 += delta;
				if (timer010 >= 1.0f) { timer010 = 1.0f; bUp = false; }
			}
			else
			{
				timer010 -= delta;
				if (timer010 <= 0.0f) { timer010 = 0.0f; bUp = true; }
			}
		}
	}

	// Método para manejar iluminación
	void DemoLight(void)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_NORMALIZE);

	}
};

//-----------------------------------------------------------------------------
// Clase principal de la aplicación
class myApplication : public cwc::glApplication
{
public:
	virtual void OnInit() { std::cout << "Hello World!\n"; }
};

//-----------------------------------------------------------------------------

int main(void)
{
	myApplication* pApp = new myApplication;
	myWindow* myWin = new myWindow();

	pApp->run();
	delete pApp;
	return 0;
}

//-----------------------------------------------------------------------------

