#include "Light.h"

Light::Light()
{
}

Light::~Light()
{
}

void Light::SetAmbientColor(float r, float g, float b, float a)
{
	m_ambientColor = XMFLOAT4(r, g, b, a);
}

void Light::SetDiffuseColor(float r, float g, float b, float a)
{
	m_diffuseColor = XMFLOAT4(r, g, b, a);
}

void Light::SetDirection(float x, float y, float z)
{
	m_direction = XMFLOAT3(x, y, z);
}

void Light::SetSpecularColor(float r, float g, float b, float a)
{
	m_specularColor = XMFLOAT4(r, g, b, a);
}

void Light::SetSpecularPower(float power)
{
	m_specularPower = power;
}

void Light::SetPosition(float x, float y, float z)
{
	m_position = XMFLOAT4(x, y, z, 1.0f);
}

XMFLOAT4 Light::GetAmbientColor()
{
	return m_ambientColor;
}

XMFLOAT4 Light::GetDiffuseColor()
{
	return m_diffuseColor;
}

XMFLOAT3 Light::GetDirection()
{
	return m_direction;
}

XMFLOAT4 Light::GetSpecularColor()
{
	return m_specularColor;
}

float Light::GetSpecularPower()
{
	return m_specularPower;
}

XMFLOAT4 Light::GetPosition()
{
	return m_position;
}
