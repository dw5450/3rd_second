#include "stdafx.h"
#include "object.h"
#include "math.h"
#include "float.h"

void Object::GetPosition(float * x, float * y, float * z)
{
	*x = m_PosX;
	*y = m_PosY;
	*z = m_PosZ;
}

void Object::GetSize(float * sx, float * sy)
{
	*sx = m_SizeX;
	*sy = m_SizeY;

}

void Object::GetColor(float * r, float * g, float * b, float * a)
{
	*r = m_r;
	*g = m_g;
	*b = m_b;
	*a = m_a;
}

void Object::GetVelocity(float * x, float * y)
{
	*x = m_VelX;
	*y = m_VelY;
}

void Object::GetAcc(float * x, float * y)
{
	*x = m_AccX;
	*y = m_AccY;
}

void Object::GetMass(float * x)
{
	*x = m_Mass;
}

void Object::GetCoefFrict(float * x)
{
	*x = m_CoefFrict;
}


void Object::SetAcc(float x, float y)
{
	m_AccX = x;
	m_AccY = y;
}

void Object::SetMass(float x)
{
	m_Mass = x;
}

void Object::SetCoefFrict(float x)
{
	m_CoefFrict = x;
}


void Object::SetVelocity(float x, float y)
{
	m_VelX = x;
	m_VelY = y;
}



void Object::SetPosition(float x, float y, float z)
{
	m_PosX = x;
	m_PosY = y;
	m_PosZ = z;
}


void Object::SetSize(float sz, float sy)
{
	m_SizeX = sz;
	m_SizeY = sy;
}

void Object::SetColor(float r, float g, float b, float a)
{
	m_r = r;
	m_g = g;
	m_b = b;
	m_a = a;
}

void Object::Update(float eTime)
{
	float magVel = sqrtf(m_VelX * m_VelX + m_VelY * m_VelY);
	float velx = m_VelX / magVel;
	float vely = m_VelY / magVel;
	float fricX = -velx;
	float fricY = -vely;
	float friction = m_CoefFrict * m_Mass * 9.8;

	fricX *= friction;
	fricY *= friction;

	if (magVel < FLT_EPSILON)
	{
		m_VelX = 0.f;
		m_VelY = 0.f;
	}
	else
	{
		float accX = fricX / m_Mass;
		float accY = fricY / m_Mass;

		// cal velocity
		float afterVelx = m_VelX + accX * eTime;
		float afterVely = m_VelY + accY * eTime;


		if (afterVelx * m_VelX < 0.f)
			m_VelX = 0.f;
		else
			m_VelX = afterVelx;

		if (afterVely * m_VelY < 0.f)
			m_VelY = 0.f;
		else
			m_VelY = afterVely;
	}


	// call velocity
	m_VelX = m_VelX + m_AccX * eTime;
	m_VelY = m_VelY + m_AccY * eTime;


	// calc position
	m_PosX = m_PosX + m_VelX * eTime;
	m_PosY = m_PosY + m_VelY * eTime;
}

void Object::ApplyForce(float x, float y, float eTime)
{
	m_AccX = x / m_Mass;
	m_AccY = y / m_Mass;

	m_VelX = m_VelX + m_AccX * eTime;
	m_VelY = m_VelY + m_AccY * eTime;

	m_AccX = 0.f;
	m_AccY = 0.f;
}
