#pragma once

struct o88oDisplay : OpenGlWidget
{
	o88oDisplay()
	{
		pmodule = NULL;
	}

	void SetModule(o88o *module)
	{
		pmodule = module;
	}

	void drawFramebuffer() override
	{
		if(pmodule == NULL)
			return;
		float square_dim = fbSize.x / NUM_o88o_RECT - margin;

		glViewport(0.0, 0.0, fbSize.x, fbSize.y);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, fbSize.x, 0.0, fbSize.y, -1.0, 1.0);

		float posY = fbSize.y - square_dim - margin;
		for(int r = 0; r < NUM_o88o_RECT; r++)
		{
			float posX = 0;
			for(int c = 0; c < NUM_o88o_RECT; c++)
			{
				glBegin(GL_QUADS);
				NVGcolor dg = pmodule->getCellColor(r,c);
				glColor3f(dg.r, dg.g, dg.b); //colore lancetta

				glVertex3f(posX, posY, 0); // top left
				glVertex3f(posX + square_dim, posY, 0); // top right 
				glVertex3f(posX + square_dim, posY + square_dim, 0); // bottom right
				glVertex3f(posX, posY + square_dim, 0); // bottom left
				glEnd();
				posX += square_dim + margin;
			}

			posY -= square_dim + margin;
		}
	}

private:
	o88o *pmodule;
	const int margin = 1;
};
