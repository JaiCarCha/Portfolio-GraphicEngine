#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

enum Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	SPEED1,
	SPEED2
};

class Camera 
{
private:
	glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 direction = glm::vec3(0.f, 0.f, -1.f);
	glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 right = glm::vec3(1.f, 0.f, 0.f);

	int cameraWidth = 1600;
	int cameraHeight = 900;
	float farPlane = 1000.f;
	float nearPlane = 0.1f;

	float pitch = 0.f;
	float yaw = -90.f;  // yaw = 0 -> positive X axis // yaw = -90 -> negative Z axis
	const float sensitivity = 0.1f;
	float zoom = 45.f;
	float cameraSpeed = 2.5f;

public:

	Camera(glm::vec3 pos = glm::vec3(0.f, 0.f, 0.f), float y = -90.f, float p = 0.f)
	{
		position = pos;
		yaw = y;
		pitch = p;

		setDirection(p, y);
	}

	Camera(glm::vec3 pos, glm::vec3 dir)
	{
		position = pos;
		
		setDirection(dir);
	}

	glm::vec3 getPosition() const
	{
		return position;
	}

	glm::mat4 getViewMatrix() const
	{
		return glm::lookAt(position, position + direction, worldUp);
	}

	glm::mat4 getProjectionMatrix(bool perspective) const
	{
		float windowRatio = (float) cameraWidth / cameraHeight;

		glm::mat4 p;
		if (perspective) p = glm::perspective(glm::radians(zoom), windowRatio, nearPlane, farPlane); // Zoom == fov
		else p = glm::ortho(-zoom * (windowRatio) * 0.5f, zoom * (windowRatio) * 0.5f, -zoom * 0.5f, zoom * 0.5f, nearPlane, farPlane);
		return p;
	}

	void cameraMouse(float xoffset, float yoffset) 
	{
		// Mouse sensitivity
		const float sensitivity = 0.10f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		// Limit pitch to avoid look up/down too much
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		//std::cout << yaw << " " << pitch << std::endl;

		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction = glm::normalize(direction);
		right = glm::normalize(glm::cross(direction, worldUp));
		up = glm::normalize(glm::cross(direction, right));
	}
	
	void cameraKeyboard(Movement m, float deltaTime) 
	{
		if (m == SPEED1) cameraSpeed = 2.5f;
		if (m == SPEED2) cameraSpeed = 5.f;

		if (m == FORWARD) position += direction * cameraSpeed * deltaTime;
		if (m == BACKWARD) position -= direction * cameraSpeed * deltaTime;
		if (m == RIGHT) position += right * cameraSpeed * deltaTime;
		if (m == LEFT) position -= right * cameraSpeed * deltaTime;
		if (m == UP) position += up * cameraSpeed * deltaTime;
		if (m == DOWN) position -= up * cameraSpeed * deltaTime;
	}

	void cameraScroll(float yoffset) 
	{
		zoom -= yoffset;
		if (zoom < 1.0f) zoom = 1.0f;
		if (zoom > 45.0f) zoom = 45.0f;
	}

	void setPosition(glm::vec3 pos)
	{
		position = pos;
	}

	void setDirection(glm::vec3 dir)
	{
		direction = glm::normalize(dir);

		right = glm::cross(direction, worldUp);
		up = glm::cross(direction, right);

		float p, y;
		bool complete = false;

		p = glm::degrees(glm::asin(direction.y));
		y = glm::degrees(glm::acos(direction.x / cos(glm::radians(pitch))));

		if (sin(glm::radians(y)) * cos(glm::radians(p)) != direction.z)
		{
			p = p * -1;
			if (cos(glm::radians(yaw)) * cos(glm::radians(pitch)) != direction.x ||
				sin(glm::radians(pitch)) != direction.y ||
				sin(glm::radians(yaw)) * cos(glm::radians(pitch)) != direction.z)
			{
				p = p * -1;
				y = y * -1;
			}
		}
		pitch = p;
		yaw = y;
	}

	void setDirection(float pitch, float yaw)
	{
		this->yaw = yaw;
		this->pitch = pitch;

		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction = glm::normalize(direction);
		right = glm::cross(direction, worldUp);
		up = glm::cross(direction, right);
	}

	float getZoom() const
	{
		return zoom;
	}

	void setZoom(float newZoom)
	{
		zoom = newZoom;
	}

	void setCameraResolution(int width, int height)
	{
		cameraWidth = width;
		cameraHeight = height;
	}

	float getFarPlane() const
	{
		return farPlane;
	}

	float getNearPlane() const
	{
		return nearPlane;
	}

	void setPlanes(float near, float far)
	{
		nearPlane = near;
		farPlane = far;
	}

};
#endif CAMERA_H