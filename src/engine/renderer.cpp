#include "renderer.hpp"

#include <algorithm>

#include "engine.hpp"
#include "sprite.hpp"

const char * LESS_BASIC_VERTEX_SHADER =
	"#version 330 core\n"
	"layout(location = 0) in vec2 position;"
	"layout(location = 1) in vec2 texCoord;"
	"uniform mat4 u_Model;"
	"uniform mat4 u_texMap;"
	"out vec2 vTexCoord;"
	"mat4 ortho(float left, float right, float bottom, float top) {"
	"return mat4(vec4(2.0 / (right - left), 0, 0, 0), vec4(0, 2.0 / top - bottom, 0, 0), vec4(0, 0, -1, 0), vec4(-(right + left) / (right - left), -(top + bottom) / (top - bottom), 0, 1));}"
	"void main() {"
		"gl_Position =  ortho(0, 1280, 0, 720) * u_Model * vec4(position, 0.0f, 1.0f);"
		"vTexCoord = vec2(u_texMap * vec4(texCoord, 0.0, 1.0));"
	"}";

const char * BASIC_VERTEX_SHADER =
	"#version 330 core\n"
	"layout(location = 0) in vec2 position;"
	"layout(location = 1) in vec2 texCoord;"
	"out vec2 vTexCoord;"
	"void main() {"
		"vec4 pos = vec4(position, 0.0f, 1.0f);"
		"gl_Position = pos;"
		"vTexCoord = texCoord;"
	"}";

// Index orders for different shapes
const unsigned int LINE_INDICES[2] = { 0, 1 };
const unsigned int SQUARE_INDICES[6] = { 0, 1, 3, 0, 2, 3 };

Renderer::Renderer() : 
	basicShader(LESS_BASIC_VERTEX_SHADER, BASIC_COLOUR_SHADER),
	textureShader(LESS_BASIC_VERTEX_SHADER, BASIC_TEXTURE_SHADER)
{
	initRectangleData();
	initLineData();

}

