#pragma once

class Object {
private:
	float m_PosX;
	float m_PosY;
	float m_PosZ;
	float m_SizeX;
	float m_SizeY;
	float m_Mass;
	float m_CoefFrict;
	

	float m_VelX;
	float m_VelY;
	float m_AccX;
	float m_AccY;

	float m_time;

	float m_r,m_g,m_b,m_a;
public:
	Object() {}

	void GetPosition(float *x, float *y, float * z);
	void GetSize(float *sz, float *sy);
	void GetColor(float *r, float *g, float *b, float *a);
	void GetVelocity(float *x, float *y);
	void GetAcc(float *x, float *y);
	void GetMass(float *x);
	void GetCoefFrict(float *x);



	void SetPosition(float x, float y, float z);
	void SetSize(float sz, float sy);
	void SetColor(float r, float g, float b, float a);
	void SetVelocity(float x, float y);
	void SetAcc(float x, float y);
	void SetMass(float x);
	void SetCoefFrict(float x);



	void Update(float eTime);
	void ApplyForce(float x, float y, float eTime);
	

	~Object() {}
};