void Renderer::clear() const {
	glClearColor(1, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::drawLine(ScreenCoord v1, ScreenCoord v2, Colour colour) {
	float positions[4] = {
		lerp(-1.f, 1.f, static_cast<float>(v1.x()) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(v1.y()) / static_cast<float>(Engine::get_instance().getWindowHeight())),
		lerp(-1.f, 1.f, static_cast<float>(v2.x()) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(v2.y()) / static_cast<float>(Engine::get_instance().getWindowHeight()))
	};
	VertexArray		va;
	VertexBuffer	vb(positions, sizeof(float) * 4);
	IndexBuffer		ib(LINE_INDICES, 2);
	// Specify the layout of the buffer data
	VertexBufferLayout layout;
	layout.pushFloat(2);
	va.addBuffer(vb, layout);

	// Set the uniform to draw the right colour
	basicShader.setUniform4f("u_Colour", colour.r(), colour.g(), colour.b(), alpha);
	drawLines(va, ib, basicShader);
}

void Renderer::drawLine_fast(ScreenCoord v1, ScreenCoord v2, Colour colour) {
	basicShader.bind();
	int width = v2.x() - v1.x();
	int height = v2.y() - v1.y();
	float matrix[16] = {
		width, 0.f, 0.f, v1.x(),
		0.f, -height, 0.f, static_cast<float>(Engine::get_instance().getWindowHeight()) - v1.y(),
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	basicShader.setUniform4f("u_Colour", colour.r(), colour.g(), colour.b(), alpha);
	basicShader.setUniformmat4("u_Model", matrix);

	glBindVertexArray(lineVAO_);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void Renderer::drawRect_fast(ScreenCoord v, int width, int height, Colour colour) {
	basicShader.bind();
	float matrix[16] = {
		width, 0.f, 0.f, v.x(),
		0.f, -height, 0.f, static_cast<float>(Engine::get_instance().getWindowHeight()) - v.y(),
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	basicShader.setUniform4f("u_Colour", colour.r(), colour.g(), colour.b(), alpha);
	basicShader.setUniformmat4("u_Model", matrix);

	glBindVertexArray(rectangleVAO_);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void Renderer::drawSprite_fast(const Sprite & sprite) {
	textureShader.bind();
	float matrix[16] = {
		sprite.w, 0.f, 0.f, sprite.x,
		0.f, -sprite.h, 0.f, static_cast<float>(Engine::get_instance().getWindowHeight()) - sprite.y,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	float texMatrix[16] = {
		float(sprite.src_w)/sprite.original_w, 0.f, 0.f, float(sprite.src_x)/sprite.original_w,
		0.f, -float(sprite.src_h)/sprite.original_h, 0.f, float(sprite.src_y + sprite.src_h)/sprite.original_h,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};


	textureShader.setUniformmat4("u_Model", matrix);
	textureShader.setUniformmat4("u_texMap", texMatrix);

	sprite.getTexture().bind();

	glBindVertexArray(rectangleVAO_);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void Renderer::drawRect(ScreenCoord v, int width, int height, Colour colour) {
	float positions[16] = {
		lerp(-1.f, 1.f, static_cast<float>(v.x()) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(v.y()) / static_cast<float>(Engine::get_instance().getWindowHeight())),
		lerp(-1.f, 1.f, static_cast<float>(v.x()) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(v.y() + height) / static_cast<float>(Engine::get_instance().getWindowHeight())),
		lerp(-1.f, 1.f, static_cast<float>(v.x() + width) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(v.y()) / static_cast<float>(Engine::get_instance().getWindowHeight())),
		lerp(-1.f, 1.f, static_cast<float>(v.x() + width) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(v.y() + height) / static_cast<float>(Engine::get_instance().getWindowHeight()))
	};
	VertexArray		va;
	VertexBuffer	vb(positions, sizeof(float) * 16);
	IndexBuffer		ib(SQUARE_INDICES, 6);
	// Specify the layout of the buffer data
	VertexBufferLayout layout;
	layout.pushFloat(2);
	va.addBuffer(vb, layout);

	// Issue the actual draw call
	basicShader.setUniform4f("u_Colour", colour.r(), colour.g(), colour.b(), alpha);
	drawTriangles(va, ib, basicShader);
}

void Renderer::drawTexture(ScreenCoord v, int width, int height, const Texture & texture) {
	float positions[16] = {
		lerp(-1.f, 1.f, static_cast<float>(v.x()) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(v.y()) / static_cast<float>(Engine::get_instance().getWindowHeight())),
		0.f, 1.f,
		lerp(-1.f, 1.f, static_cast<float>(v.x()) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(v.y() + height) / static_cast<float>(Engine::get_instance().getWindowHeight())),
		0.f, 0.f,
		lerp(-1.f, 1.f, static_cast<float>(v.x() + width) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(v.y()) / static_cast<float>(Engine::get_instance().getWindowHeight())),
		1.f, 1.f,
		lerp(-1.f, 1.f, static_cast<float>(v.x() + width) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(v.y() + height) / static_cast<float>(Engine::get_instance().getWindowHeight())),
		1.f, 0.f
	};
	VertexArray		va;
	VertexBuffer	vb(positions, sizeof(float) * 16);
	IndexBuffer		ib(SQUARE_INDICES, 6);
	// Specify the layout of the buffer data
	VertexBufferLayout layout;
	layout.pushFloat(2);
	layout.pushFloat(2);
	va.addBuffer(vb, layout);

	// Bind the texture and draw
	texture.bind();
	drawTriangles(va, ib, textureShader);
}

void Renderer::drawSprite(const Sprite & sprite) {
	float positions[16] = {
		// coordinate 1
		lerp(-1.f, 1.f, static_cast<float>(sprite.x) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(sprite.y) / static_cast<float>(Engine::get_instance().getWindowHeight())),
		lerp(0.f, 1.f, static_cast<float>(sprite.src_x) / static_cast<float>(sprite.original_w)),
		lerp(0.f, 1.f, static_cast<float>(sprite.src_y + sprite.src_h) / static_cast<float>(sprite.original_h)),
		// coordinate 2
		lerp(-1.f, 1.f, static_cast<float>(sprite.x) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(sprite.y + sprite.h) / static_cast<float>(Engine::get_instance().getWindowHeight())),
		lerp(0.f, 1.f, static_cast<float>(sprite.src_x) / static_cast<float>(sprite.original_w)),
		lerp(0.f, 1.f, static_cast<float>(sprite.src_y) / static_cast<float>(sprite.original_h)),
		// coordinate 3
		lerp(-1.f, 1.f, static_cast<float>(sprite.x + sprite.w) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(sprite.y) / static_cast<float>(Engine::get_instance().getWindowHeight())),
		lerp(0.f, 1.f, static_cast<float>(sprite.src_x + sprite.src_w) / static_cast<float>(sprite.original_w)),
		lerp(0.f, 1.f, static_cast<float>(sprite.src_y + sprite.src_h) / static_cast<float>(sprite.original_h)),
		// coordinate 4
		lerp(-1.f, 1.f, static_cast<float>(sprite.x + sprite.w) / static_cast<float>(Engine::get_instance().getWindowWidth())),
		lerp(1.f, -1.f, static_cast<float>(sprite.y + sprite.h) / static_cast<float>(Engine::get_instance().getWindowHeight())),
		lerp(0.f, 1.f, static_cast<float>(sprite.src_x + sprite.src_w) / static_cast<float>(sprite.original_w)),
		lerp(0.f, 1.f, static_cast<float>(sprite.src_y) / static_cast<float>(sprite.original_h))
		
	};
	VertexArray		va;
	VertexBuffer	vb(positions, sizeof(float) * 16);
	IndexBuffer		ib(SQUARE_INDICES, 6);
	// Specify the layout of the buffer data
	VertexBufferLayout layout;
	layout.pushFloat(2);
	layout.pushFloat(2);
	va.addBuffer(vb, layout);

	// Bind the texture and draw
	sprite.getTexture().bind();
	drawTriangles(va, ib, textureShader);
}

void Renderer::drawTriangles(const VertexArray & va, const IndexBuffer & ib, const Shader & shader) const {
	draw(va, ib, shader, GL_TRIANGLES);
}

void Renderer::drawLines(const VertexArray & va, const IndexBuffer & ib, const Shader & shader) const {
	draw(va, ib, shader, GL_LINES);
}

void Renderer::drawLineStrip(const VertexArray & va, const IndexBuffer & ib, const Shader & shader) const {
	draw(va, ib, shader, GL_LINE_STRIP);
}

void Renderer::draw(const VertexArray & va, const IndexBuffer & ib, const Shader & shader, GLenum type) const {
	shader.bind();
	va.bind();
	ib.bind();

	// TODO: Get GLtype from index buffer
	glDrawElements(type, ib.getCount(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::setAlpha(float a) {
	alpha = std::fmin(1.f, std::fmax(0.f, a));
}


void Renderer::initRectangleData() {
	glGenVertexArrays(1, &rectangleVAO_);
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	GLfloat vertices[] = {
		// Pos      // Tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(rectangleVAO_);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (void*)(2 * sizeof(GL_FLOAT)));


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void Renderer::initLineData() {
	glGenVertexArrays(1, &lineVAO_);
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	GLfloat lineVertices[] = {
		// Pos
		0.0f, 0.0f,
		1.0f, 1.0f,
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

	glBindVertexArray(lineVAO_);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}